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
#include <nanogui/slidecanvas.h>
#include <math.h>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

NAMESPACE_BEGIN(nanogui)

SlideImage::SlideImage(Widget *parent, const std::string& fileName)
    : MediaItemBase(parent),
		mImageMode(1), //Image mode to scaling
		mImageHandle(0), //unloaded state
		mFileLoadError(false){
	mFileName = fileName;
}

SlideImage::~SlideImage()
{
    //delete texture;
}


void SlideImage::draw(NVGcontext *ctx) {

    nvgSave(ctx);
    
	//Draw the image
	drawImage(ctx);

	nvgRestore(ctx);

	//Draw handles, borders and what not
    MediaItemBase::draw(ctx);
}

void SlideImage::drawImage(NVGcontext *ctx){
	if (mImageHandle == 0 && mFileLoadError == false) {
		mImageHandle = nvgCreateImage(ctx, mFileName.c_str(), 0);
		if (mImageHandle == 0) {
			printf("Error opening file: %s\n", mFileName.c_str());
			mFileLoadError = true;
			return;
		}
		else {
			printf("Loaded the file.\n");
		}
	}

	if (mFileLoadError) {
		return;
	}

	int w, h;

	nvgImageSize(ctx, mImageHandle, &w, &h);

	float inRatio = ((float)w)/h;
	float outRatio = ((float)(mSize.x()-mHandleSize))/(mSize.y()-mHandleSize);
	int maxOutputWidth = mSize.x()-mHandleSize, maxOutputHeight = mSize.y()-mHandleSize;
	int outputWidth = 0, outputHeight = 0;

	//0=Crop, 1=Scale, 2=Stretch
	switch(mImageMode)
	{
		case 0:
			if(inRatio > outRatio)
			{
				outputHeight = maxOutputHeight;
				outputWidth = ((float)maxOutputHeight)*inRatio;
			}else{
				outputWidth = maxOutputWidth;
				outputHeight = ((float)maxOutputWidth)/inRatio;
			}
			break;
		case 1:
			if(inRatio < outRatio)
			{
				outputHeight = maxOutputHeight;
				outputWidth = ((float)maxOutputHeight)*inRatio;
			}else{
				outputWidth = maxOutputWidth;
				outputHeight = ((float)maxOutputWidth)/inRatio;
			}
			break;
		case 2:
			outputHeight = maxOutputHeight;
			outputWidth = maxOutputWidth;
			break;
		default:
			printf("Bad image mode, setting to scale\n");
			mImageMode = 1;
			break;
	}



	NVGpaint imgPaint = nvgImagePattern(ctx,
			mPos.x()+(mSize.x()/2)-outputWidth/2,
			mPos.y()+(mSize.y()/2)-outputHeight/2,
			outputWidth,outputHeight,
			0, mImageHandle, 1);

	nvgBeginPath(ctx);

	if(mImageMode == 1)
		nvgRect(ctx,
				mPos.x()+(mSize.x()/2)-outputWidth/2,
				mPos.y()+(mSize.y()/2)-outputHeight/2,
				outputWidth,outputHeight);
	else
		nvgRect(ctx, mPos.x()+mHandleSize/2,mPos.y()+mHandleSize/2,mSize.x()-mHandleSize,mSize.y()-mHandleSize);

	nvgFillPaint(ctx, imgPaint);
	nvgFill(ctx);
}

void SlideImage::dispose() {
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
