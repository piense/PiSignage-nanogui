/*
    src/window.cpp -- Top-level window widget

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/slideimage.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/serializer/core.h>
#include <nanogui/slidecanvas.h>
#include <math.h>

NAMESPACE_BEGIN(nanogui)

SlideImage::SlideImage(Widget *parent)
    : Widget(parent), mImagePos(.5,.5), mImageSize(.25,.25) {
	mPos = {40,40};
	mSize = {90, 90};
	mHandleSize = 10;
	mDrag = false;
}

Vector2i SlideImage::preferredSize(NVGcontext *ctx) const {
    Vector2i result = {mCanvas->mCanvasSize.x() * mImageSize.x(),
    		mCanvas->mCanvasSize.y() * mImageSize.y()};

    result = mSize;

    return result;
}

void SlideImage::performLayout(NVGcontext *ctx) {

	Widget::performLayout(ctx);
}

void SlideImage::draw(NVGcontext *ctx) {
    int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;
    int hh = mTheme->mWindowHeaderHeight;

    /* Draw window */
    nvgSave(ctx);

    /*
    mSize = {mCanvas->mCanvasSize.x() * mImageSize.x(),
    		mCanvas->mCanvasSize.y() * mImageSize.y()};*/

    //Outer widget rectangle
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x()+mHandleSize/2, mPos.y()+mHandleSize/2,
    		mSize.x()-mHandleSize, mSize.y()-mHandleSize, cr);
    NVGcolor col = mMouseFocus ? NVGcolor{1,0,0,1} : NVGcolor{1,0,0,1};
    nvgFillColor(ctx, col);
    nvgFill(ctx);

    if(mFocused)
    	drawHandles(ctx);

    nvgRestore(ctx);

    Widget::draw(ctx);
}

void SlideImage::drawHandles(NVGcontext *ctx){
	nvgFillColor(ctx, NVGcolor{0,0,0,1});

	int x[3] = {0,mSize.x()/2-mHandleSize/2,mSize.x()-mHandleSize};
	int y[3] = {0,mSize.y()/2-mHandleSize/2,mSize.y()-mHandleSize};

	bool overHandle = false;

	for(int xL = 0;xL<3;xL++){
		for(int yL = 0;yL<3;yL++){
			nvgBeginPath(ctx);
			nvgRect(ctx, x[xL]+mPos.x(), y[yL]+mPos.y(), mHandleSize, mHandleSize);

			//Is the mouse over the image?
			if(mMouseFocus)
			{
				//Yup, see if it's over the handle
				if(mLastMouse.x() < x[xL]+mHandleSize && mLastMouse.x() > x[xL] &&
					mLastMouse.y() < y[yL]+mHandleSize && mLastMouse.y() > y[yL]){
					nvgFillColor(ctx, NVGcolor{1,1,1,1});
					overHandle = true;
				}
				else
					nvgFillColor(ctx, NVGcolor{0,0,0,1});
			}else{
				nvgFillColor(ctx, NVGcolor{0,0,0,1});
			}

			nvgFill(ctx);
		}
	}

	//Highlight the middle handle to show we're moving
	if(mMouseFocus && !overHandle){
		nvgBeginPath(ctx);
		nvgRect(ctx, x[1]+mPos.x(), y[1]+mPos.y(), mHandleSize, mHandleSize);
		nvgFillColor(ctx, NVGcolor{1,1,1,1});
		nvgFill(ctx);
	}
}

void SlideImage::dispose() {

}

void SlideImage::center() {
}

bool SlideImage::mouseDragEvent(const Vector2i &p, const Vector2i &rel,
                            int button, int /* modifiers */) {

	Vector2i cp = p.cwiseMax(Vector2i::Zero()).cwiseMin(mParent->size());
	Vector2i delta = cp - mMouseDownPos;

	//See if it's over the handle
	if(mMouseDownHandle.x() < 3 && mMouseDownHandle.y()<3)
	{
		if(mMouseDownHandle.x() == 1 &&  mMouseDownHandle.y() == 1) //Center
		{
			mPos = delta + mMouseDownWidgetPos;
		}

		if( mMouseDownHandle.y() == 0){ //Top Row
			mSize.y() = mMouseDownWidgetSize.y() - delta.y();
			mPos.y() = std::fmin(mMouseDownWidgetPos.y() + delta.y(),
					mMouseDownWidgetSize.y()+mMouseDownWidgetPos.y());
		}

		if( mMouseDownHandle.y() == 2){ //Bottom Row
			mSize.y() = mMouseDownWidgetSize.y() + cp.y() - mMouseDownPos.y();
			mPos.y() = std::fmin(mMouseDownWidgetPos.y() + mMouseDownWidgetSize.y() + delta.y(),
					mMouseDownWidgetPos.y());
		}

		if(mMouseDownHandle.x() == 0){ //Left Column
			mSize.x() = mMouseDownWidgetSize.x() - delta.x();
			mPos.x() = std::fmin(mMouseDownWidgetPos.x() + delta.x(),
					mMouseDownWidgetSize.x()+mMouseDownWidgetPos.x());
			}

		if( mMouseDownHandle.x() == 2){ //Right Column
			mSize.x() = mMouseDownWidgetSize.x() + delta.x();
			mPos.x() = std::fmin(mMouseDownWidgetPos.x() + mMouseDownWidgetSize.x() + delta.x(),
					mMouseDownWidgetPos.x());
		}

		mSize = mSize.cwiseAbs();//TODO
	}else{
		//Not over a handle
	}

    return false;
}

bool SlideImage::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){
	mLastMouse = p-mPos;
	return Widget::mouseMotionEvent(p, rel, button, modifiers);
}

bool SlideImage::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    if( button == GLFW_MOUSE_BUTTON_1){
    	mMouseDownPos = p;
    	mMouseDownWidgetSize = mSize;
    	mMouseDownWidgetPos = mPos;

    	Vector2i PosOnWidget = p-mPos;

    	int x[3] = {0,mMouseDownWidgetSize.x()/2-mHandleSize/2,mMouseDownWidgetSize.x()-mHandleSize};
    	int y[3] = {0,mMouseDownWidgetSize.y()/2-mHandleSize/2,mMouseDownWidgetSize.y()-mHandleSize};

    	//Default to middle handle so clicking not on
    	//a handle drags the whole object
    	mMouseDownHandle = {1,1};

    	for(int xL = 0;xL<3;xL++){
    		for(int yL = 0;yL<3;yL++){
				//Yup, see if it's over the handle
				if(PosOnWidget.x() < x[xL]+mHandleSize && PosOnWidget.x() > x[xL] &&
						PosOnWidget.y() < y[yL]+mHandleSize && PosOnWidget.y() > y[yL])
				{
					mMouseDownHandle = {xL,yL};
					xL = 3;
					yL = 3;
				}
    		}
    	}

    	return true;
    }

    return false;
}

bool SlideImage::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    Widget::scrollEvent(p, rel);
    return true;
}

void SlideImage::refreshRelativePlacement() {
    /* Overridden in \ref Popup */
}

void SlideImage::save(Serializer &s) const {
    Widget::save(s);
}

bool SlideImage::load(Serializer &s) {
    if (!Widget::load(s)) return false;
//    if (!s.get("title", mTitle)) return false;
//    if (!s.get("modal", mModal)) return false;

    return true;
}

NAMESPACE_END(nanogui)
