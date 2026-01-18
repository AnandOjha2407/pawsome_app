@echo off
echo ========================================
echo   DogGPT Backend Server - Production
echo ========================================
echo.
echo Starting server with LocalTunnel...
echo This will create a public URL for testers
echo.
echo Press Ctrl+C to stop
echo.

REM Start the server in background
start "DogGPT Server" cmd /k "node server.js"

REM Wait for server to start
timeout /t 3 /nobreak >nul 2>&1

REM Start LocalTunnel
echo Starting LocalTunnel...
echo Your public URL: https://doggpt-demo.loca.lt
echo.
lt --port 3000 --subdomain doggpt-demo

pause

