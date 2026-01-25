import { useEffect, useState } from 'react';
import { QrCode, Camera } from 'lucide-react';
import { loadWasmModule } from './utils/wasmLoader';
import { CameraScanner } from './components/CameraScanner';
import { BarcodeGenerator } from './components/BarcodeGenerator';
import { ThemeToggle } from './components/ThemeToggle';

function App() {
  const [wasmLoaded, setWasmLoaded] = useState(false);
  const [loadError, setLoadError] = useState<string>('');
  const [activeTab, setActiveTab] = useState<'generate' | 'scan'>('generate');

  useEffect(() => {
    loadWasmModule()
      .then(() => {
        setWasmLoaded(true);
        console.log('WASM module loaded');
      })
      .catch((err) => {
        setLoadError(err.message);
        console.error('Failed to load WASM:', err);
      });
  }, []);

  if (loadError) {
    return <div className="error">Failed to load WASM module: {loadError}</div>;
  }

  if (!wasmLoaded) {
    return <div className="loading">Loading Lab2QRCode WASM module...</div>;
  }

  return (
    <div className="app">
       <header>
         <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', gap: '20px' }}>
           <div style={{ textAlign: 'center', flex: 1 }}>
             <h1>Lab2QRCode Web</h1>
             <p>Barcode Generator & Scanner (WASM-powered)</p>
           </div>
           <ThemeToggle />
         </div>
       </header>

       <nav>
         <button
           className={activeTab === 'generate' ? 'active' : ''}
           onClick={() => setActiveTab('generate')}
           style={{ display: 'flex', alignItems: 'center', gap: '8px' }}
         >
           <QrCode size={18} />
           <span>Generate</span>
         </button>
         <button
           className={activeTab === 'scan' ? 'active' : ''}
           onClick={() => setActiveTab('scan')}
           style={{ display: 'flex', alignItems: 'center', gap: '8px' }}
         >
           <Camera size={18} />
           <span>Scan</span>
         </button>
       </nav>

      <main>
        {activeTab === 'generate' ? <BarcodeGenerator /> : <CameraScanner />}
      </main>
    </div>
  );
}

export default App;
