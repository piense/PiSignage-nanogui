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
#include <nanogui/window.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/serializer/core.h>
#include <nanogui/mediaitembase.h>
#include <math.h>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

NAMESPACE_BEGIN(nanogui)

MediaItemBase::MediaItemBase(Widget *parent)
    : Widget(parent), mCanvasPos(.5,.5), mCanvasSize(.25,.25),
	  mIsXSnap(false), mIsYSnap(false){
	mPos = {40,40};
	mSize = {90, 90};
	mHandleSize = 10;
	mDrag = false;
}

/*
MediaItemBase::~MediaItemBase()
{
    //delete texture;
}*/

Vector2i MediaItemBase::preferredSize(NVGcontext *ctx) const {
    Vector2i result = mSize;
    return result;
}

void MediaItemBase::performLayout(NVGcontext *ctx) {
	//TODO: Compensate for edges
	mSize.x() = mCanvas->mCanvasSize.x() * mCanvasSize.x();
	mSize.y() = mCanvas->mCanvasSize.y() * mCanvasSize.y();

	mPos.x() = mCanvas->mCanvasSize.x() * mCanvasPos.x() + mCanvas->mCanvasPos.x() - mSize.x() / 2.0;
	mPos.y() = mCanvas->mCanvasSize.y() * mCanvasPos.y() + mCanvas->mCanvasPos.y() - mSize.y() / 2.0;
}

//Need to figure out how this aligns with the 
//parent classes. Should probably be called
//after the parent classes drawing,
//or maybe we create a virtual method for the parent class to
//override.
void MediaItemBase::draw(NVGcontext *ctx) {
    nvgSave(ctx);

    //Outer widget rectangle
    if(mFocused){
		nvgBeginPath(ctx);
		nvgRect(ctx, mPos.x()+mHandleSize/2, mPos.y()+mHandleSize/2,
				mSize.x()-mHandleSize, mSize.y()-mHandleSize);
		NVGcolor col = NVGcolor{1,0,0,1};
		nvgStrokeColor(ctx, col);
		nvgStroke(ctx);

		drawHandles(ctx);
    }

    drawSnaps(ctx);

    nvgRestore(ctx);

    Widget::draw(ctx);
}

void MediaItemBase::drawSnaps(NVGcontext *ctx){
	int thickness = 2;

	if(mIsXSnap){
	    nvgSave(ctx);
	    nvgScissor(ctx,0,0,mParent->width(),mParent->height());
		nvgBeginPath(ctx);
		nvgRect(ctx, mCanvas->mCanvasPos.x() + mCanvas->mCanvasSize.x()/2 - thickness/2,
				mCanvas->mCanvasPos.y(),
				thickness,
				mCanvas->mCanvasSize.y());
		NVGcolor col = NVGcolor{1,1,1,1};
		nvgFillColor(ctx, col);
		nvgFill(ctx);
		nvgRestore(ctx);
	}

	if(mIsYSnap){
	    nvgSave(ctx);
	    nvgScissor(ctx,0,0,mParent->width(),mParent->height());
		nvgBeginPath(ctx);
		nvgRect(ctx,
				mCanvas->mCanvasPos.x(),
				mCanvas->mCanvasPos.y() + mCanvas->mCanvasSize.y()/2 - thickness/2,
				mCanvas->mCanvasSize.x(),
				thickness);
		NVGcolor col = NVGcolor{1,1,1,1};
		nvgFillColor(ctx, col);
		nvgFill(ctx);
		nvgRestore(ctx);
	}
}

void MediaItemBase::drawHandles(NVGcontext *ctx){
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
				if(mLastMouse.x() < x[xL]+mHandleSize &&
					mLastMouse.x() > x[xL]-mHandleSize &&
					mLastMouse.y() < y[yL]+mHandleSize &&
					mLastMouse.y() > y[yL]-mHandleSize){

					nvgFillColor(ctx, NVGcolor{1,1,1,1});
					overHandle = true;
				}
				else
					nvgFillColor(ctx, NVGcolor{1,0,0,1});
			}else{
				nvgFillColor(ctx, NVGcolor{1,0,0,1});
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

void MediaItemBase::dispose() {
}

bool MediaItemBase::mouseDragEvent(const Vector2i &p, const Vector2i &rel,
                            int button, int /* modifiers */) {

	Vector2i cp = p.cwiseMax(mCanvas->mCanvasPos).cwiseMin(mCanvas->mCanvasPos+mCanvas->mCanvasSize);
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


	//TODO: Generalize snapping to all handles. Something like: Pos Snap(Pos)
	//X Position Snap
	if( std::abs((mCanvas->mCanvasPos.x() + mCanvas->mCanvasSize.x()/2) - (mPos.x() + mSize.x()/2)) < 25
			&& mMouseDownHandle.x() == 1 &&  mMouseDownHandle.y() == 1){
		mPos.x() = mCanvas->mCanvasPos.x() + mCanvas->mCanvasSize.x()/2 - mSize.x()/2;
		mIsXSnap = true;
	}else
	{
		mIsXSnap = false;
	}

	//Y Position Snap
	if( std::abs((mCanvas->mCanvasPos.y() + mCanvas->mCanvasSize.y()/2) - (mPos.y() + mSize.y()/2)) < 25
			&& mMouseDownHandle.x() == 1 &&  mMouseDownHandle.y() == 1){
		mPos.y() = mCanvas->mCanvasPos.y() + mCanvas->mCanvasSize.y()/2 - mSize.y()/2;
		mIsYSnap = true;
	}else{
		mIsYSnap = false;
	}

	UpdateCanvasCoordinates();
	//mCanvas->ImageItemUpdate(this);

    return true;
}

void MediaItemBase::UpdateCanvasCoordinates(){
    mCanvasPos.x() = ((float) (mPos.x()+mSize.x()/2) - mCanvas->mCanvasPos.x()) / mCanvas->mCanvasSize.x();
    mCanvasPos.y() = ((float) (mPos.y()+mSize.y()/2) - mCanvas->mCanvasPos.y()) / mCanvas->mCanvasSize.y();
    mCanvasSize.x() = ((float)(mSize.x()-mHandleSize)) / mCanvas->mCanvasSize.x();
    mCanvasSize.y() = ((float)(mSize.y()-mHandleSize)) / mCanvas->mCanvasSize.y();
}

bool MediaItemBase::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers){
	mLastMouse = p-mPos;
	return Widget::mouseMotionEvent(p, rel, button, modifiers);
}

bool MediaItemBase::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
	if (down == false)
	{
		mIsYSnap = false;
		mIsXSnap = false;
	}

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
				if(PosOnWidget.x() < x[xL]+mHandleSize &&
					PosOnWidget.x() > x[xL]-mHandleSize &&
					PosOnWidget.y() < y[yL]+mHandleSize &&
					PosOnWidget.y() > y[yL]-mHandleSize)
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

bool MediaItemBase::focusEvent(bool focused)
{
	UpdateCanvasCoordinates();
/*	if(true) //Should be (focused)????
		mCanvas->ImageItemUpdate(this);
	if(false)//Should be (!focused)???
		mCanvas->ImageLostFocus(this);*/
	return Widget::focusEvent(focused);
}

bool MediaItemBase::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    Widget::scrollEvent(p, rel);
    return true;
}

void MediaItemBase::refreshRelativePlacement() {
    /* Overridden in \ref Popup */
}

void MediaItemBase::save(Serializer &s) const {
    Widget::save(s);
}

bool MediaItemBase::load(Serializer &s) {
    if (!Widget::load(s)) return false;
//    if (!s.get("title", mTitle)) return false;
//    if (!s.get("modal", mModal)) return false;

    return true;
}

NAMESPACE_END(nanogui)
