import React, { useState, useEffect, useRef } from 'react'
import { createRoot } from 'react-dom/client'

export default function App() {
  const [status, setStatus] = useState<string>('Disconnected')
  const [messages, setMessages] = useState<string[]>([])
  const [input, setInput] = useState<string>('')
  
  const ws = useRef<WebSocket | null>(null)

  useEffect(() => {
    ws.current = new WebSocket('ws://127.0.0.1:8080')

    ws.current.onopen = () => {
      setStatus('Connected')
    }

    ws.current.onmessage = (event) => {
      setMessages((prev) => [...prev, event.data])
    }

    ws.current.onclose = () => {
      setStatus('Disconnected')
    }

    return () => {
      if (ws.current) {
        ws.current.close()
      }
    }
  }, [])

  const sendMessage = () => {
    if (ws.current && ws.current.readyState === WebSocket.OPEN && input.trim() !== '') {
      ws.current.send(input)
      setInput('')
    }
  }

  return (
    <div style={{ padding: '20px', fontFamily: 'sans-serif' }}>
      <h2>C++ Backend Connection</h2>
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
          Send to Server
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