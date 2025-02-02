/*
  * This externs file prevents the Closure JS compiler from minifying away
  * names of objects created by Emscripten.
  * Basically, by defining empty objects and functions here, Closure will
  * know not to rename them.  This is needed because of our pre-js files,
  * that is, the JS we hand-write to bundle into the output. That JS will be
  * hit by the closure compiler and thus needs to know about what functions
  * have special names and should not be minified.
  *
  * Emscripten does not support automatically generating an externs file, so we
  * do it by hand. The general process is to write some JS code, and then put any
  * calls to CanvasKit or related things in here. Running ./compile.sh and then
  * looking at the minified results or running the Release trybot should
  * verify nothing was missed. Optionally, looking directly at the minified
  * pathkit.js can be useful when developing locally.
  *
  * Docs:
  *   https://github.com/cljsjs/packages/wiki/Creating-Externs
  *   https://github.com/google/closure-compiler/wiki/Types-in-the-Closure-Type-System
  *
  * Example externs:
  *   https://github.com/google/closure-compiler/tree/master/externs
  */

  var CanvasKit = {
    // public API (i.e. things we declare in the pre-js file or in the cpp bindings)
    MakeCanvasSurface: function() {},
    ColorSpace: {
      SRGB: {},
      // private API (from C++ bindings)
      _MakeSRGB: function() {},
    },

    // private
    _MakeGrContext: function() {},
    _MakeWebGLCanvasSurface: function() {},

    // Things Enscriptem adds for us

    /**
    * @type {Float32Array}
    */
    HEAPF32: {},
    /**
    * @type {Float64Array}
    */
    HEAPF64: {},
    /**
    * @type {Uint8Array}
    */
    HEAPU8: {},
    /**
    * @type {Uint16Array}
    */
    HEAPU16: {},
    /**
    * @type {Uint32Array}
    */
    HEAPU32: {},
    /**
    * @type {Int8Array}
    */
    HEAP8: {},
    /**
    * @type {Int16Array}
    */
    HEAP16: {},
    /**
    * @type {Int32Array}
    */
    HEAP32: {},

    _malloc: function() {},
    _free: function() {},
    onRuntimeInitialized: function() {},
  };
