import type { Lab2QRCodeModule } from '@/types/wasm';

let wasmModule: Lab2QRCodeModule | null = null;
let loadingPromise: Promise<Lab2QRCodeModule> | null = null;

async function waitForWasmScript(maxWaitMs = 5000): Promise<void> {
  const startTime = Date.now();
  while (!(window as any).createLab2QRCodeModule) {
    if (Date.now() - startTime > maxWaitMs) {
      throw new Error('Timeout waiting for lab2qrcode.js to load');
    }
    await new Promise(resolve => setTimeout(resolve, 100));
  }
}

export async function loadWasmModule(): Promise<Lab2QRCodeModule> {
  if (wasmModule) {
    return wasmModule;
  }

  if (loadingPromise) {
    return loadingPromise;
  }

  loadingPromise = (async () => {
    try {
      await waitForWasmScript();
      
      const createModule = (window as any).createLab2QRCodeModule;
      
      if (!createModule) {
        throw new Error('WASM module not loaded. Make sure lab2qrcode.js is included.');
      }

      wasmModule = await createModule();
      console.log('Lab2QRCode WASM module loaded successfully');
      if (!wasmModule) {
        throw new Error('WASM module initialization returned null');
      }
      return wasmModule;
    } catch (error) {
      loadingPromise = null;
      throw new Error(`Failed to load WASM module: ${error}`);
    }
  })();

  return loadingPromise;
}

export function getWasmModule(): Lab2QRCodeModule {
  if (!wasmModule) {
    throw new Error('WASM module not loaded yet. Call loadWasmModule() first.');
  }
  return wasmModule;
}
