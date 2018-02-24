/*
    nanogui/window.h -- Top-level window widget

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/widget.h>
#include <nanogui/slidecanvas.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

NAMESPACE_BEGIN(nanogui)

/**
 * \class SlideJpeg slidejpeg.h
 *
 * \brief Represents an image on the slide, must be place on a slidecanvas
 */
class NANOGUI_EXPORT SlideImage : public Widget {
    friend class Popup;
public:
    SlideImage(Widget *parent, const std::string& fileName);

    /// Return the panel used to house window buttons
    Widget *buttonPanel();

    /// Dispose the window
    void dispose();

    /// Draw the window
    virtual void draw(NVGcontext *ctx) override;
    /// Handle a mouse motion event (default implementation: propagate to children)
    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    /// Handle window drag events
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    /// Handle mouse events recursively and bring the current window to the top
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    /// Accept scroll events and propagate them to the widget under the mouse cursor
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;
    /// Compute the preferred size of the widget
    virtual Vector2i preferredSize(NVGcontext *ctx) const override;
    /// Invoke the associated layout generator to properly place child widgets, if any
    virtual void performLayout(NVGcontext *ctx) override;
    virtual void save(Serializer &s) const override;
    virtual bool load(Serializer &s) override;

    SlideCanvas *mCanvas;
protected:
    /// Internal helper function to maintain nested window position values; overridden in \ref Popup
    virtual void refreshRelativePlacement();

    void drawHandles(NVGcontext *ctx);
protected:
    void drawImage(NVGcontext *ctx);
    void updateImageParameters();
    Vector2f imageSizeF() const { return mImageSize.cast<float>(); }
    Vector2f positionF() const { return mPos.cast<float>(); }
    Vector2f sizeF() const { return mSize.cast<float>(); }
    Vector2f scaledImageSizeF() const { return (mScale * mImageSize.cast<float>()); }

    void center();
    void fit();

    // Image parameters.
    GLShader mShader;
    GLuint mImageID;
    Vector2i mImageSize;

    // Image display parameters.
    float mScale;
    Vector2f mOffset;
    bool mFixedScale;
    bool mFixedOffset;

    //Screen ratio width/height of the target screen resolution
    float windowRatio;

    bool mDrag;

    int mHandleSize;

    Vector2i mLastMouse;
    Vector2i mMouseDownWidgetPos;
    Vector2i mMouseDownWidgetSize;
    Vector2i mMouseDownPos;
    Vector2i mMouseDownHandle;

    //Relative to canvas
    Vector2i mCanvasImagePos;
    Vector2i mCanvasImageSize;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class GLTexture {
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
        : mTextureName(textureName), mTextureId(0) {}

    GLTexture(const std::string& textureName, GLint textureId)
        : mTextureName(textureName), mTextureId(textureId) {}

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
        : mTextureName(std::move(other.mTextureName)),
        mTextureId(other.mTextureId) {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
    *  Load a file in memory and create an OpenGL texture.
    *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
    */
    handleType load(const std::string& fileName) {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if (!textureData)
            throw std::invalid_argument("Could not load texture data from file " + fileName);
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
            case 1: internalFormat = GL_R8; format = GL_RED; break;
            case 2: internalFormat = GL_RG8; format = GL_RG; break;
            case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
            case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
            default: internalFormat = 0; format = 0; break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:


    std::string mTextureName;
    GLuint mTextureId;
};



NAMESPACE_END(nanogui)
