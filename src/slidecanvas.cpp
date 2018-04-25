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
#include <nanogui/window.h>
#include <nanogui/serializer/core.h>
#include <nanogui/mediaitembase.h>

NAMESPACE_BEGIN(nanogui)

SlideCanvas::SlideCanvas(Widget *parent)
    : Widget(parent), windowRatio(16.0/9.0),mSelectedImage(NULL){ }

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

		mCanvasSize.x() = windowRatio > widgetRatio ? mSize.x() - 20 : (mSize.y() - 20)*windowRatio;
		mCanvasSize.y() = windowRatio > widgetRatio ? (mSize.x() - 20)/windowRatio : mSize.y() - 20;
		mCanvasPos.x() = (windowRatio > widgetRatio ? 10 : (mSize.x() - mCanvasSize.x()) / 2);
		mCanvasPos.y() = (windowRatio > widgetRatio ? (mSize.y() - mCanvasSize.y()) / 2 : 10);

		nvgBeginPath(ctx);
		nvgRect(ctx, mCanvasPos.x()+mPos.x(), mCanvasPos.y()+mPos.y(), mCanvasSize.x(), mCanvasSize.y());
		NVGcolor col = NVGcolor({0.0,0.0,0.0,1.0});
		nvgFillColor(ctx, col);
		nvgFill(ctx);
    }


    nvgRestore(ctx);

    //Draw child widgets
    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    for (auto child : mChildren) {
        if (child->visible()) {
            nvgSave(ctx);
            nvgIntersectScissor(ctx, mCanvasPos.x(),mCanvasPos.y(), mCanvasSize.x(), mCanvasSize.y());
            nvgIntersectScissor(ctx, child->position().x(), child->position().y(),
            		child->size().x(), child->size().y());
            child->draw(ctx);
            nvgRestore(ctx);
        }
    }
    nvgRestore(ctx);
}

/*
void SlideCanvas::ImageItemUpdate(SlideImage *image)
{
	mSelectedImage = image;
}

void SlideCanvas::ImageLostFocus(SlideImage *image){
	if(mSelectedImage == image){
		mSelectedImage = NULL;
	}
}*/

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
	{
		for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
			Widget *child = *it;
			if (child->focused())
			{
				while (propertiesPanel->childCount() > 0)
					propertiesPanel->removeChild(0);

				//TODO: Find a better way than this cast
				((MediaItemBase *)child)->initPropertiesPanel(propertiesPanel);

				//TODO: Figure out how to just do layout on one widget
				screen()->performLayout();
			}
		}
		return true;
	}

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
