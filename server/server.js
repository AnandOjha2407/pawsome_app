// server/server.js
require('dotenv').config();
const express = require('express');
const cors = require('cors');

const chatRoute = require('./routes/chat');

const app = express();
app.use(cors());
app.use(express.json({ limit: '1mb' }));

// --- Health check (for quick network testing) ---
app.get('/health', (req, res) => res.json({ ok: true }));

// Chat API
app.use('/api/chat', chatRoute);
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Chat proxy listening on ${PORT}`));
