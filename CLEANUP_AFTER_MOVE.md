# Cleanup Steps After Moving Project from D to C Drive

## Your Project Structure
- Root: `C:\Users\Anand Ojha\NOTES\doggpt`
- Project: `C:\Users\Anand Ojha\NOTES\doggpt\doggpt`

## Steps to Fix EAS Build Scanning Issue

### 1. Navigate to Project Directory
```powershell
cd "C:\Users\Anand Ojha\NOTES\doggpt\doggpt"
```

### 2. Clean Expo Cache
```powershell
Remove-Item -Recurse -Force .expo -ErrorAction SilentlyContinue
```

### 3. Clean Node Modules (Remove and Reinstall)
```powershell
Remove-Item -Recurse -Force node_modules -ErrorAction SilentlyContinue
Remove-Item -Force package-lock.json -ErrorAction SilentlyContinue
npm install
```

### 4. Clean Android Build Cache
```powershell
Remove-Item -Recurse -Force android\build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force android\.gradle -ErrorAction SilentlyContinue
```

### 5. Build with EAS_NO_VCS (Recommended)
This prevents EAS from scanning version control directories:
```powershell
cd "C:\Users\Anand Ojha\NOTES\doggpt\doggpt"
$env:EAS_NO_VCS=1
npx eas build -p android --profile preview
```

### 6. Alternative: Check for Symlinks
If the above doesn't work, check for symlinks that might point to AppData:
```powershell
cd "C:\Users\Anand Ojha\NOTES\doggpt\doggpt"
Get-ChildItem -Recurse -Force | Where-Object { $_.LinkType -ne $null } | Select-Object FullName, LinkType
```

## Important Notes
- Always run EAS build commands from `C:\Users\Anand Ojha\NOTES\doggpt\doggpt` (the project root)
- The `.easignore` file is already in the project directory and should exclude problematic paths
- Using `EAS_NO_VCS=1` is the most reliable solution for this issue
