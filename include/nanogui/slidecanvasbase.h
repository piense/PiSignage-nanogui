/*
    nanogui/slidecanvasbase.h -- Base class of slidecanvas

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/object.h>
#include <nanogui/theme.h>
#include <vector>

NAMESPACE_BEGIN(nanogui)

class SlideImage;


/**
 * \class SlideCanvasBase slidecanvasbase.h nanogui/slidecanvasbase.h
 *
 * \brief Base class of all widgets.
 *
 * \ref
 */
class NANOGUI_EXPORT SlideCanvasBase {
public:
    Vector2i mCanvasPos;
    Vector2i mCanvasSize;
    virtual void ImageItemUpdate(SlideImage *image) = 0;
    virtual void ImageLostFocus(SlideImage *image) = 0;
};

NAMESPACE_END(nanogui)
