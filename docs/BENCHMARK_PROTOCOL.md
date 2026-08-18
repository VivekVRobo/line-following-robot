# Benchmark Protocol

Use this before adding performance claims.

## Record conditions
- track geometry and length
- line width/material/color
- floor material/reflectivity
- lighting
- sensor height
- battery chemistry and start voltage
- motor/gearbox, wheel diameter and wheelbase
- controller commit SHA
- calibration constants and PID/speed settings

## Minimum run set
Perform at least 10 trials from the same start point. Keep failed runs in the dataset.

## Recommended metrics
- completion rate
- lap/course time
- mean absolute line error
- RMS and p95 line error
- recovery events and recovery duration
- motor saturation ratio
- battery voltage before/after the set

## Evidence
Store raw serial logs with the experiment or attach them to a release. Publish derived metrics with the exact analysis command. Simulator results must always be labeled **simulation** and never presented as physical-track results.
