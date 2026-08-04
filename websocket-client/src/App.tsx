import React, { useState } from 'react'
import { createRoot } from 'react-dom/client'

export default function App() {
  const [status, setStatus] = useState<string>('Ready')
  const [messages, setMessages] = useState<string[]>([])
  const [input, setInput] = useState<string>('')

  const sendMessage = async () => {
    if (input.trim() === '') return

    const payload = input
    setInput('')
    setStatus('Sending...')

    try {
      const response = await fetch('http://127.0.0.1:8080', {
        method: 'POST',
        headers: {
          'Content-Type': 'text/plain'
        },
        body: payload
      })

      if (!response.ok) {
        throw new Error(`HTTP error: ${response.status}`)
      }

      const reply = await response.text()
      setMessages((prev) => [...prev, reply])
      setStatus('Ready')
    } catch (error) {
      setMessages((prev) => [...prev, 'Failed to connect or receive response'])
      setStatus('Error')
    }
  }

  return (
    <div style={{ padding: '20px', fontFamily: 'sans-serif' }}>
      <h2>C++ Backend Connection (HTTP)</h2>
      <p>Status: <strong>{status}</strong></p>
      
      <div style={{ marginBottom: '20px' }}>
        <input
          type="text"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && sendMessage()}
          style={{ padding: '5px', marginRight: '10px', width: '250px' }}
        />
        <button onClick={sendMessage} style={{ padding: '5px 15px' }}>
          Send HTTP Request
        </button>
      </div>

      <div style={{ border: '1px solid silver', padding: '10px', minHeight: '150px' }}>
        <h4 style={{ marginTop: 0 }}>Server Responses:</h4>
        <ul style={{ listStyleType: 'none', padding: 0 }}>
          {messages.map((msg, index) => (
            <li key={index} style={{ padding: '4px 0', borderBottom: '1px solid lightgray' }}>
              {msg}
            </li>
          ))}
        </ul>
      </div>
    </div>
  )
}

const rootElement = document.getElementById('root')
if (rootElement) {
  createRoot(rootElement).render(<App />)
}