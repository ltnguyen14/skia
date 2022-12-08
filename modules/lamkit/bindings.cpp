#include "include/core/SkColorSpace.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <iostream>
#include <vector>

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

// skottie
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/utils/SkottieUtils.h"

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

class SkottieAssetProvider : public skottie::ResourceProvider {
 public:
  ~SkottieAssetProvider() override = default;
  using AssetVec = std::vector<std::pair<SkString, sk_sp<SkData>>>;

  static sk_sp<SkottieAssetProvider> Make(AssetVec assets) {
    return sk_sp<SkottieAssetProvider>(new SkottieAssetProvider(assets));
  }

  sk_sp<skottie::ImageAsset> loadImageAsset(const char[] /* path */,
                                            const char name[],
                                            const char[] /* id */) const override {
    if (auto data = this->findAsset(name)) {
      return skresources::MultiFrameImageAsset::Make(std::move(data));
    }

    return nullptr;
  }

 private:
  explicit SkottieAssetProvider(AssetVec assets)
      : fAssets(std::move(assets)) {}

  sk_sp<SkData> findAsset(const char name[]) const {
    for (const auto& asset : fAssets) {
      if (asset.first.equals(name)) {
        return asset.second;
      }
    }

    SkDebugf("Could not find %s\n", name);
    return nullptr;
  }

  const AssetVec fAssets;
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

void Paint(sk_sp<SkSurface> surface) {
  const SkScalar scale = 256.0f;
  const SkScalar R = 0.45f * scale;
  const SkScalar TAU = 6.2831853f;
  SkPath path;
  path.moveTo(R, 0.0f);
  for (int i = 1; i < 7; ++i) {
    SkScalar theta = 3 * i * TAU / 7;
    path.lineTo(R * cos(theta), R * sin(theta));
  }
  path.close();
  SkPaint p;
  p.setAntiAlias(true);
  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  canvas->translate(0.5f * scale, 0.5f * scale);
  canvas->drawPath(path, p);

  surface->flush();
}

void RenderAnimation(sk_sp<SkSurface> surface, sk_sp<skottie::Animation> animation, double frame) {
  animation->seekFrame(frame);
  animation->render(surface->getCanvas());
  surface->flush();
}

EMSCRIPTEN_BINDINGS(Skottie) {
  class_<skottie::Animation>("Animation")
      .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>");

  class_<GrDirectContext>("GrDirectContext")
      .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");

  class_<SkSurface>("Surface")
      .smart_ptr<sk_sp<SkSurface>>("sk_sp<Surface>");

  class_<SkColorSpace>("ColorSpace")
      .smart_ptr<sk_sp<SkColorSpace>>("sk_sp<ColorSpace>")
      .class_function("_MakeSRGB", &SkColorSpace::MakeSRGB);

  function("_MakeGrContext", &MakeGrContext);
  function("_MakeOnScreenGLSurface", &MakeOnScreenGLSurface);

  // public
  function("Paint", &Paint);
  function("RenderAnimation", &RenderAnimation);
  function("MakeAnimation", optional_override([](std::string json, sk_sp<skottie::ResourceProvider> rp)->sk_sp<skottie::Animation> {
    return skottie::Animation::Builder()
        .setResourceProvider(std::move(rp))
        .make(json.c_str(), json.size());
  }));

  function("MakeResourceProvider", optional_override([]()->sk_sp<skottie::ResourceProvider> {
    SkottieAssetProvider::AssetVec assets;

    for (size_t i = 0; i < assetCount; i++) {
      auto name  = SkString(assetNames[i]);
      auto bytes = SkData::MakeFromMalloc(assetDatas[i], assetSizes[i]);
      assets.push_back(std::make_pair(std::move(name), std::move(bytes)));
    }
    return SkottieAssetProvider::Make(std::move(assets));
  }));
}
