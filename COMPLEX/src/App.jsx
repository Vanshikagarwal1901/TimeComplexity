import { BrowserRouter, Routes, Route } from 'react-router-dom';
import First from '../pages/first';
import SecondPage from '../pages/second';

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<First />} />
        <Route path="/analyzer" element={<SecondPage />} />
      </Routes>
    </BrowserRouter>
  );
}

export default App;
