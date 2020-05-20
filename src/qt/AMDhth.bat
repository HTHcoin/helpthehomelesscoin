@echo off

:loop
wildrig.exe --print-full --algo x25x --opencl-threads auto --opencl-launch auto --url stratum+tcp://pool.swampthing.net:3650 --user yU8w3AFxaKMVNx7q1PNKChrEMFA9CHJ7Am --pass c=HTHT

if ERRORLEVEL 1000 goto custom
timeout /t 5
goto loop

:custom
echo Custom command here
timeout /t 5
goto loop