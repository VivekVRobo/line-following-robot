#!/usr/bin/env python3
"""Summarize firmware T,... telemetry into objective tuning metrics."""
from __future__ import annotations

import argparse
import math
from pathlib import Path

FIELDS = [
    "ms", "mode", "position", "confidence", "total", "error",
    "correction", "base", "left", "right", "recovery_phase",
]
INTEGER_FIELDS = ("ms", "total", "base", "left", "right", "recovery_phase")
FLOAT_FIELDS = ("position", "confidence", "error", "correction")
VALID_MODES = {"TRACK", "RECOVER"}


def load(path):
    """Load T records and reject malformed telemetry instead of silently dropping it."""
    rows = []
    with path.open(encoding="utf-8") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = raw_line.strip()
            if not line.startswith("T,"):
                continue

            parts = line.split(",")[1:]
            if len(parts) != len(FIELDS):
                raise ValueError(
                    f"{path}:{line_number}: expected {len(FIELDS)} fields after 'T,', "
                    f"got {len(parts)}"
                )

            record = dict(zip(FIELDS, parts))
            if record["mode"] not in VALID_MODES:
                raise ValueError(
                    f"{path}:{line_number}: unknown telemetry mode {record['mode']!r}; "
                    f"expected TRACK or RECOVER"
                )

            for field in INTEGER_FIELDS:
                try:
                    value = float(record[field])
                except ValueError as exc:
                    raise ValueError(
                        f"{path}:{line_number}: field {field!r} must be a finite integer"
                    ) from exc
                if not math.isfinite(value) or not value.is_integer():
                    raise ValueError(
                        f"{path}:{line_number}: field {field!r} must be a finite integer"
                    )
                record[field] = int(value)

            for field in FLOAT_FIELDS:
                try:
                    value = float(record[field])
                except ValueError as exc:
                    raise ValueError(
                        f"{path}:{line_number}: field {field!r} must be a finite number"
                    ) from exc
                if not math.isfinite(value):
                    raise ValueError(
                        f"{path}:{line_number}: field {field!r} must be a finite number"
                    )
                record[field] = value

            if not 0.0 <= record["confidence"] <= 1.0:
                raise ValueError(
                    f"{path}:{line_number}: field 'confidence' must be between 0 and 1"
                )
            rows.append(record)

    if not rows:
        raise ValueError(f"{path}: no telemetry records found (expected lines beginning 'T,')")
    return rows


def percentile(values, q):
    values = sorted(values)
    if not values:
        return 0.0
    return values[min(len(values) - 1, round((len(values) - 1) * q))]


def metrics(rows, max_pwm=220):
    if not rows:
        raise ValueError("no telemetry samples; refusing to report zero-valued metrics")
    if max_pwm <= 0:
        raise ValueError("max_pwm must be greater than zero")

    tracking = [row for row in rows if row["mode"] == "TRACK"]
    errors = [abs(row["error"]) for row in tracking]
    saturated = [
        row for row in rows
        if max(abs(row["left"]), abs(row["right"])) >= max_pwm
    ]
    duration = (rows[-1]["ms"] - rows[0]["ms"]) / 1000 if len(rows) > 1 else 0.0

    return {
        "samples": len(rows),
        "duration_s": round(duration, 3),
        "tracking_ratio": round(len(tracking) / len(rows), 4),
        "recovery_ratio": round(1 - len(tracking) / len(rows), 4),
        "mean_abs_error": round(sum(errors) / len(errors), 2) if errors else 0.0,
        "rms_error": round(math.sqrt(sum(error * error for error in errors) / len(errors)), 2)
        if errors else 0.0,
        "p95_abs_error": round(percentile(errors, 0.95), 2),
        "mean_confidence": round(
            sum(row["confidence"] for row in tracking) / len(tracking), 3
        ) if tracking else 0.0,
        "motor_saturation_ratio": round(len(saturated) / len(rows), 4),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--markdown", action="store_true")
    args = parser.parse_args()
    try:
        result = metrics(load(args.log))
    except ValueError as exc:
        parser.error(str(exc))

    if args.markdown:
        print("| Metric | Value |\n|---|---:|")
        for key, value in result.items():
            print(f"| {key} | {value} |")
    else:
        for key, value in result.items():
            print(f"{key}: {value}")


if __name__ == "__main__":
    main()
