# DogGPT Backend Startup Script with LocalTunnel
# This script starts the backend server and localtunnel for public access

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DogGPT Backend Server + Tunnel" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$serverPath = Join-Path $PSScriptRoot "server"

if (-not (Test-Path $serverPath)) {
    Write-Host "Error: Server directory not found!" -ForegroundColor Red
    exit 1
}

Set-Location $serverPath

# Check if .env exists
if (-not (Test-Path ".env")) {
    Write-Host "Warning: .env file not found. Creating one..." -ForegroundColor Yellow
    @"
PROVIDER=gemini
GEMINI_API_KEY=your_api_key_here
PORT=3000
"@ | Out-File -FilePath ".env" -Encoding utf8
    Write-Host "Please update .env with your GEMINI_API_KEY" -ForegroundColor Yellow
}

Write-Host "Starting backend server..." -ForegroundColor Green
$serverJob = Start-Job -ScriptBlock {
    Set-Location $using:serverPath
    node server.js
}

Start-Sleep -Seconds 3

Write-Host "Starting LocalTunnel..." -ForegroundColor Green
Write-Host "Your public URL will be: https://doggpt-demo.loca.lt" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press Ctrl+C to stop both server and tunnel" -ForegroundColor Yellow
Write-Host ""

try {
    lt --port 3000 --subdomain doggpt-demo
} finally {
    Write-Host "Stopping server..." -ForegroundColor Yellow
    Stop-Job $serverJob
    Remove-Job $serverJob
}

