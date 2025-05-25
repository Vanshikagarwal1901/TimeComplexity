import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import axios from 'axios';
import {
  LineChart, Line, XAxis, YAxis, Tooltip, CartesianGrid, ResponsiveContainer
} from 'recharts';

function SecondPage() {
  const [code, setCode] = useState(`write your code here\n}`);
  const languages = ['C', 'C++', 'Python', 'Java'];
  const [selectedLanguage, setSelectedLanguage] = useState('C');
  const [timeComplexity, setTimeComplexity] = useState(null);

  const handleSubmit = () => {
    axios.post('http://127.0.0.1:8000/analyze_code/', {
      code: code,
      language: selectedLanguage
    }, {
      headers: { 'Content-Type': 'application/json' }
    })
    .then(response => {
      setTimeComplexity(response.data.time_complexity);
    })
    .catch(error => {
      console.error('Error submitting code:', error);
    });
  };

  const complexities = ['O(1)', 'O(log n)', 'O(n)', 'O(n log n)', 'O(n^2)', 'O(n^3)', 'O(2^n)', 'O(n!)'];
  const colors = {
    'O(1)': '#2ecc71',
    'O(log n)': '#3498db',
    'O(n)': '#9b59b6',
    'O(n log n)': '#f39c12',
    'O(n^2)': '#e74c3c',
    'O(n^3)': '#1abc9c',
    'O(2^n)': '#34495e',
    'O(n!)': '#8e44ad'
  };

const normalize = (arr) => {
  const max = Math.max(...arr);
  return arr.map((val) => val / max);
};

const factorial = (n) => n <= 1 ? 1 : n * factorial(n - 1);

const generateComplexityData = () => {
  const raw = {
    n: [],
    'O(1)': [],
    'O(log n)': [],
    'O(n)': [],
    'O(n log n)': [],
    'O(n^2)': [],
    'O(n^3)': [],
    'O(2^n)': [],
    'O(n!)': [],
  };

  for (let n = 1; n <= 10; n++) {
    raw.n.push(n);
    raw['O(1)'].push(1);
    raw['O(log n)'].push(Math.log2(n));
    raw['O(n)'].push(n);
    raw['O(n log n)'].push(n * Math.log2(n));
    raw['O(n^2)'].push(n ** 2);
    raw['O(n^3)'].push(n ** 3);
    raw['O(2^n)'].push(2 ** n);
    raw['O(n!)'].push(factorial(n));
  }

  const normalized = {};
  for (const key in raw) {
    normalized[key] = normalize(raw[key]);
  }

  const final = [];
  for (let i = 0; i < raw.n.length; i++) {
    const entry = { n: raw.n[i] };
    for (const key in normalized) {
      if (key !== 'n') entry[key] = normalized[key][i];
    }
    final.push(entry);
  }

  return final;
};


  const data = generateComplexityData();

  return (
    <div style={{
      height: '100vh',
      width: '100vw',
      overflow: 'hidden',
      backgroundColor: '#F4F9E9',
      padding: '10px',
      boxSizing: 'border-box'
    }}>
      <div style={{
        display: 'flex',
        gap: '10px',
        height: '100%',
        width: '100%'
      }}>

        {/* Code Editor Panel */}
        <div style={{
          width: '70%',
          display: 'flex',
          flexDirection: 'column',
          borderRadius: '25px',
          border: '3px solid #153243',
          overflow: 'hidden',
          boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
        }}>
          <div style={{
            backgroundColor: '#153243',
            color: 'white',
            textAlign: 'left',
            padding: '10px 0px 0px 10px'
          }}>
            <div style={{ fontFamily: 'heyyy', fontSize: '24px', fontWeight: 'bold' }}>CODE</div>
          </div>

          <div style={{ display: 'flex', width: '100%' }}>
            {languages.map((lang) => (
              <div
                key={lang}
                style={{
                  flex: '1',
                  padding: '8px',
                  textAlign: 'center',
                  cursor: 'pointer',
                  backgroundColor: selectedLanguage === lang ? '#bdc3c7' : '#e0e0e0',
                  borderRight: '1px solid #a0a0a0'
                }}
                onClick={() => setSelectedLanguage(lang)}
              >
                {lang}
              </div>
            ))}
          </div>

          <div style={{
            position: 'relative',
            flexGrow: 1,
            backgroundColor: '#2c4453'
          }}>
            <textarea
              style={{
                width: '100%',
                height: '100%',
                padding: '15px',
                fontFamily: 'monospace',
                fontSize: '14px',
                backgroundColor: '#EEF0EB',
                border: 'none',
                resize: 'none',
                outline: 'none',
                boxSizing: 'border-box'
              }}
              value={code}
              onChange={(e) => setCode(e.target.value)}
            />

            <button
              style={{
                position: 'absolute',
                bottom: '20px',
                right: '20px',
                backgroundColor: '#153243',
                color: 'white',
                borderRadius: '50%',
                width: '78px',
                height: '78px',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                border: 'none',
                cursor: 'pointer'
              }}
              onClick={handleSubmit}
            >
              <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="white">
                <path d="M8 5v14l11-7z" />
              </svg>
            </button>
          </div>
        </div>

        {/* Sidebar */}
        <div style={{
          width: '30%',
          display: 'flex',
          flexDirection: 'column',
          gap: '10px'
        }}>

          {/* Graph Representation */}
          <div style={{
            flex: '1',
            display: 'flex',
            flexDirection: 'column',
            borderRadius: '25px',
            border: '3px solid #153243',
            overflow: 'hidden',
            boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
          }}>
            <div style={{
              backgroundColor: '#153243',
              color: 'white',
              textAlign: 'center',
              padding: '10px 0px 0px 10px'
            }}>
              <div style={{ fontFamily: 'heyyy', fontSize: '24px', fontWeight: 'bold' }}>Graph Representation</div>
            </div>
            <div style={{
              flexGrow: 1,
              backgroundColor: '#EEF0EB',
              padding: '15px'
            }}>
              {timeComplexity ? (
<div style={{  width: '100%', height: '100%' }}>
  <ResponsiveContainer width="100%" height="100%">
    <LineChart data={data}>
      <CartesianGrid stroke="#ccc" />
      <XAxis />
      <YAxis />
      <Tooltip />
      {[...complexities].reverse().map((comp) => (
        <Line
          key={comp}
          type="monotone"
          dataKey={comp}
          stroke={colors[comp]}
          strokeWidth={timeComplexity === comp ? 3 : 1.5}
          strokeDasharray={timeComplexity === comp ? '' : '4 4'}
          dot={false}
        />
      ))}
    </LineChart>
  </ResponsiveContainer>
</div>


              ) : (
                <p style={{ textAlign: 'center', color: '#666' }}>
                  Time complexity graph will appear here.
                </p>
              )}
            </div>
          </div>

          {/* Time Complexity Value */}
          <div style={{
            flex: '1',
            display: 'flex',
            flexDirection: 'column',
            borderRadius: '25px',
            border: '3px solid #153243',
            overflow: 'hidden',
            boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
          }}>
            <div style={{
              backgroundColor: '#153243',
              color: 'white',
              textAlign: 'center',
              padding: '10px'
            }}>
              <div style={{ fontFamily: 'heyyy', fontSize: '24px', fontWeight: 'bold' }}>Time Complexity</div>
            </div>
            <div style={{
              flexGrow: 1,
              backgroundColor: '#EEF0EB',
              padding: '15px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center'
            }}>
              <p style={{ fontSize: '18px' }}>Time Complexity: {timeComplexity}</p>
            </div>
          </div>

        </div>
      </div>
    </div>
  );
}

export default SecondPage;
