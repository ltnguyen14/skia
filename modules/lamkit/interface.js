// Adds compile-time JS functions to augment the CanvasKit interface.
  (function(CanvasKit) {
    // Adds JS functions to augment the CanvasKit interface.
      // For example, if there is a wrapper around the C++ call or logic to allow
    // chaining, it should go here.

      // CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
      // Anything that modifies an exposed class (e.g. Path) should be set
    // after onRuntimeInitialized, otherwise, it can happen outside of that scope.
      CanvasKit.onRuntimeInitialized = function() {
        // All calls to 'this' need to go in externs.js so closure doesn't minify them away.
          CanvasKit.ColorSpace.SRGB = CanvasKit.ColorSpace._MakeSRGB();

        // Run through the JS files that are added at compile time.
          if (CanvasKit._extraInitializations) {
            CanvasKit._extraInitializations.forEach(function(init) {
              init();
            });
          }
      }; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.
  }(Module)); // When this file is loaded in, the high level object is "Module";

