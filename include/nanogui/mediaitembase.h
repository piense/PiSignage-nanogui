#pragma once

/*
    nanogui/mediaitembase.h -- Base for all slide media items

	PiSigage by David Corrigan

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

//Responsible for media items on the canvas
//and maybe media items not on the canvas - we'll see
//Supports positioning, sizing, dragging, and resizing of the item
//Also provides virtual hooks to populate the properties pane

/**
 * \class SlideJpeg slidejpeg.h
 *
 * \brief Represents an image on the slide, must be place on a slidecanvas
 */
class NANOGUI_EXPORT MediaItemBase : public Widget {

public:
	MediaItemBase(Widget *parent);

    /// Return the panel used to house window buttons
    //Widget *buttonPanel(); //idk if this is necessary

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
    virtual bool focusEvent(bool focused) override;

	//The properties panel controls for the media item
	virtual Widget *initPropertiesPanel(Window *parent) = 0;

	//Item's rectangle on the canvas
    Vector2f mCanvasSize; //0-1 tuple, 0,0 is top left
    Vector2f mCanvasPos; //0-1 tuple, 0,0 is top left

    //From Widget: Vector2i mPos, mSize; Relative to parent frame

	SlideCanvasBase *mCanvas;

protected:
    /// Internal helper function to maintain nested window position values; overridden in \ref Popup
    virtual void refreshRelativePlacement();

    void drawHandles(NVGcontext *ctx);
    void drawSnaps(NVGcontext *ctx);

    bool mDrag;

    //Size of drag handles on canvas
    int mHandleSize;

    Vector2i mLastMouse;
    Vector2i mMouseDownWidgetPos;
    Vector2i mMouseDownWidgetSize;
    Vector2i mMouseDownPos;
    Vector2i mMouseDownHandle;

    void UpdateCanvasCoordinates();

    bool mIsXSnap;
    bool mIsYSnap;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


NAMESPACE_END(nanogui)
