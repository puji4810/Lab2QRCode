import { useState, useRef } from 'react';
import { Upload } from 'lucide-react';
import { useCamera } from '@/hooks/useCamera';
import { getWasmModule } from '@/utils/wasmLoader';
import type { BarcodeResult } from '@/types/wasm';

export function CameraScanner() {
  const { videoRef, isActive, error, devices, startCamera, stopCamera } = useCamera({
    facingMode: 'environment',
    width: 1280,
    height: 720,
  });

  const [, setScanning] = useState(false);
  const [lastResult, setLastResult] = useState<BarcodeResult | null>(null);
  const scanIntervalRef = useRef<number>();

  const handleStartScan = async () => {
    await startCamera();
    setScanning(true);
    
    scanIntervalRef.current = window.setInterval(async () => {
      if (!videoRef.current || !videoRef.current.videoWidth) return;

      const video = videoRef.current;
      const canvas = document.createElement('canvas');
      canvas.width = video.videoWidth;
      canvas.height = video.videoHeight;
      
      const ctx = canvas.getContext('2d');
      if (!ctx) return;

      ctx.drawImage(video, 0, 0);
      const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

      try {
        const wasm = getWasmModule();
        const result = wasm.decodeBarcode(imageData, true, true);
        
        if (result.success) {
          setLastResult(result);
          console.log('Barcode detected:', result);
        }
      } catch (err) {
        console.error('Decode error:', err);
      }
    }, 500);
  };

  const handleStopScan = () => {
    if (scanIntervalRef.current) {
      clearInterval(scanIntervalRef.current);
    }
    stopCamera();
    setScanning(false);
  };

  const handleFileUpload = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    try {
      const img = new Image();
      const reader = new FileReader();

      reader.onload = (e) => {
        img.onload = () => {
          const canvas = document.createElement('canvas');
          canvas.width = img.width;
          canvas.height = img.height;
          
          const ctx = canvas.getContext('2d');
          if (!ctx) return;

          ctx.drawImage(img, 0, 0);
          const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

          try {
            const wasm = getWasmModule();
            const result = wasm.decodeBarcode(imageData, true, true);
            
            if (result.success) {
              setLastResult(result);
              console.log('Barcode decoded from file:', result);
            } else {
              setLastResult({ success: false, content: '', format: '', error: 'No barcode detected in image' });
            }
          } catch (err) {
            console.error('Decode error:', err);
            setLastResult({ success: false, content: '', format: '', error: err instanceof Error ? err.message : 'Decode failed' });
          }
        };
        img.src = e.target?.result as string;
      };

      reader.readAsDataURL(file);
    } catch (err) {
      console.error('File upload error:', err);
    }
  };

  return (
    <div className="camera-scanner">
      <div className="video-container">
        <video ref={videoRef} autoPlay playsInline muted />
      </div>

      {error && <div className="error">{error}</div>}

      <div className="controls">
        {!isActive ? (
          <>
            <button onClick={handleStartScan}>Start Camera</button>
            <label htmlFor="file-upload" style={{ 
              display: 'inline-flex', 
              alignItems: 'center', 
              gap: '8px',
              padding: '10px 20px',
              backgroundColor: 'var(--primary-color, #007bff)',
              color: 'white',
              borderRadius: '4px',
              cursor: 'pointer',
              border: 'none',
              fontSize: '16px'
            }}>
              <Upload size={18} />
              <span>Upload Image</span>
            </label>
            <input 
              id="file-upload"
              type="file" 
              accept="image/*" 
              onChange={handleFileUpload}
              style={{ display: 'none' }}
            />
          </>
        ) : (
          <button onClick={handleStopScan}>Stop Camera</button>
        )}
      </div>

      {lastResult && (
        <div className={lastResult.success ? "result" : "error"}>
          {lastResult.success ? (
            <>
              <h3>Detected: {lastResult.format}</h3>
              <p>{lastResult.content}</p>
            </>
          ) : (
            <p>{lastResult.error || 'No barcode detected'}</p>
          )}
        </div>
      )}

      {devices.length > 0 && (
        <div className="devices">
          <select onChange={(e) => startCamera(e.target.value)}>
            {devices.map((device) => (
              <option key={device.deviceId} value={device.deviceId}>
                {device.label || `Camera ${device.deviceId.slice(0, 8)}`}
              </option>
            ))}
          </select>
        </div>
      )}
    </div>
  );
}
