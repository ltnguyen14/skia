// Adds compile-time JS functions to augment the LamKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(LamKit){
    LamKit._extraInitializations = LamKit._extraInitializations || [];
    LamKit._extraInitializations.push(function() {
      LamKit.MakeCanvasSurface = function(element, colorspace) {
        // webgl context
        const ctx = GL.createContext(element, {
          'alpha': 1,
          'depth': 1,
          'stencil': 8,
          'antialias': 0,
          'premultipliedAlpha': 1,
          'preserveDrawingBuffer': 0,
          'preferLowPowerToHighPerformance': 0,
          'failIfMajorPerformanceCaveat': 0,
          'enableExtensionsByDefault': 1,
          'explicitSwapControl': 0,
          'renderViaOffscreenBackBuffer': 0,
        });
        if (!ctx) {
          return 0;
        }

        GL.makeContextCurrent(ctx);
        GL.currentContext.GLctx.getExtension('WEBGL_debug_renderer_info');

        // gr context
        const grCtx = this._MakeGrContext();
        grCtx._context = ctx;
        GL.currentContext.grDirectContext = grCtx;

        // create surface
        const surface = this._MakeOnScreenGLSurface(grCtx, element.width, element.height, LamKit.ColorSpace.SRGB);
        surface._context = ctx;

        return surface;
      }
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
