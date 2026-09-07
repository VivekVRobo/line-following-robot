#!/usr/bin/env python3
"""Dependency-free differential-drive line follower reference simulator.

This is a deterministic controls/regression aid, not a physics-validated digital
twin and not a source of physical performance claims.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import random
from dataclasses import asdict, dataclass
from pathlib import Path


SIMULATOR_MODEL_VERSION = 2


@dataclass
class SimConfig:
    dt: float = 0.01
    seconds: float = 20.0
    wheelbase_m: float = 0.12
    max_speed_mps: float = 0.60
    max_pwm: int = 220
    sensor_half_width_m: float = 0.035
    sensor_forward_offset_m: float = 0.045
    track_amplitude_m: float = 0.05
    track_wavelength_m: float = 1.2
    noise_std_position: float = 18.0
    initial_offset_m: float = 0.02
    seed: int = 7


class PID:
    def __init__(self, kp: float = 0.085, ki: float = 0.0008, kd: float = 0.30, output_limit: float = 190.0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.output_limit = output_limit
        self.reset()

    def reset(self) -> None:
        self.i = 0.0
        self.prev = None
        self.derivative = 0.0

    def update(self, error: float, dt: float) -> float:
        raw_derivative = 0.0 if self.prev is None else (error - self.prev) / dt
        self.derivative = 0.35 * raw_derivative + 0.65 * self.derivative
        candidate_i = max(-4500.0, min(4500.0, self.i + error * dt))
        candidate = self.kp * error + self.ki * candidate_i + self.kd * self.derivative
        limited = max(-self.output_limit, min(self.output_limit, candidate))
        if not (abs(candidate - limited) > 1e-9 and candidate * error > 0):
            self.i = candidate_i
        self.prev = error
        return max(
            -self.output_limit,
            min(self.output_limit, self.kp * error + self.ki * self.i + self.kd * self.derivative),
        )


def track_y(x: float, config: SimConfig) -> float:
    return config.track_amplitude_m * math.sin(2 * math.pi * x / config.track_wavelength_m)


def track_slope(x: float, config: SimConfig) -> float:
    return (
        config.track_amplitude_m
        * (2 * math.pi / config.track_wavelength_m)
        * math.cos(2 * math.pi * x / config.track_wavelength_m)
    )


def sensor_lateral_error(x: float, y: float, heading: float, config: SimConfig) -> float:
    """Approximate line error at the forward sensor-bar center.

    Version 1 of the simulator sampled the line at the robot center. During a
    pure recovery spin the center does not translate, so that model could not
    represent the sensor bar sweeping across the line. Version 2 projects a
    sensor-bar center ahead of the axle and measures its approximate normal
    displacement from the local sinusoidal track tangent.
    """
    sensor_x = x + math.cos(heading) * config.sensor_forward_offset_m
    sensor_y = y + math.sin(heading) * config.sensor_forward_offset_m
    line_y = track_y(sensor_x, config)
    tangent_angle = math.atan(track_slope(sensor_x, config))
    return (sensor_y - line_y) * math.cos(tangent_angle)


def adaptive_speed(position: float, confidence: float) -> int:
    slowdown = 0.48 * min(1.0, abs(position) / 2000.0) + 0.30 * (1.0 - confidence)
    return round(max(90.0, 145.0 * (1.0 - min(0.85, slowdown))))


def mix(base: float, correction: float, max_pwm: int) -> tuple[int, int]:
    left, right = base + correction, base - correction
    peak = max(abs(left), abs(right))
    if peak > max_pwm:
        scale = max_pwm / peak
        left *= scale
        right *= scale
    return round(left), round(right)


def simulate(config: SimConfig) -> list[dict]:
    if config.dt <= 0 or config.seconds <= 0:
        raise ValueError("dt and seconds must be > 0")
    if config.sensor_half_width_m <= 0 or config.sensor_forward_offset_m < 0:
        raise ValueError("sensor geometry must be non-negative with positive half width")

    random.seed(config.seed)
    pid = PID()
    x, y, heading = 0.0, config.initial_offset_m, 0.0
    rows: list[dict] = []
    recovering = False
    last_direction = 1
    recovery_started = 0.0

    for step in range(int(config.seconds / config.dt)):
        t = step * config.dt
        sensor_error = sensor_lateral_error(x, y, heading, config)
        visible = abs(sensor_error) <= config.sensor_half_width_m
        raw_position = (sensor_error / config.sensor_half_width_m) * 2000.0
        position = raw_position + random.gauss(0, config.noise_std_position)
        confidence = (
            max(0.0, min(1.0, 1.0 - abs(sensor_error) / config.sensor_half_width_m))
            if visible
            else 0.0
        )

        if visible:
            if recovering:
                pid.reset()
                recovering = False
            error = position
            if error > 70:
                last_direction = 1
            elif error < -70:
                last_direction = -1
            base = adaptive_speed(error, confidence)
            correction = pid.update(error, config.dt)
            left, right = mix(base, correction, config.max_pwm)
            mode = "TRACK"
            phase = 0
        else:
            if not recovering:
                recovering = True
                recovery_started = t
                pid.reset()
            elapsed = t - recovery_started
            if elapsed < 0.35:
                direction, speed, phase = last_direction, 105, 1
            else:
                segment = int((elapsed - 0.35) / 0.60)
                direction = last_direction if segment % 2 == 0 else -last_direction
                speed, phase = 125, 2 + (segment % 2)
            left, right = direction * speed, -direction * speed
            base, correction, error, mode = 0, 0.0, 0.0, "RECOVER"

        left_velocity = (left / config.max_pwm) * config.max_speed_mps
        right_velocity = (right / config.max_pwm) * config.max_speed_mps
        velocity = 0.5 * (left_velocity + right_velocity)
        omega = (right_velocity - left_velocity) / config.wheelbase_m
        x += velocity * math.cos(heading) * config.dt
        y += velocity * math.sin(heading) * config.dt
        heading += omega * config.dt

        rows.append(
            {
                "ms": round(t * 1000),
                "mode": mode,
                "position": round(position if visible else 0.0, 2),
                "confidence": round(confidence, 4),
                "total": round(confidence * 5000),
                "error": round(error, 2),
                "correction": round(correction, 2),
                "base": base,
                "left": left,
                "right": right,
                "recovery_phase": phase,
                "x_m": round(x, 5),
                "y_m": round(y, 5),
                "track_y_m": round(track_y(x, config), 5),
                "cross_track_m": round(y - track_y(x, config), 5),
                "sensor_cross_track_m": round(sensor_error, 5),
            }
        )
    return rows


def _percentile(values: list[float], fraction: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, max(0, math.ceil(fraction * len(ordered)) - 1))
    return ordered[index]


def summarize(rows: list[dict]) -> dict:
    if not rows:
        return {
            "samples": 0,
            "distance_x_m": 0.0,
            "rms_cross_track_m": 0.0,
            "p95_abs_cross_track_m": 0.0,
            "max_cross_track_m": 0.0,
            "rms_sensor_cross_track_m": 0.0,
            "recovery_ratio": 0.0,
            "recovery_episodes": 0,
            "max_recovery_duration_s": 0.0,
        }

    errors = [abs(float(row["cross_track_m"])) for row in rows]
    sensor_errors = [abs(float(row["sensor_cross_track_m"])) for row in rows]
    recovery_count = sum(row["mode"] == "RECOVER" for row in rows)

    dt_s = 0.0
    if len(rows) > 1:
        dt_s = max(0.0, (float(rows[1]["ms"]) - float(rows[0]["ms"])) / 1000.0)
    recovery_episodes = 0
    recovery_streak = 0
    max_recovery_streak = 0
    previous_mode = None
    for row in rows:
        mode = row["mode"]
        if mode == "RECOVER":
            recovery_streak += 1
            max_recovery_streak = max(max_recovery_streak, recovery_streak)
            if previous_mode != "RECOVER":
                recovery_episodes += 1
        else:
            recovery_streak = 0
        previous_mode = mode

    return {
        "samples": len(rows),
        "distance_x_m": round(float(rows[-1]["x_m"]) - float(rows[0]["x_m"]), 3),
        "rms_cross_track_m": round(math.sqrt(sum(error * error for error in errors) / len(errors)), 5),
        "p95_abs_cross_track_m": round(_percentile(errors, 0.95), 5),
        "max_cross_track_m": round(max(errors), 5),
        "rms_sensor_cross_track_m": round(
            math.sqrt(sum(error * error for error in sensor_errors) / len(sensor_errors)), 5
        ),
        "recovery_ratio": round(recovery_count / len(rows), 4),
        "recovery_episodes": recovery_episodes,
        "max_recovery_duration_s": round(max_recovery_streak * dt_s, 3),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seconds", type=float, default=20.0)
    parser.add_argument("--seed", type=int, default=7)
    parser.add_argument("--output", type=Path, default=Path("artifacts/simulation.csv"))
    parser.add_argument("--summary", type=Path, default=Path("artifacts/simulation_summary.json"))
    args = parser.parse_args()

    config = SimConfig(seconds=args.seconds, seed=args.seed)
    rows = simulate(config)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    payload = {
        "simulator_model_version": SIMULATOR_MODEL_VERSION,
        "evidence_type": "deterministic_reference_simulation",
        "physical_evidence": False,
        "physics_validated": False,
        "claim_boundary": (
            "The simulator is a deterministic controls/regression model. Metrics are not measured robot performance."
        ),
        "config": asdict(config),
        "summary": summarize(rows),
    }
    args.summary.parent.mkdir(parents=True, exist_ok=True)
    args.summary.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
