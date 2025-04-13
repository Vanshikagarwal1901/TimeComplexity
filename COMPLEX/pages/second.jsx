import React, { useState } from 'react';
import { Link } from 'react-router-dom';

function SecondPage() {
  const [code, setCode] = useState(`write your code here
}`);

  const languages = ['C', 'C++', 'Python', 'Java'];
  const [selectedLanguage, setSelectedLanguage] = useState('C');

  const handleSubmit = () => {
    console.log("Submitting code:", code);
    console.log("Selected language:", selectedLanguage);
  };

  return (
    <div style={{
      height: '100vh',
      width: '100vw',
      overflow: 'hidden',
      backgroundColor: '#F4F9E9',
      padding: '10px', // Adding main padding here
      boxSizing: 'border-box'
    }}>
      <div style={{
        display: 'flex',
        gap: '10px',
        height: 'calc(100%)', // Account for padding
        width: '100%'
      }}>
        {/* Left Panel */}
        <div style={{
          width: '70%',
          display: 'flex',
          flexDirection: 'column',
          borderRadius: '25px',
          border : '3px ',
          borderStyle : 'solid',
          borderColor :'#153243',
          overflow: 'hidden',
          boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
        }}>
          {/* CODE Header */}
          <div style={{
            backgroundColor: '#153243',
            color: 'white',
            textAlign: 'left',
            padding: '10px 0px 0px 10px'
          }}>
            <div style={{ fontFamily:'heyyy',fontSize: '24px', fontWeight: 'bold' }}>CODE</div>
          </div>
          
          {/* Language Tabs */}
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
          
          {/* Code Editor */}
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
            
            {/* Play Button */}
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
              <svg 
                xmlns="http://www.w3.org/2000/svg" 
                width="24" 
                height="24" 
                viewBox="0 0 24 24" 
                fill="white"
              >
                <path d="M8 5v14l11-7z" />
              </svg>
            </button>
          </div>
        </div>
        
        {/* Right Panel */}
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
            border:'3px',
            borderColor:'#153243',
            borderStyle:'solid',
            overflow: 'hidden',
            boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
          }}>
            <div style={{
              backgroundColor: '#153243',
              color: 'white',
              textAlign: 'center',
              padding: '10px 0px 0px 10px'
            }}>
              <div style={{ fontFamily:'heyyy',fontSize: '24px', fontWeight: 'bold' }}>Graph Representation</div>
            </div>
            <div style={{
              flexGrow: 1,
              backgroundColor: '#EEF0EB',
              padding: '15px'
            }}>
              {/* Content for graph would go here */}
            </div>
          </div>
          
          {/* Time Complexity */}
          <div style={{
            flex: '1',
            display: 'flex',
            flexDirection: 'column',
            borderRadius: '25px',
            border:'3px',
            borderColor:'#153243',
            borderStyle:'solid',
            overflow: 'hidden',
            boxShadow: '0 4px 6px rgba(0, 0, 0, 0.5)'
          }}>
            <div style={{
              backgroundColor: '#153243',
              color: 'white',
              textAlign: 'center',
              padding: '10px'
            }}>
              <div style={{fontFamily:'heyyy', fontSize: '24px', fontWeight: 'bold' }}>Time Complexity</div>
            </div>
            <div style={{
              flexGrow: 1,
              backgroundColor: '#EEF0EB',
              padding: '15px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center'
            }}>
              <p style={{ fontSize: '18px' }}></p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default SecondPage;