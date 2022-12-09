// Adds compile-time JS functions to augment the LamKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(LamKit){
  LamKit._extraInitializations = LamKit._extraInitializations || [];
  LamKit._extraInitializations.push(function() {
    const nullptr = 0;
    LamKit.Malloc = function(typedArray, len) {
      var byteLen = len * typedArray.BYTES_PER_ELEMENT;
      var ptr = LamKit._malloc(byteLen);
      return {
        '_ck': true,
        'length': len,
        'byteOffset': ptr,
        typedArray: null,
        'subarray': function(start, end) {
          var sa = this['toTypedArray']().subarray(start, end);
          sa['_ck'] = true;
          return sa;
        },
        'toTypedArray': function() {
          // Check if the previously allocated array is still usable.
          // If it's falsy, then we haven't created an array yet.
          // If it's empty, then WASM resized memory and emptied the array.
          if (this.typedArray && this.typedArray.length) {
            return this.typedArray;
          }
          this.typedArray = new typedArray(LamKit.HEAPU8.buffer, ptr, len);
          // add a marker that this was allocated in C++ land
          this.typedArray['_ck'] = true;
          return this.typedArray;
        },
      };
    };

    LamKit.Free = function(mallocObj) {
      LamKit._free(mallocObj['byteOffset']);
      mallocObj['byteOffset'] = nullptr;
      // Set these to null to make sure the TypedArrays can be garbage collected.
      mallocObj['toTypedArray'] = null;
      mallocObj.typedArray = null;
    };

    function wasMalloced(obj) {
      return obj && obj['_ck'];
    }

    function copy1dArray(arr, dest, ptr) {
      if (!arr || !arr.length) {
        return nullptr;
      }
      // This was created with LamKit.Malloc, so it's already been copied.
      if (wasMalloced(arr)) {
        return arr.byteOffset;
      }
      var bytesPerElement = LamKit[dest].BYTES_PER_ELEMENT;
      if (!ptr) {
        ptr = LamKit._malloc(arr.length * bytesPerElement);
      }
      // In c++ terms, the WASM heap is a uint8_t*, a long buffer/array of single
      // byte elements. When we run _malloc, we always get an offset/pointer into
      // that block of memory.
      // LamKit exposes some different views to make it easier to work with
      // different types. HEAPF32 for example, exposes it as a float*
      // However, to make the ptr line up, we have to do some pointer arithmetic.
      // Concretely, we need to convert ptr to go from an index into a 1-byte-wide
      // buffer to an index into a 4-byte-wide buffer (in the case of HEAPF32)
      // and thus we divide ptr by 4.
      // It is important to make sure we are grabbing the freshest view of the
      // memory possible because if we call _malloc and the heap needs to grow,
      // the TypedArrayView will no longer be valid.
      LamKit[dest].set(arr, ptr / bytesPerElement);
      return ptr;
    }

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

    // convenient wrapper
    LamKit.MakeResourceProvider = function(assets) {
      const assetNamePtrs = [];
      const assetDataPtrs = [];
      const assetSizes    = [];

      const assetKeys = Object.keys(assets || {});
      for (let i = 0; i < assetKeys.length; i++) {
        const key = assetKeys[i];
        const buffer = assets[key];
        const data = new Uint8Array(buffer);

        const iptr = LamKit._malloc(data.byteLength);
        LamKit.HEAPU8.set(data, iptr);
        assetDataPtrs.push(iptr);
        assetSizes.push(data.byteLength);

        // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
        // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
        // Add 1 for null terminator
        const strLen = lengthBytesUTF8(key) + 1;
        const strPtr = LamKit._malloc(strLen);

        stringToUTF8(key, strPtr, strLen);
        assetNamePtrs.push(strPtr);
      }

      // Not entirely sure if it matters, but the uintptr_t are 32 bits
      // we want to copy our array of uintptr_t into the right size memory.
      var namesPtr      = copy1dArray(assetNamePtrs, "HEAPU32");
      var assetsPtr     = copy1dArray(assetDataPtrs, "HEAPU32");
      var assetSizesPtr = copy1dArray(assetSizes,    "HEAPU32");

      const resourceProvider = LamKit._MakeResourceProvider(assetKeys.length, namesPtr, assetsPtr, assetSizesPtr);

      LamKit._free(namesPtr);
      LamKit._free(assetsPtr);
      LamKit._free(assetSizesPtr);

      return resourceProvider;
    }
  });
}(Module)); // When this file is loaded in, the high level object is "Module";
