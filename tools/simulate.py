#!/usr/bin/env python3
"""Dependency-free differential-drive line follower reference simulator.

A repeatable tuning/regression aid; it is not a physics-validated digital twin.
"""
from __future__ import annotations
import argparse,csv,json,math,random
from dataclasses import dataclass
from pathlib import Path

@dataclass
class SimConfig:
    dt:float=0.01; seconds:float=20.0; wheelbase_m:float=0.12; max_speed_mps:float=0.60
    max_pwm:int=220; sensor_half_width_m:float=0.035; track_amplitude_m:float=0.05
    track_wavelength_m:float=1.2; noise_std_position:float=18.0; initial_offset_m:float=0.02; seed:int=7

class PID:
    def __init__(self,kp=.085,ki=.0008,kd=.30,output_limit=190.):
        self.kp,self.ki,self.kd,self.output_limit=kp,ki,kd,output_limit; self.reset()
    def reset(self): self.i=0.; self.prev=None; self.derivative=0.
    def update(self,error,dt):
        raw=0. if self.prev is None else (error-self.prev)/dt
        self.derivative=.35*raw+.65*self.derivative
        candidate_i=max(-4500.,min(4500.,self.i+error*dt))
        candidate=self.kp*error+self.ki*candidate_i+self.kd*self.derivative
        limited=max(-self.output_limit,min(self.output_limit,candidate))
        if not (abs(candidate-limited)>1e-9 and candidate*error>0): self.i=candidate_i
        self.prev=error
        return max(-self.output_limit,min(self.output_limit,self.kp*error+self.ki*self.i+self.kd*self.derivative))

def track_y(x,c): return c.track_amplitude_m*math.sin(2*math.pi*x/c.track_wavelength_m)
def adaptive_speed(position,confidence):
    slowdown=.48*min(1.,abs(position)/2000.)+.30*(1.-confidence)
    return round(max(90.,145.*(1.-min(.85,slowdown))))
def mix(base,correction,max_pwm):
    left,right=base+correction,base-correction; peak=max(abs(left),abs(right))
    if peak>max_pwm:
        scale=max_pwm/peak; left*=scale; right*=scale
    return round(left),round(right)

def simulate(c:SimConfig):
    random.seed(c.seed); pid=PID(); x,y,heading=0.,c.initial_offset_m,0.; rows=[]; recovering=False; last_direction=1; recovery_started=0.
    for step in range(int(c.seconds/c.dt)):
        t=step*c.dt; lateral=y-track_y(x,c); visible=abs(lateral)<=c.sensor_half_width_m
        raw_position=(lateral/c.sensor_half_width_m)*2000.; position=raw_position+random.gauss(0,c.noise_std_position)
        confidence=max(0.,min(1.,1.-abs(lateral)/c.sensor_half_width_m)) if visible else 0.
        if visible:
            if recovering: pid.reset(); recovering=False
            error=position
            if error>70:last_direction=1
            elif error<-70:last_direction=-1
            base=adaptive_speed(error,confidence); correction=pid.update(error,c.dt); left,right=mix(base,correction,c.max_pwm); mode='TRACK'; phase=0
        else:
            if not recovering: recovering=True; recovery_started=t; pid.reset()
            elapsed=t-recovery_started
            if elapsed<.35: direction,speed,phase=last_direction,105,1
            else:
                segment=int((elapsed-.35)/.60); direction=last_direction if segment%2==0 else -last_direction; speed,phase=125,2+(segment%2)
            left,right=direction*speed,-direction*speed; base,correction,error,mode=0,0.,0.,'RECOVER'
        lv=(left/c.max_pwm)*c.max_speed_mps; rv=(right/c.max_pwm)*c.max_speed_mps; v=.5*(lv+rv); omega=(rv-lv)/c.wheelbase_m
        x+=v*math.cos(heading)*c.dt; y+=v*math.sin(heading)*c.dt; heading+=omega*c.dt
        rows.append({'ms':round(t*1000),'mode':mode,'position':round(position if visible else 0.,2),'confidence':round(confidence,4),'total':round(confidence*5000),'error':round(error,2),'correction':round(correction,2),'base':base,'left':left,'right':right,'recovery_phase':phase,'x_m':round(x,5),'y_m':round(y,5),'track_y_m':round(track_y(x,c),5),'cross_track_m':round(y-track_y(x,c),5)})
    return rows

def summarize(rows):
    errors=[abs(r['cross_track_m']) for r in rows]; recovering=sum(r['mode']=='RECOVER' for r in rows)
    return {'samples':len(rows),'distance_x_m':round(rows[-1]['x_m']-rows[0]['x_m'],3) if rows else 0.,'rms_cross_track_m':round(math.sqrt(sum(e*e for e in errors)/len(errors)),5) if errors else 0.,'max_cross_track_m':round(max(errors),5) if errors else 0.,'recovery_ratio':round(recovering/len(rows),4) if rows else 0.}

def main():
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('--seconds',type=float,default=20.); p.add_argument('--seed',type=int,default=7); p.add_argument('--output',type=Path,default=Path('artifacts/simulation.csv')); p.add_argument('--summary',type=Path,default=Path('artifacts/simulation_summary.json')); a=p.parse_args()
    rows=simulate(SimConfig(seconds=a.seconds,seed=a.seed)); a.output.parent.mkdir(parents=True,exist_ok=True)
    with a.output.open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)
    result=summarize(rows); a.summary.parent.mkdir(parents=True,exist_ok=True); a.summary.write_text(json.dumps(result,indent=2)+'\n'); print(json.dumps(result,indent=2))
if __name__=='__main__': main()
