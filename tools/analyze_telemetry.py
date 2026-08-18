#!/usr/bin/env python3
"""Summarize firmware T,... telemetry into objective tuning metrics."""
from __future__ import annotations
import argparse,math
from pathlib import Path
FIELDS=['ms','mode','position','confidence','total','error','correction','base','left','right','recovery_phase']
def load(path):
    rows=[]
    with path.open() as f:
        for line in f:
            line=line.strip()
            if not line.startswith('T,'):continue
            parts=line.split(',')[1:]
            if len(parts)!=len(FIELDS):continue
            r=dict(zip(FIELDS,parts))
            for k in ['ms','total','base','left','right','recovery_phase']:r[k]=int(float(r[k]))
            for k in ['position','confidence','error','correction']:r[k]=float(r[k])
            rows.append(r)
    return rows
def percentile(values,q):
    values=sorted(values)
    if not values:return 0.
    return values[min(len(values)-1,round((len(values)-1)*q))]
def metrics(rows,max_pwm=220):
    tracking=[r for r in rows if r['mode']=='TRACK']; errors=[abs(r['error']) for r in tracking]; sat=[r for r in rows if max(abs(r['left']),abs(r['right']))>=max_pwm]; duration=(rows[-1]['ms']-rows[0]['ms'])/1000 if len(rows)>1 else 0.
    return {'samples':len(rows),'duration_s':round(duration,3),'tracking_ratio':round(len(tracking)/len(rows),4) if rows else 0.,'recovery_ratio':round(1-len(tracking)/len(rows),4) if rows else 0.,'mean_abs_error':round(sum(errors)/len(errors),2) if errors else 0.,'rms_error':round(math.sqrt(sum(e*e for e in errors)/len(errors)),2) if errors else 0.,'p95_abs_error':round(percentile(errors,.95),2),'mean_confidence':round(sum(r['confidence'] for r in tracking)/len(tracking),3) if tracking else 0.,'motor_saturation_ratio':round(len(sat)/len(rows),4) if rows else 0.}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('log',type=Path);p.add_argument('--markdown',action='store_true');a=p.parse_args();m=metrics(load(a.log))
    if a.markdown:
        print('| Metric | Value |\n|---|---:|');[print(f'| {k} | {v} |') for k,v in m.items()]
    else:[print(f'{k}: {v}') for k,v in m.items()]
if __name__=='__main__':main()
