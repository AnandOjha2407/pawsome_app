# DogGPT Backend Startup Script
# This script starts the backend server and optionally sets up localtunnel

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DogGPT Backend Server Startup" -ForegroundColor Cyan
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

Write-Host "Starting backend server on port 3000..." -ForegroundColor Green
Write-Host "Server will be available at: http://localhost:3000" -ForegroundColor Green
Write-Host "For physical devices, use: http://192.168.1.4:3000" -ForegroundColor Green
Write-Host ""
Write-Host "Press Ctrl+C to stop the server" -ForegroundColor Yellow
Write-Host ""

# Start the server
node server.js

