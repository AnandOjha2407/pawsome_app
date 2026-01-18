const express = require('express');
const fetch = require('node-fetch');

const router = express.Router();

router.post('/', async (req, res) => {
  try {
    const { messages } = req.body;
    if (!messages || !Array.isArray(messages)) {
      return res.status(400).json({ error: 'messages required' });
    }

    const provider = (process.env.PROVIDER || 'gemini').toLowerCase();

    if (provider === 'gemini') {
      const key = process.env.GEMINI_API_KEY;
      if (!key) return res.status(500).json({ error: 'GEMINI_API_KEY missing' });

      // Convert messages to Gemini format, preserving conversation history
      const contents = messages.map((msg) => ({
        role: msg.role === 'assistant' ? 'model' : 'user',
        parts: [{ text: msg.content }],
      }));

      const resp = await fetch(
        `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=${key}`,
        {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            contents: contents,
            generationConfig: { 
              temperature: 0.7, 
              maxOutputTokens: 1024,  // Increased from 200
              topP: 0.8,
              topK: 40
            },
          }),
        }
      );

      const data = await resp.json();
      console.log('Gemini response:', JSON.stringify(data, null, 2));

      const reply =
        data?.candidates?.[0]?.content?.parts?.[0]?.text ||
        data?.candidates?.[0]?.output ||
        data?.output_text ||
        'No reply';

      return res.json({ reply });
    }

    return res.status(400).json({ error: 'Unknown provider' });
  } catch (err) {
    console.error('Server Error:', err);
    return res.status(500).json({ error: 'server_error', details: String(err) });
  }
});

module.exports = router;
