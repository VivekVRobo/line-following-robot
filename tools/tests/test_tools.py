import unittest
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import simulate,calibrate_sensors,analyze_telemetry,regression_suite
class ToolTests(unittest.TestCase):
 def test_simulator_is_deterministic(self):
  c=simulate.SimConfig(seconds=.2,seed=123);self.assertEqual(simulate.simulate(c),simulate.simulate(c))
 def test_calibration_detects_high_line_polarity(self):
  g={'floor':{f's{i}':[100,120,110] for i in range(5)},'line':{f's{i}':[800,820,810] for i in range(5)}};lo,hi,p,sep=calibrate_sensors.derive(g);self.assertTrue(p);self.assertTrue(all(h>l for l,h in zip(lo,hi)));self.assertTrue(all(s>600 for s in sep))
 def test_telemetry_metrics(self):
  rows=[{'ms':0,'mode':'TRACK','position':10.,'confidence':.8,'total':1000,'error':10.,'correction':1.,'base':100,'left':101,'right':99,'recovery_phase':0},{'ms':50,'mode':'RECOVER','position':0.,'confidence':0.,'total':0,'error':0.,'correction':0.,'base':0,'left':105,'right':-105,'recovery_phase':1}];m=analyze_telemetry.metrics(rows);self.assertEqual(m['samples'],2);self.assertEqual(m['tracking_ratio'],.5);self.assertEqual(m['mean_abs_error'],10.)
 def test_regression_gate_evaluation(self):
  m={'mean_rms_m':.007,'worst_rms_m':.013,'mean_recovery_ratio':0.,'mean_progress_m':2.1};passed,checks=regression_suite.evaluate(m);self.assertTrue(passed,checks)
if __name__=='__main__':unittest.main()
