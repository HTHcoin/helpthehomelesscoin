@echo off
title MINING HTH X25x
t-rex.exe -a x25x -o stratum+tcp://pool.swampthing.net:3650 -u yU8w3AFxaKMVNx7q1PNKChrEMFA9CHJ7Am -p c=HTHT
wildrig.exe --algo x25x --opencl-threads auto --opencl-launch auto --url stratum+tcp://pool.swampthing.net:3650 --user yU8w3AFxaKMVNx7q1PNKChrEMFA9CHJ7Am  --pass c=HTHT
pause
