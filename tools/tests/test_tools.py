import math
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import analyze_telemetry
import calibrate_sensors
import regression_suite
import robustness_sweep
import simulate


class ToolTests(unittest.TestCase):
    def test_simulator_is_deterministic(self):
        config = simulate.SimConfig(seconds=0.2, seed=123)
        self.assertEqual(simulate.simulate(config), simulate.simulate(config))

    def test_forward_sensor_offset_makes_heading_affect_line_sample(self):
        config = simulate.SimConfig(
            track_amplitude_m=0.0,
            sensor_forward_offset_m=0.045,
        )
        error_heading_zero = simulate.sensor_lateral_error(0.0, 0.02, 0.0, config)
        error_heading_quarter_turn = simulate.sensor_lateral_error(0.0, 0.02, math.pi / 2, config)
        self.assertNotAlmostEqual(error_heading_zero, error_heading_quarter_turn)

    def test_zero_sensor_offset_reproduces_center_sample_for_flat_track(self):
        config = simulate.SimConfig(track_amplitude_m=0.0, sensor_forward_offset_m=0.0)
        self.assertAlmostEqual(simulate.sensor_lateral_error(0.0, 0.02, 0.0, config), 0.02)
        self.assertAlmostEqual(simulate.sensor_lateral_error(0.0, 0.02, math.pi / 2, config), 0.02)

    def test_summary_reports_recovery_episodes(self):
        rows = [
            {"ms": 0, "mode": "TRACK", "cross_track_m": 0.0, "sensor_cross_track_m": 0.0, "x_m": 0.0},
            {"ms": 10, "mode": "RECOVER", "cross_track_m": 0.01, "sensor_cross_track_m": 0.01, "x_m": 0.0},
            {"ms": 20, "mode": "RECOVER", "cross_track_m": 0.01, "sensor_cross_track_m": 0.01, "x_m": 0.0},
            {"ms": 30, "mode": "TRACK", "cross_track_m": 0.0, "sensor_cross_track_m": 0.0, "x_m": 0.01},
            {"ms": 40, "mode": "RECOVER", "cross_track_m": 0.01, "sensor_cross_track_m": 0.01, "x_m": 0.01},
        ]
        summary = simulate.summarize(rows)
        self.assertEqual(summary["recovery_episodes"], 2)
        self.assertAlmostEqual(summary["max_recovery_duration_s"], 0.02)

    def test_calibration_detects_high_line_polarity(self):
        groups = {
            "floor": {f"s{i}": [100, 120, 110] for i in range(5)},
            "line": {f"s{i}": [800, 820, 810] for i in range(5)},
        }
        low, high, polarity, separation = calibrate_sensors.derive(groups)
        self.assertTrue(polarity)
        self.assertTrue(all(hi > lo for lo, hi in zip(low, high)))
        self.assertTrue(all(value > 600 for value in separation))

    def test_telemetry_loader_accepts_sample_log_and_ignores_preamble(self):
        sample = Path(__file__).resolve().parents[2] / "examples" / "sample_telemetry.log"
        rows = analyze_telemetry.load(sample)
        self.assertEqual(len(rows), 6)
        self.assertEqual(rows[0]["mode"], "TRACK")

    def test_telemetry_loader_rejects_truncated_record_with_line_number(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "truncated.log"
            path.write_text("serial banner\nT,1000,TRACK\n", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, r"truncated\.log:2: expected 11 fields"):
                analyze_telemetry.load(path)

    def test_telemetry_loader_rejects_non_finite_numeric_value(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "non-finite.log"
            path.write_text(
                "T,1000,TRACK,0.0,0.8,100,nan,0.0,100,100,100,0\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, r"field 'error' must be a finite number"):
                analyze_telemetry.load(path)

    def test_telemetry_loader_rejects_unknown_mode(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "unknown-mode.log"
            path.write_text(
                "T,1000,PAUSED,0.0,0.8,100,0.0,0.0,100,100,100,0\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, "unknown telemetry mode"):
                analyze_telemetry.load(path)

    def test_telemetry_loader_rejects_log_without_records(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "empty.log"
            path.write_text("line-follower,v2,SAFE-STOP\n# telemetry header\n", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "no telemetry records found"):
                analyze_telemetry.load(path)

    def test_telemetry_metrics_rejects_empty_samples(self):
        with self.assertRaisesRegex(ValueError, "refusing to report zero-valued metrics"):
            analyze_telemetry.metrics([])

    def test_telemetry_metrics(self):
        rows = [
            {
                "ms": 0,
                "mode": "TRACK",
                "position": 10.0,
                "confidence": 0.8,
                "total": 1000,
                "error": 10.0,
                "correction": 1.0,
                "base": 100,
                "left": 101,
                "right": 99,
                "recovery_phase": 0,
            },
            {
                "ms": 50,
                "mode": "RECOVER",
                "position": 0.0,
                "confidence": 0.0,
                "total": 0,
                "error": 0.0,
                "correction": 0.0,
                "base": 0,
                "left": 105,
                "right": -105,
                "recovery_phase": 1,
            },
        ]
        metrics = analyze_telemetry.metrics(rows)
        self.assertEqual(metrics["samples"], 2)
        self.assertEqual(metrics["tracking_ratio"], 0.5)
        self.assertEqual(metrics["mean_abs_error"], 10.0)

    def test_regression_gate_evaluation(self):
        metrics = {
            "mean_rms_m": 0.020,
            "worst_rms_m": 0.025,
            "mean_recovery_ratio": 0.03,
            "mean_progress_m": 1.8,
        }
        passed, checks = regression_suite.evaluate(metrics)
        self.assertTrue(passed, checks)

    def test_robustness_matrix_has_expected_size(self):
        configs = list(robustness_sweep.scenario_matrix(seconds=0.1))
        self.assertEqual(len(configs), 243)
        self.assertTrue(all(config.seconds == 0.1 for config in configs))


if __name__ == "__main__":
    unittest.main()
