/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window.h"

#include "SkSurface.h"
#include "SkCanvas.h"
#include "WindowContext.h"

namespace sk_app {

Window::Window() {}

void Window::detach() {
    delete fWindowContext;
    fWindowContext = nullptr;
}

void Window::onBackendCreated() {
    for (int i = 0; i < fLayers.count(); ++i) {
        fLayers[i]->onBackendCreated();
    }
}

bool Window::onChar(SkUnichar c, uint32_t modifiers) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->onChar(c, modifiers)) {
            return true;
        }
    }
    return false;
}

bool Window::onKey(Key key, InputState state, uint32_t modifiers) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->onKey(key, state, modifiers)) {
            return true;
        }
    }
    return false;
}

bool Window::onMouse(int x, int y, InputState state, uint32_t modifiers) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->onMouse(x, y, state, modifiers)) {
            return true;
        }
    }
    return false;
}

bool Window::onMouseWheel(float delta, uint32_t modifiers) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->onMouseWheel(delta, modifiers)) {
            return true;
        }
    }
    return false;
}

bool Window::onTouch(intptr_t owner, InputState state, float x, float y) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->onTouch(owner, state, x, y)) {
            return true;
        }
    }
    return false;
}

void Window::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    for (int i = 0; i < fLayers.count(); ++i) {
        fLayers[i]->onUIStateChanged(stateName, stateValue);
    }
}

void Window::onPaint() {
    if (!fWindowContext) {
        return;
    }
    markInvalProcessed();
    sk_sp<SkSurface> backbuffer = fWindowContext->getBackbufferSurface();
    if (backbuffer) {
        // draw into the canvas of this surface
        SkCanvas* canvas = backbuffer->getCanvas();

        for (int i = 0; i < fLayers.count(); ++i) {
            fLayers[i]->onPaint(canvas);
        }

        canvas->flush();

        fWindowContext->swapBuffers();
    } else {
        printf("no backbuffer!?\n");
        // try recreating testcontext
    }
}

void Window::onResize(int w, int h) {
    if (!fWindowContext) {
        return;
    }
    fWindowContext->resize(w, h);
}

int Window::width() {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->width();
}

int Window::height() {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->height();
}

void Window::setRequestedDisplayParams(const DisplayParams& params, bool /* allowReattach */) {
    fRequestedDisplayParams = params;
    if (fWindowContext) {
        fWindowContext->setDisplayParams(fRequestedDisplayParams);
    }
}

int Window::sampleCount() const {
    if (!fWindowContext) {
        return -1;
    }
    return fWindowContext->sampleCount();
}

int Window::stencilBits() const {
    if (!fWindowContext) {
        return -1;
    }
    return fWindowContext->stencilBits();
}

const GrContext* Window::getGrContext() const {
    if (!fWindowContext) {
        return nullptr;
    }
    return fWindowContext->getGrContext();
}

void Window::inval() {
    if (!fWindowContext) {
        return;
    }
    if (!fIsContentInvalidated) {
        fIsContentInvalidated = true;
        onInval();
    }
}

void Window::markInvalProcessed() {
    fIsContentInvalidated = false;
}

}   // namespace sk_app
