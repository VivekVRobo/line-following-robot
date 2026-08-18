#!/usr/bin/env python3
"""Generate per-sensor constants from CSV columns label,s0,s1,s2,s3,s4."""
from __future__ import annotations
import argparse,csv
from pathlib import Path
SENSORS=[f's{i}' for i in range(5)]
def percentile(values,q):
    values=sorted(values)
    if not values: raise ValueError('empty sample set')
    pos=(len(values)-1)*q; lo=int(pos); hi=min(lo+1,len(values)-1); frac=pos-lo
    return values[lo]*(1-frac)+values[hi]*frac
def load(path):
    g={'floor':{s:[] for s in SENSORS},'line':{s:[] for s in SENSORS}}
    with path.open(newline='') as f:
        for row in csv.DictReader(f):
            label=row['label'].strip().lower()
            if label not in g: raise ValueError(f'unknown label: {label}')
            for s in SENSORS:g[label][s].append(int(row[s]))
    if not all(g[label][s] for label in g for s in SENSORS): raise ValueError('need floor and line samples for every sensor')
    return g
def derive(g):
    lows=[]; highs=[]; floor=[]; line=[]
    for s in SENSORS:
        values=g['floor'][s]+g['line'][s]; low=round(percentile(values,.03)); high=round(percentile(values,.97)); high=max(high,low+1); lows.append(low); highs.append(high); floor.append(sum(g['floor'][s])/len(g['floor'][s])); line.append(sum(g['line'][s])/len(g['line'][s]))
    return lows,highs,sum(line)>sum(floor),[abs(a-b) for a,b in zip(line,floor)]
def arr(v):return '{'+', '.join(str(x) for x in v)+'}'
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('csv',type=Path);a=p.parse_args();lo,hi,pol,sep=derive(load(a.csv));print('// Paste into include/config.h');print(f'constexpr std::array<int, SENSOR_COUNT> SENSOR_MIN = {arr(lo)};');print(f'constexpr std::array<int, SENSOR_COUNT> SENSOR_MAX = {arr(hi)};');print(f"constexpr bool LINE_IS_HIGHER_RAW = {'true' if pol else 'false'};");print('// Mean floor/line separation per sensor:',', '.join(f'{x:.1f}' for x in sep))
if __name__=='__main__':main()
