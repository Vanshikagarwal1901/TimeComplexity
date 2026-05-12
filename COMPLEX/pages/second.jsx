import React, { useMemo, useState } from 'react';
import axios from 'axios';

const sampleCode = `using System;

public class Demo
{
    public static void Main()
    {
        int n = 100;

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                Console.WriteLine(i + j);
            }
        }

        while (n > 1)
        {
            n /= 2;
        }
    }
}`;

const complexityScale = ['O(1)', 'O(log n)', 'O(n)', 'O(n log n)', 'O(n^2)', 'O(n^3)', 'higher'];

function ComplexityGraph({ nodes }) {
  const graphNodes = nodes?.length ? nodes : [{ Label: 'result', Complexity: 'O(1)', Level: 0, Depth: 0 }];
  const maxLevel = 6;

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column', gap: '10px' }}>
      <div style={{ flex: 1, display: 'flex', alignItems: 'stretch', gap: '10px', minHeight: 0 }}>
        <div style={{
          display: 'flex',
          flexDirection: 'column-reverse',
          justifyContent: 'space-between',
          color: '#153243',
          fontSize: '11px',
          width: '64px',
          textAlign: 'right',
          paddingBottom: '20px'
        }}>
          {complexityScale.map(label => <span key={label}>{label}</span>)}
        </div>
        <div style={{
          flex: 1,
          display: 'flex',
          alignItems: 'flex-end',
          gap: '12px',
          borderLeft: '2px solid #153243',
          borderBottom: '2px solid #153243',
          padding: '10px 10px 20px',
          minWidth: 0
        }}>
          {graphNodes.map((node, index) => {
            const level = Math.max(0, Math.min(maxLevel, node.Level ?? 0));
            const height = `${Math.max(10, ((level + 1) / (maxLevel + 1)) * 100)}%`;
            return (
              <div key={`${node.Label}-${index}`} style={{
                flex: '1 1 0',
                height: '100%',
                display: 'flex',
                flexDirection: 'column',
                justifyContent: 'flex-end',
                alignItems: 'center',
                minWidth: '38px'
              }}>
                <div title={`${node.Label}: ${node.Complexity}`} style={{
                  width: '100%',
                  maxWidth: '48px',
                  height,
                  background: '#2c4453',
                  borderRadius: '6px 6px 0 0',
                  boxShadow: 'inset 0 1px 0 rgba(255,255,255,0.25)'
                }} />
                <span style={{
                  marginTop: '6px',
                  color: '#153243',
                  fontSize: '11px',
                  maxWidth: '64px',
                  overflow: 'hidden',
                  textOverflow: 'ellipsis',
                  whiteSpace: 'nowrap'
                }}>
                  {node.Label}
                </span>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}

function SecondPage() {
  const [code, setCode] = useState(sampleCode);

  const languages = ['C#'];
  const [selectedLanguage, setSelectedLanguage] = useState('C#');
  const [timeComplexity, setTimeComplexity] = useState(null);
  const [nodes, setNodes] = useState([]);
  const [explanation, setExplanation] = useState([]);
  const [error, setError] = useState(null);
  const [isAnalyzing, setIsAnalyzing] = useState(false);

  const resultText = useMemo(() => {
    if (isAnalyzing) return 'Analyzing...';
    if (error) return error;
    return timeComplexity ?? 'Run the analyzer';
  }, [error, isAnalyzing, timeComplexity]);

  const handleSubmit = () => {
    setIsAnalyzing(true);
    setError(null);
    axios.post('http://127.0.0.1:8000/analyzer/analyze_code/', {
      code: code,
      language: selectedLanguage
    }, {
      headers: { 'Content-Type': 'application/json' }
    })
    .then(response => {
      setTimeComplexity(response.data.time_complexity);
      setNodes(response.data.nodes ?? []);
      setExplanation(response.data.explanation ?? []);
    })
    .catch(error => {
      setTimeComplexity(null);
      setNodes([]);
      setExplanation([]);
      setError(error.response?.data?.error ?? 'Could not reach the analyzer');
    })
    .finally(() => setIsAnalyzing(false));
  };

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
        height: 'calc(100%)', 
        width: '100%'
      }}>

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

          <div style={{
            backgroundColor: '#153243',
            color: 'white',
            textAlign: 'left',
            padding: '10px 0px 0px 10px'
          }}>
            <div style={{ fontFamily:'heyyy',fontSize: '24px', fontWeight: 'bold' }}>CODE</div>
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
        






        <div style={{
          width: '30%',
          display: 'flex',
          flexDirection: 'column',
          gap: '10px'
        }}>
         
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
              padding: '15px',
              minHeight: 0
            }}>
              <ComplexityGraph nodes={nodes} />
            </div>
          </div>
          


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
              flexDirection: 'column',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '12px',
              color: '#153243'
            }}>
              <p style={{
                fontSize: error ? '16px' : '34px',
                fontWeight: 'bold',
                margin: 0,
                color: error ? '#9b1c31' : '#153243'
              }}>
                {resultText}
              </p>
              <div style={{
                width: '100%',
                maxHeight: '110px',
                overflow: 'auto',
                textAlign: 'left',
                fontSize: '12px',
                lineHeight: 1.45
              }}>
                {explanation.slice(0, 4).map((line, index) => (
                  <p key={index} style={{ margin: '0 0 6px' }}>{line}</p>
                ))}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default SecondPage;
