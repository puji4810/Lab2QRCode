import { useState } from 'react';
import { Download } from 'lucide-react';
import { getWasmModule } from '@/utils/wasmLoader';
import type { BarcodeFormat } from '@/types/wasm';

const BARCODE_FORMATS: BarcodeFormat[] = [
  'QRCode', 'Aztec', 'DataMatrix', 'PDF417',
  'Code128', 'Code39', 'Code93', 'Codabar',
  'EAN8', 'EAN13', 'UPCA', 'UPCE', 'ITF'
];

const READ_ONLY_FORMATS: BarcodeFormat[] = [
  'MicroQRCode', 'RMQRCode', 'MaxiCode',
  'DataBar', 'DataBarExpanded', 'DataBarLimited', 'DXFilmEdge'
];

export function BarcodeGenerator() {
  const [content, setContent] = useState('');
  const [format, setFormat] = useState<BarcodeFormat>('QRCode');
  const [useBase64, setUseBase64] = useState(true);
  const [imageData, setImageData] = useState<string>('');
  const [error, setError] = useState<string>('');

  const handleGenerate = async () => {
    if (!content) {
      setError('Please enter content');
      return;
    }

    if (READ_ONLY_FORMATS.includes(format)) {
      setError(`${format} is not supported for generation. This format can only be decoded (read-only).`);
      return;
    }

    try {
      setError('');
      const wasm = getWasmModule();
      const svgString = wasm.generateBarcode(content, format, 400, 400, useBase64);
      const base64Svg = btoa(unescape(encodeURIComponent(svgString)));
      setImageData(`data:image/svg+xml;base64,${base64Svg}`);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Generation failed');
    }
  };

   const dataURItoBlob = (dataURI: string): Blob => {
     const [header, base64Data] = dataURI.split(',');
     const mimeType = header.split(':')[1].split(';')[0];
     const byteCharacters = atob(base64Data);
     const byteNumbers = new Array(byteCharacters.length);
     for (let i = 0; i < byteCharacters.length; i++) {
       byteNumbers[i] = byteCharacters.charCodeAt(i);
     }
     const byteArray = new Uint8Array(byteNumbers);
     return new Blob([byteArray], { type: mimeType });
   };

   const handleDownload = async () => {
     if (!imageData) return;

     const filename = `barcode-${format}-${Date.now()}.svg`;
     const isMobile = /iPhone|iPad|iPod|Android/i.test(navigator.userAgent);

     // Try Web Share API on mobile
     if (isMobile && navigator.share && navigator.canShare) {
       try {
         const blob = dataURItoBlob(imageData);
         const file = new File([blob], filename, { type: blob.type });
         if (navigator.canShare({ files: [file] })) {
           await navigator.share({ files: [file], title: 'Barcode' });
           return;
         }
       } catch (error: any) {
         if (error.name === 'AbortError') return; // User cancelled
         console.error('Share failed:', error);
         // Fall through to Blob URL fallback
       }
     }

     // Blob URL fallback (desktop or share failed)
     const blob = dataURItoBlob(imageData);
     const blobUrl = URL.createObjectURL(blob);
     const link = document.createElement('a');
     link.href = blobUrl;
     link.download = filename;
     document.body.appendChild(link);
     link.click();
     document.body.removeChild(link);
     setTimeout(() => URL.revokeObjectURL(blobUrl), 100);
   };

  return (
    <div className="barcode-generator">
      <div className="form">
        <textarea
          value={content}
          onChange={(e) => setContent(e.target.value)}
          placeholder="Enter content to encode"
          rows={4}
        />

        <select value={format} onChange={(e) => setFormat(e.target.value as BarcodeFormat)}>
          {BARCODE_FORMATS.map((fmt) => (
            <option key={fmt} value={fmt}>{fmt}</option>
          ))}
        </select>

        <div style={{ fontSize: '0.85em', color: '#666', marginTop: '-8px' }}>
          Note: MicroQR, rMQR, MaxiCode, DataBar variants, and DXFilmEdge are read-only formats (decoding only).
        </div>

        <label>
          <input
            type="checkbox"
            checked={useBase64}
            onChange={(e) => setUseBase64(e.target.checked)}
          />
          Use Base64 Encoding
        </label>

        <button onClick={handleGenerate}>Generate</button>
      </div>

      {error && <div className="error">{error}</div>}

       {imageData && (
         <div className="result">
           <img src={imageData} alt="Generated Barcode" />
           <button onClick={handleDownload} style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
             <Download size={18} />
             <span>Download</span>
           </button>
         </div>
       )}
    </div>
  );
}
