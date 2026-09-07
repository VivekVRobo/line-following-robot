#!/usr/bin/env python3
"""Characterize the reference simulator across deterministic disturbance axes.

This tool intentionally reports a software-model robustness envelope instead of
claiming physical track performance.
"""
from __future__ import annotations

import argparse
import itertools
import json
import math
from collections import defaultdict
from pathlib import Path

import simulate


def percentile(values: list[float], fraction: float) -> float:
    ordered = sorted(values)
    if not ordered:
        return 0.0
    index = min(len(ordered) - 1, max(0, math.ceil(fraction * len(ordered)) - 1))
    return ordered[index]


def scenario_matrix(seconds: float):
    for seed, offset, noise, amplitude, wavelength in itertools.product(
        (1, 7, 19),
        (-0.025, 0.0, 0.025),
        (0.0, 18.0, 36.0),
        (0.03, 0.05, 0.07),
        (0.8, 1.2, 1.8),
    ):
        yield simulate.SimConfig(
            seconds=seconds,
            seed=seed,
            initial_offset_m=offset,
            noise_std_position=noise,
            track_amplitude_m=amplitude,
            track_wavelength_m=wavelength,
        )


def grouped_means(rows: list[dict], key: str) -> dict[str, dict[str, float]]:
    groups: dict[float | int, list[dict]] = defaultdict(list)
    for row in rows:
        groups[row[key]].append(row)
    output = {}
    for value, items in sorted(groups.items(), key=lambda pair: float(pair[0])):
        output[str(value)] = {
            "scenarios": len(items),
            "mean_rms_cross_track_m": sum(item["rms_cross_track_m"] for item in items) / len(items),
            "mean_recovery_ratio": sum(item["recovery_ratio"] for item in items) / len(items),
            "mean_progress_m": sum(item["distance_x_m"] for item in items) / len(items),
        }
    return output


def run(seconds: float) -> dict:
    rows: list[dict] = []
    for config in scenario_matrix(seconds):
        summary = simulate.summarize(simulate.simulate(config))
        rows.append(
            {
                "seed": config.seed,
                "initial_offset_m": config.initial_offset_m,
                "noise_std_position": config.noise_std_position,
                "track_amplitude_m": config.track_amplitude_m,
                "track_wavelength_m": config.track_wavelength_m,
                **summary,
            }
        )

    rms_values = [row["rms_cross_track_m"] for row in rows]
    recovery_values = [row["recovery_ratio"] for row in rows]
    progress_values = [row["distance_x_m"] for row in rows]
    worst_rms = max(rows, key=lambda row: row["rms_cross_track_m"])
    worst_recovery = max(rows, key=lambda row: row["recovery_ratio"])
    lowest_progress = min(rows, key=lambda row: row["distance_x_m"])

    return {
        "schema_version": 1,
        "simulator_model_version": simulate.SIMULATOR_MODEL_VERSION,
        "evidence_type": "deterministic_simulator_robustness_characterization",
        "physical_evidence": False,
        "physics_validated": False,
        "claim_boundary": (
            "The sweep characterizes only the repository's lightweight deterministic model across configured "
            "disturbance axes. It is not a measured robot robustness envelope or hardware PID validation."
        ),
        "scenario_count": len(rows),
        "axes": {
            "seed": [1, 7, 19],
            "initial_offset_m": [-0.025, 0.0, 0.025],
            "noise_std_position": [0.0, 18.0, 36.0],
            "track_amplitude_m": [0.03, 0.05, 0.07],
            "track_wavelength_m": [0.8, 1.2, 1.8],
            "seconds": seconds,
        },
        "aggregate": {
            "mean_rms_cross_track_m": sum(rms_values) / len(rms_values),
            "p95_rms_cross_track_m": percentile(rms_values, 0.95),
            "max_rms_cross_track_m": max(rms_values),
            "mean_recovery_ratio": sum(recovery_values) / len(recovery_values),
            "p95_recovery_ratio": percentile(recovery_values, 0.95),
            "mean_progress_m": sum(progress_values) / len(progress_values),
            "min_progress_m": min(progress_values),
        },
        "extrema": {
            "worst_rms_scenario": worst_rms,
            "worst_recovery_scenario": worst_recovery,
            "lowest_progress_scenario": lowest_progress,
        },
        "sensitivity": {
            "initial_offset_m": grouped_means(rows, "initial_offset_m"),
            "noise_std_position": grouped_means(rows, "noise_std_position"),
            "track_amplitude_m": grouped_means(rows, "track_amplitude_m"),
            "track_wavelength_m": grouped_means(rows, "track_wavelength_m"),
        },
        "scenarios": rows,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seconds", type=float, default=5.0)
    parser.add_argument("--output", type=Path, default=Path("artifacts/robustness-sweep.json"))
    args = parser.parse_args()
    report = run(args.seconds)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"scenario_count": report["scenario_count"], "aggregate": report["aggregate"]}, indent=2))


if __name__ == "__main__":
    main()
