import { useState } from 'react';
import { Link } from 'react-router-dom';

function SecondPage() {
  const [language, setLanguage] = useState('');
  const languages = ['c', 'c++', 'python', 'java'];

  return (
    <div className="min-h-screen bg-[#f4f9f8] p-6 flex flex-col items-center font-mono">

      <div className="w-full max-w-6xl mb-4 text-left">
        <Link to="/">
          <button className="text-[#0f2f39] hover:text-[#123849] font-semibold underline">
            ← Back to Home
          </button>
        </Link>
      </div>

      <div className="w-full max-w-6xl flex flex-1 gap-6">
        
        <div className="flex-[2] bg-white rounded-xl shadow-lg p-4 flex flex-col">
          <h2 className="text-xl font-bold text-[#0f2f39] mb-2">CODE</h2>

          <div className="flex space-x-4 mb-4">
            {languages.map((lang) => (
              <button
                key={lang}
                onClick={() => setLanguage(lang)}
                className={`px-4 py-1 rounded-t-md border-b-2 transition ${
                  language === lang
                    ? 'border-[#0f2f39] text-[#0f2f39] font-semibold'
                    : 'border-transparent text-gray-500 hover:text-[#0f2f39]'
                }`}
              >
                {lang.toUpperCase()}
              </button>
            ))}
          </div>

          <div className="bg-[#f5f5f5] rounded-lg flex-1 overflow-auto px-4 py-2">
            <div className="h-full flex justify-center items-center">
              <pre className="font-mono whitespace-pre-wrap">
                <code className="text-center text-sm block w-full">
                {`Write your code here`}
                </code>
              </pre>
            </div>
          </div>
        </div>

        <div className="flex-1 flex flex-col gap-6">
          
          <div className="bg-white rounded-xl shadow-lg p-4 flex-1 border border-gray-200">
            <h2 className="text-lg font-semibold mb-4 border-b pb-2 border-gray-300">Graph Representation</h2>
            <div className="w-full h-60 flex justify-center items-center">
              <img
                src="https://upload.wikimedia.org/wikipedia/commons/thumb/7/7e/Comparison_computational_complexity.svg/640px-Comparison_computational_complexity.svg.png"
                alt="Graph"
                className="max-h-full max-w-full object-contain"
              />
            </div>
          </div>


          <div className="bg-white rounded-xl shadow-lg p-4 flex-1 border border-gray-200">
            <h2 className="text-lg font-semibold mb-4 border-b pb-2 border-gray-300">Time Complexity</h2>
            <p className="text-gray-700 text-base text-center">
              The time complexity is: <strong className="text-[#0f2f39]">log(n)</strong>
            </p>
          </div>
        </div>
      </div>
    </div>
  );
}

export default SecondPage;
