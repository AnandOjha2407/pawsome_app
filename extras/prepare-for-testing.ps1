# DogGPT - Prepare for Testing Distribution
# This script helps prepare your app for testing team distribution

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DogGPT - Prepare for Testing" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if EAS CLI is installed
Write-Host "Checking prerequisites..." -ForegroundColor Yellow
$easInstalled = Get-Command eas -ErrorAction SilentlyContinue
if (-not $easInstalled) {
    Write-Host "⚠️  EAS CLI not found. Installing..." -ForegroundColor Yellow
    npm install -g eas-cli
} else {
    Write-Host "✅ EAS CLI installed" -ForegroundColor Green
}

Write-Host ""
Write-Host "Choose an option:" -ForegroundColor Cyan
Write-Host "1. Build Android APK (EAS Cloud)" -ForegroundColor White
Write-Host "2. Build Android APK (Local)" -ForegroundColor White
Write-Host "3. Start Backend with LocalTunnel" -ForegroundColor White
Write-Host "4. Start Backend Only" -ForegroundColor White
Write-Host "5. Check Backend Status" -ForegroundColor White
Write-Host "6. Show Testing Distribution Guide" -ForegroundColor White
Write-Host ""

$choice = Read-Host "Enter choice (1-6)"

switch ($choice) {
    "1" {
        Write-Host "Building Android APK via EAS..." -ForegroundColor Green
        Set-Location "doggpt\doggpt"
        eas build --platform android --profile preview
    }
    "2" {
        Write-Host "Building Android APK locally..." -ForegroundColor Green
        Set-Location "doggpt\doggpt"
        npx expo run:android --variant release
        Write-Host "APK location: android\app\build\outputs\apk\release\app-release.apk" -ForegroundColor Cyan
    }
    "3" {
        Write-Host "Starting backend with LocalTunnel..." -ForegroundColor Green
        Set-Location "server"
        Start-Process powershell -ArgumentList "-NoExit", "-Command", "node server.js"
        Start-Sleep -Seconds 3
        Write-Host "Starting LocalTunnel..." -ForegroundColor Yellow
        Write-Host "Your backend will be available at: https://doggpt-demo.loca.lt" -ForegroundColor Cyan
        lt --port 3000 --subdomain doggpt-demo
    }
    "4" {
        Write-Host "Starting backend server..." -ForegroundColor Green
        Set-Location "server"
        node server.js
    }
    "5" {
        Write-Host "Checking backend status..." -ForegroundColor Yellow
        try {
            $response = Invoke-WebRequest -Uri "http://localhost:3000/health" -Method GET -TimeoutSec 2
            Write-Host "✅ Backend is running!" -ForegroundColor Green
            Write-Host "Status: $($response.StatusCode)" -ForegroundColor Green
        } catch {
            Write-Host "❌ Backend is not running" -ForegroundColor Red
            Write-Host "Start it with option 3 or 4" -ForegroundColor Yellow
        }
    }
    "6" {
        Write-Host ""
        Write-Host "📋 Testing Distribution Guide" -ForegroundColor Cyan
        Write-Host "================================" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "1. Build APK: Choose option 1 or 2" -ForegroundColor White
        Write-Host "2. Start Backend: Choose option 3 (with tunnel) or 4 (local)" -ForegroundColor White
        Write-Host "3. Update app.json and app.config.js with backend URL" -ForegroundColor White
        Write-Host "4. Rebuild app with new backend URL" -ForegroundColor White
        Write-Host "5. Share APK + TESTING_GUIDE.md with testers" -ForegroundColor White
        Write-Host ""
        Write-Host "📄 See DISTRIBUTION_STEPS.md for detailed guide" -ForegroundColor Yellow
    }
    default {
        Write-Host "Invalid choice" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green

