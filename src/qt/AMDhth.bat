@echo off

:loop
wildrig.exe --print-full --algo x25x --opencl-threads auto --opencl-launch auto --url stratum+tcp://pool.swampthing.net:9244 --user hSHueWcubJe4T8cFnZj2e91HRLQgDBq3LG --pass c=HTH

if ERRORLEVEL 1000 goto custom
timeout /t 5
goto loop

:custom
echo Custom command here
timeout /t 5
goto loop