/*
    src/window.cpp -- Top-level window widget

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/slidecanvas.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/serializer/core.h>

NAMESPACE_BEGIN(nanogui)

SlideCanvas::SlideCanvas(Widget *parent)
    : Widget(parent), mCanvasSize(16,9), mCanvasPos(0,0), windowRatio(16.0/9.0) { }

Vector2i SlideCanvas::preferredSize(NVGcontext *ctx) const {
    Vector2i result = Widget::preferredSize(ctx);

    result = Vector2i(16,9);

    return result;
}

void SlideCanvas::performLayout(NVGcontext *ctx) {
	Widget::performLayout(ctx);
}

void SlideCanvas::draw(NVGcontext *ctx) {
    int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;
    int hh = mTheme->mWindowHeaderHeight;

    /* Draw window */
    nvgSave(ctx);

    //Outer widget rectangle
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
    nvgFillColor(ctx, mMouseFocus ? mTheme->mWindowFillFocused
                                  : mTheme->mWindowFillUnfocused);
    nvgFill(ctx);

    //TODO: Don't recalc this every draw cycle, flag size changes
    if(mSize.y() != 0 && mSize.x() != 0){
        windowRatio = 16.0/9.0;
		float widgetRatio = ((float)(mSize.x()-20)) / (mSize.y()-20);

		Vector2i inset;
		inset.x() = windowRatio > widgetRatio ? mSize.x() - 20 : (mSize.y() - 20)*windowRatio;
		inset.y() = windowRatio > widgetRatio ? (mSize.x() - 20)/windowRatio : mSize.y() - 20;

		nvgBeginPath(ctx);
		nvgRect(ctx,
				(windowRatio > widgetRatio ? 10 : (mSize.x() - inset.x()) / 2)+mPos.x(),
						(windowRatio > widgetRatio ? (mSize.y() - inset.y()) / 2 : 10)+mPos.y(),
				inset.x(), inset.y());
		NVGcolor col = NVGcolor({0.0,0.0,0.0,1.0});
		nvgFillColor(ctx, col);
		nvgFill(ctx);
    }


    nvgRestore(ctx);

    Widget::draw(ctx);
}

void SlideCanvas::dispose() {

}

void SlideCanvas::center() {
}

bool SlideCanvas::mouseDragEvent(const Vector2i &p, const Vector2i &rel,
                            int button, int /* modifiers */) {

	/*
    if (mResizeH && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0) {
		return true;
	}*/

    return false;
}

bool SlideCanvas::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){

	return Widget::mouseMotionEvent(p, rel, button, modifiers);
}

bool SlideCanvas::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    return false;
}

bool SlideCanvas::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    Widget::scrollEvent(p, rel);
    return true;
}

void SlideCanvas::refreshRelativePlacement() {
    /* Overridden in \ref Popup */
}

void SlideCanvas::save(Serializer &s) const {
    Widget::save(s);
}

bool SlideCanvas::load(Serializer &s) {
    if (!Widget::load(s)) return false;
//    if (!s.get("title", mTitle)) return false;
//    if (!s.get("modal", mModal)) return false;

    return true;
}

NAMESPACE_END(nanogui)
