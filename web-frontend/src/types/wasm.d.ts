export interface BarcodeResult {
  success: boolean;
  content: string;
  format: string;
  error: string;
}

export type BarcodeFormat =
  | 'QRCode'
  | 'MicroQRCode'
  | 'RMQRCode'
  | 'Aztec'
  | 'DataMatrix'
  | 'PDF417'
  | 'MaxiCode'
  | 'Code128'
  | 'Code39'
  | 'Code93'
  | 'Codabar'
  | 'EAN8'
  | 'EAN13'
  | 'UPCA'
  | 'UPCE'
  | 'ITF'
  | 'DataBar'
  | 'DataBarExpanded'
  | 'DataBarLimited'
  | 'DXFilmEdge';

export interface Lab2QRCodeModule {
  base64Encode(data: string): string;
  base64Decode(encoded: string): string;
  
  generateBarcode(
    content: string,
    format: string,
    width?: number,
    height?: number,
    useBase64?: boolean
  ): string;
  
  decodeBarcode(
    imageData: ImageData | Uint8Array,
    tryAllFormats?: boolean,
    useBase64?: boolean
  ): BarcodeResult;
  
  decodeBarcodes(
    imageData: ImageData | Uint8Array,
    tryAllFormats?: boolean,
    useBase64?: boolean
  ): BarcodeResult[];
}

declare global {
  function createLab2QRCodeModule(): Promise<Lab2QRCodeModule>;
}

export {};
