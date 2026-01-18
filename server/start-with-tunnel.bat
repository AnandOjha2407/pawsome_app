@echo off
echo Starting DogGPT Backend Server with LocalTunnel...
echo.
echo Starting server on port 3000...
start "DogGPT Server" cmd /k "node server.js"
timeout /t 3 /nobreak >nul 2>&1
echo.
echo Starting LocalTunnel...
echo Your public URL will be shown below:
lt --port 3000 --subdomain doggpt-demo
pause

