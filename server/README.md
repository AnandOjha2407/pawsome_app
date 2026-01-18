# DogGPT Backend Server

Backend server for the DogGPT chatbot functionality.

## Quick Start

### Option 1: Simple Start (Local Network)
```powershell
# From the project root
.\start-backend.ps1
```

This starts the server on `http://localhost:3000` and makes it available on your local network at `http://192.168.1.4:3000`

### Option 2: With Public Tunnel (for remote access)
```powershell
# From the project root
.\start-backend-tunnel.ps1
```

This starts the server and creates a public URL via LocalTunnel: `https://doggpt-demo.loca.lt`

### Option 3: Manual Start
```powershell
cd server
node server.js
```

## Configuration

The server requires a `.env` file with:
```
PROVIDER=gemini
GEMINI_API_KEY=your_api_key_here
PORT=3000
```

## Keeping Server Running

### Windows (PowerShell)
The server will run until you close the terminal or press Ctrl+C.

### To keep it running in background:
1. Use Task Scheduler to run `start-backend.ps1` on startup
2. Or use a process manager like PM2:
   ```powershell
   npm install -g pm2
   pm2 start server.js --name doggpt-backend
   pm2 save
   pm2 startup
   ```

### Using PM2 (Recommended for Production)
```powershell
npm install -g pm2
cd server
pm2 start server.js --name doggpt-backend
pm2 save  # Save process list
pm2 startup  # Generate startup script
```

## API Endpoints

- `GET /health` - Health check
- `POST /api/chat` - Chat endpoint (requires `messages` array in body)

## Troubleshooting

1. **Port 3000 already in use**: Change PORT in `.env` file
2. **API Key error**: Make sure GEMINI_API_KEY is set in `.env`
3. **Network issues**: Ensure your device and computer are on the same WiFi network
4. **Android Emulator**: Use `10.0.2.2:3000` instead of local IP
5. **Physical Device**: Use your computer's local IP (e.g., `192.168.1.4:3000`)

