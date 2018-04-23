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
#include <nanogui/slidecanvasbase.h>
#include <nanogui/mediaitembase.h>

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
class NANOGUI_EXPORT SlideImage : public MediaItemBase {
    friend class Popup;
public:
    SlideImage(Widget *parent, const std::string& fileName);
    ~SlideImage();

    /// Dispose the window
    void dispose();

    /// Draw the window
    virtual void draw(NVGcontext *ctx) override;

    virtual void save(Serializer &s) const override;
    virtual bool load(Serializer &s) override;

    //TODO: Enum
    int mImageMode; //0=Crop, 1=Scale, 2=Stretch

protected:
    void drawImage(NVGcontext *ctx);

    int mImageHandle;

    //Screen ratio width/height of the target screen resolution
    float windowRatio;

    std::string mFileName;
	bool mFileLoadError;

    bool mIsXSnap;
    bool mIsYSnap;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


NAMESPACE_END(nanogui)
