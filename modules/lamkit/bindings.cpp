#include "include/core/SkColorSpace.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <iostream>

#ifdef CK_ENABLE_WEBGL
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrCaps.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines_impl.h"

#include <webgl/webgl1.h>
#endif // CK_ENABLE_WEBGL

using namespace emscripten;

struct ColorSettings {
  ColorSettings(sk_sp<SkColorSpace> colorSpace) {
    if (colorSpace == nullptr || colorSpace->isSRGB()) {
      colorType = kRGBA_8888_SkColorType;
      pixFormat = GR_GL_RGBA8;
    } else {
      colorType = kRGBA_F16_SkColorType;
      pixFormat = GR_GL_RGBA16F;
    }
  }
  SkColorType colorType;
  GrGLenum pixFormat;
};

sk_sp<GrDirectContext> MakeGrContext() {
  // We assume that any calls we make to GL for the remainder of this function will go to the
  // desired WebGL Context.
  // setup interface.
  auto interface = GrGLMakeNativeInterface();
  // setup context
  return GrDirectContext::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height,
    sk_sp<SkColorSpace> colorSpace) {
  // WebGL should already be clearing the color and stencil buffers, but do it again here to
  // ensure Skia receives them in the expected state.
  emscripten_glBindFramebuffer(GL_FRAMEBUFFER, 0);
  emscripten_glClearColor(0, 0, 0, 0);
  emscripten_glClearStencil(0);
  emscripten_glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  dContext->resetContext(kRenderTarget_GrGLBackendState | kMisc_GrGLBackendState);

  // The on-screen canvas is FBO 0. Wrap it in a Skia render target so Skia can render to it.
  GrGLFramebufferInfo info;
  info.fFBOID = 0;

  GrGLint sampleCnt;
  emscripten_glGetIntegerv(GL_SAMPLES, &sampleCnt);

  GrGLint stencil;
  emscripten_glGetIntegerv(GL_STENCIL_BITS, &stencil);

  if (!colorSpace) {
    colorSpace = SkColorSpace::MakeSRGB();
  }

  const auto colorSettings = ColorSettings(colorSpace);
  info.fFormat = colorSettings.pixFormat;
  GrBackendRenderTarget target(width, height, sampleCnt, stencil, info);
  sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(dContext.get(), target,
        kBottomLeft_GrSurfaceOrigin, colorSettings.colorType, colorSpace, nullptr));

  if(!surface) {
    std::cout << "surface not created" << std::endl;
  }
  return surface;
}

EMSCRIPTEN_BINDINGS(Skottie) {
  class_<GrDirectContext>("GrDirectContext")
    .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");

  class_<SkSurface>("Surface")
    .smart_ptr<sk_sp<SkSurface>>("sk_sp<Surface>");

  class_<SkColorSpace>("ColorSpace")
    .smart_ptr<sk_sp<SkColorSpace>>("sk_sp<ColorSpace>")
    .class_function("_MakeSRGB", &SkColorSpace::MakeSRGB);

  function("_MakeGrContext", &MakeGrContext);
  function("_MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
}
