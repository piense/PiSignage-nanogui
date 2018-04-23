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

NAMESPACE_BEGIN(nanogui)

/**
 * \class SlideCanvas slidecanvas.h slidecanvas/window.h
 *
 * \brief Basically an empty area to drag widgets around.
 */
class NANOGUI_EXPORT SlideCanvas : public Widget, public SlideCanvasBase {
    friend class Popup;
public:
    SlideCanvas(Widget *parent);

    /// Return the panel used to house window buttons
    Widget *buttonPanel();

    /// Dispose the window
    void dispose();

    /// Center the window in the current \ref Screen
    void center();

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

    //TODO: Generalize with MediaItem base class
    //virtual void ImageItemUpdate(SlideImage *image) override;
    //virtual void ImageLostFocus(SlideImage *image) override;

    SlideImage *selectedImage(){return mSelectedImage;}
protected:
    /// Internal helper function to maintain nested window position values; overridden in \ref Popup
    virtual void refreshRelativePlacement();
protected:
    //Screen ratio width/height of the target screen resolution
    float windowRatio;

    SlideImage *mSelectedImage;


public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

NAMESPACE_END(nanogui)
