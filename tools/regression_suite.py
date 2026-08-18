#!/usr/bin/env python3
"""Multi-scenario regression gate for the reference simulator.

Thresholds detect simulator regressions only; they are not physical robot guarantees.
"""
from __future__ import annotations
import argparse,json
from dataclasses import dataclass
from pathlib import Path
import simulate
@dataclass
class Gate:
    max_mean_rms_m:float=.012; max_worst_rms_m:float=.020; max_mean_recovery_ratio:float=.08; min_mean_progress_m:float=1.5
def scenarios(seconds=8.):
    for seed in (1,7,19,41):
        for offset in (-.025,-.012,0.,.012,.025): yield simulate.SimConfig(seconds=seconds,seed=seed,initial_offset_m=offset)
def run(seconds=8.):
    out=[]
    for c in scenarios(seconds):
        s=simulate.summarize(simulate.simulate(c));s['seed']=c.seed;s['initial_offset_m']=c.initial_offset_m;out.append(s)
    return out
def aggregate(r):
    return {'scenario_count':len(r),'mean_rms_m':sum(x['rms_cross_track_m'] for x in r)/len(r),'worst_rms_m':max(x['rms_cross_track_m'] for x in r),'mean_recovery_ratio':sum(x['recovery_ratio'] for x in r)/len(r),'mean_progress_m':sum(x['distance_x_m'] for x in r)/len(r)}
def evaluate(m,g=Gate()):
    checks={'mean_rms':m['mean_rms_m']<=g.max_mean_rms_m,'worst_rms':m['worst_rms_m']<=g.max_worst_rms_m,'recovery_ratio':m['mean_recovery_ratio']<=g.max_mean_recovery_ratio,'progress':m['mean_progress_m']>=g.min_mean_progress_m};return all(checks.values()),checks
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--seconds',type=float,default=8.);p.add_argument('--output',type=Path,default=Path('artifacts/regression.json'));a=p.parse_args();r=run(a.seconds);m=aggregate(r);passed,checks=evaluate(m);payload={'passed':passed,'checks':checks,'aggregate':m,'scenarios':r};a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(payload,indent=2)+'\n');print(json.dumps({'passed':passed,'checks':checks,'aggregate':m},indent=2));raise SystemExit(0 if passed else 1)
if __name__=='__main__':main()
