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
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

NAMESPACE_BEGIN(nanogui)

namespace {
    std::vector<std::string> tokenize(const std::string &string,
                                      const std::string &delim = "\n",
                                      bool includeEmpty = false) {
        std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
        std::vector<std::string> tokens;

        while (lastPos != std::string::npos) {
            std::string substr = string.substr(lastPos, pos - lastPos);
            if (!substr.empty() || includeEmpty)
                tokens.push_back(std::move(substr));
            lastPos = pos;
            if (lastPos != std::string::npos) {
                lastPos += 1;
                pos = string.find_first_of(delim, lastPos);
            }
        }

        return tokens;
    }

    constexpr char const *const defaultImageViewVertexShader =
        R"(#version 330
        uniform vec2 scaleFactor;
        uniform vec2 position;
        in vec2 vertex;
        out vec2 uv;
        void main() {
            uv = vertex;
            vec2 scaledVertex = (vertex * scaleFactor) + position;
            gl_Position  = vec4(2.0*scaledVertex.x - 1.0,
                                1.0 - 2.0*scaledVertex.y,
                                0.0, 1.0);

        })";

    constexpr char const *const defaultImageViewFragmentShader =
        R"(#version 330
        uniform sampler2D image;
        out vec4 color;
        in vec2 uv;
        void main() {
            color = texture(image, uv);
        })";

}


SlideImage::SlideImage(Widget *parent, const std::string& fileName)
    : Widget(parent), mCanvasImagePos(.5,.5), mCanvasImageSize(.25,.25), mImageID(0), mScale(1.0f),
	  mOffset(Vector2f::Zero()), mFixedScale(false), mFixedOffset(false), mImageID(0){
	mPos = {40,40};
	mSize = {90, 90};
	mHandleSize = 10;
	mDrag = false;

    GLTexture texture("SlideImage");
    auto data = texture.load(fileName);


	mImageID = texture.texture();
	updateImageParameters();
	fit();

	//Image Init stuff
    updateImageParameters();
    mShader.init("ImageViewShader", defaultImageViewVertexShader,
                 defaultImageViewFragmentShader);

    MatrixXu indices(3, 2);
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 1;

    MatrixXf vertices(2, 4);
    vertices.col(0) << 0, 0;
    vertices.col(1) << 1, 0;
    vertices.col(2) << 0, 1;
    vertices.col(3) << 1, 1;

    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib("vertex", vertices);
}

SlideImage::~SlideImage()
{
    mShader.free();
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
    nvgRect(ctx, mPos.x()+mHandleSize/2, mPos.y()+mHandleSize/2,
    		mSize.x()-mHandleSize, mSize.y()-mHandleSize);
    NVGcolor col = mMouseFocus ? NVGcolor{1,0,0,1} : NVGcolor{1,0,0,1};
    nvgStrokeColor(ctx, col);
    nvgStroke(ctx);

    if(mFocused)
    	drawHandles(ctx);

    nvgRestore(ctx);

    Widget::draw(ctx);
}

void SlideImage::drawImage(NVGcontext *ctx){
    // Calculate several variables that need to be send to OpenGL in order for the image to be
    // properly displayed inside the widget.
    const Screen* screen = dynamic_cast<const Screen*>(this->window()->parent());
    assert(screen);
    Vector2f screenSize = screen->size().cast<float>();
    Vector2f scaleFactor = mScale * imageSizeF().cwiseQuotient(screenSize);
    Vector2f positionInScreen = absolutePosition().cast<float>();
    Vector2f positionAfterOffset = positionInScreen + mOffset;
    Vector2f imagePosition = positionAfterOffset.cwiseQuotient(screenSize);
    glEnable(GL_SCISSOR_TEST);
    float r = screen->pixelRatio();
    glScissor(positionInScreen.x() * r,
              (screenSize.y() - positionInScreen.y() - size().y()) * r,
              size().x() * r, size().y() * r);
    mShader.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mImageID);
    mShader.setUniform("image", 0);
    mShader.setUniform("scaleFactor", scaleFactor);
    mShader.setUniform("position", imagePosition);
    mShader.drawIndexed(GL_TRIANGLES, 0, 2);
    glDisable(GL_SCISSOR_TEST);

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


void SlideImage::updateImageParameters() {
    // Query the width of the OpenGL texture.
    glBindTexture(GL_TEXTURE_2D, mImageID);
    GLint w, h;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    mImageSize = Vector2i(w, h);
}

void SlideImage::center() {
    mOffset = (sizeF() - scaledImageSizeF()) / 2;
}

void SlideImage::fit() {
    // Calculate the appropriate scaling factor.
    mScale = (sizeF().cwiseQuotient(imageSizeF())).minCoeff();
    center();
}






NAMESPACE_END(nanogui)
