#!/usr/bin/env python3
"""Multi-scenario regression gate for the reference simulator.

Thresholds detect regressions of the deterministic simulator/controller model
only; they are not physical robot guarantees or PID tuning targets.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path

import simulate


@dataclass(frozen=True)
class Gate:
    # Version-2 regression envelopes were reset after correcting the simulated
    # sensor location from robot-center sampling to a forward sensor bar.
    max_mean_rms_m: float = 0.024
    max_worst_rms_m: float = 0.028
    max_mean_recovery_ratio: float = 0.08
    min_mean_progress_m: float = 1.5


def scenarios(seconds: float = 8.0):
    for seed in (1, 7, 19, 41):
        for offset in (-0.025, -0.012, 0.0, 0.012, 0.025):
            yield simulate.SimConfig(seconds=seconds, seed=seed, initial_offset_m=offset)


def run(seconds: float = 8.0) -> list[dict]:
    output: list[dict] = []
    for config in scenarios(seconds):
        summary = simulate.summarize(simulate.simulate(config))
        summary["seed"] = config.seed
        summary["initial_offset_m"] = config.initial_offset_m
        output.append(summary)
    return output


def aggregate(results: list[dict]) -> dict:
    return {
        "scenario_count": len(results),
        "mean_rms_m": sum(item["rms_cross_track_m"] for item in results) / len(results),
        "worst_rms_m": max(item["rms_cross_track_m"] for item in results),
        "mean_recovery_ratio": sum(item["recovery_ratio"] for item in results) / len(results),
        "mean_progress_m": sum(item["distance_x_m"] for item in results) / len(results),
        "total_recovery_episodes": sum(item["recovery_episodes"] for item in results),
        "worst_recovery_duration_s": max(item["max_recovery_duration_s"] for item in results),
    }


def evaluate(metrics: dict, gate: Gate = Gate()) -> tuple[bool, dict[str, bool]]:
    checks = {
        "mean_rms": metrics["mean_rms_m"] <= gate.max_mean_rms_m,
        "worst_rms": metrics["worst_rms_m"] <= gate.max_worst_rms_m,
        "recovery_ratio": metrics["mean_recovery_ratio"] <= gate.max_mean_recovery_ratio,
        "progress": metrics["mean_progress_m"] >= gate.min_mean_progress_m,
    }
    return all(checks.values()), checks


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seconds", type=float, default=8.0)
    parser.add_argument("--output", type=Path, default=Path("artifacts/regression.json"))
    args = parser.parse_args()

    results = run(args.seconds)
    metrics = aggregate(results)
    passed, checks = evaluate(metrics)
    payload = {
        "schema_version": 2,
        "simulator_model_version": simulate.SIMULATOR_MODEL_VERSION,
        "evidence_type": "deterministic_simulation_regression",
        "physical_evidence": False,
        "physics_validated": False,
        "claim_boundary": (
            "Regression thresholds are internal software-model envelopes and are not measured track-performance limits."
        ),
        "passed": passed,
        "gate": asdict(Gate()),
        "checks": checks,
        "aggregate": metrics,
        "scenarios": results,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"passed": passed, "checks": checks, "aggregate": metrics}, indent=2))
    raise SystemExit(0 if passed else 1)


if __name__ == "__main__":
    main()
