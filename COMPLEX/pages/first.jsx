import '../src/App.css';

function First() {
  return (
    <div
      style={{
        height: '100vh',
        width: '100vw',
        overflow: 'hidden',
        backgroundColor: '#ffffff',
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        border: '5px solid black',
      }}
      className="w-full"
    >
      <div style={{ textAlign: 'center' }}>
        <h2
          style={{
            fontFamily: 'cursive',
            fontWeight: 'lighter',
            color: '#2c4453',
            marginBottom: '0',
          }}
        >
          time complexity
        </h2>
        <h1
          style={{
            fontSize: '64px',
            fontWeight: 'bold',
            color: '#2c4453',
            letterSpacing: '2px',
            marginTop: '0',
          }}
        >
          ANALYZER
        </h1>
      </div>
    </div>
  );
}

export default First;
