let Module = null;
let initialized = false;

self.onmessage = async function (e) {
  const { type, expr } = e.data;

  if (type === 'init') {
    if (initialized) return;
    initialized = true;
    try {
      importScripts('/wasm/qseries.js');
      Module = await createQSeries({
        locateFile: (path) => '/wasm/' + path,
      });
      const banner = Module.get_banner();
      self.postMessage({ type: 'ready', banner });
    } catch (err) {
      self.postMessage({ type: 'error', error: 'Failed to load qseries engine: ' + (err.message || String(err)) });
    }
    return;
  }

  if (type === 'evaluate') {
    if (!Module) {
      self.postMessage({ type: 'error', error: 'Engine not initialized' });
      return;
    }
    try {
      const result = Module.evaluate(expr);
      self.postMessage({ type: 'result', result });
    } catch (err) {
      self.postMessage({ type: 'error', error: err.message || String(err) });
    }
  }
};
