/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/slidecanvas.h>
#include <nanogui/slideimage.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") { // @suppress("Class members should be properly initialized")
        using namespace nanogui;

        Window *window = new Window(this, "Slide");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        new Label(window, "Dissolve Time :", "sans-bold");
		mDissolveTime = new TextBox(window);
		mDissolveTime->setEditable(true);
		mDissolveTime->setFixedSize(Vector2i(100, 20));
		mDissolveTime->setValue("5.0");
		mDissolveTime->setUnits("Sec.");
		mDissolveTime->setDefaultValue("1.0");
		mDissolveTime->setFontSize(16);
		mDissolveTime->setFormat("^[0-9]*\\.?[0-9]+$");

		new Label(window, "Duration:", "sans-bold");
		mSlideTime = new TextBox(window);
		mSlideTime->setEditable(true);
		mSlideTime->setFixedSize(Vector2i(130, 20));
		mSlideTime->setValue("10.0");
		mSlideTime->setUnits("Sec.");
		mSlideTime->setDefaultValue("1.0");
		mSlideTime->setFontSize(16);
		mSlideTime->setFormat("^[0-9]*\\.?[0-9]+$");

		new Label(window, "Screen Dimensions:", "sans-bold");
		mScreenSize = new TextBox(window);
		mScreenSize->setEditable(true);
		mScreenSize->setFixedSize(Vector2i(130, 20));
		mScreenSize->setValue("1920 x 1080");
		mScreenSize->setUnits("Px");
		mScreenSize->setDefaultValue("1920");
		mScreenSize->setFontSize(16);
		mScreenSize->setFormat("^[0-9]{1,4}\\s*[x]\\s*[0-9]{1,4}$");
		mScreenSize->setCallback( [&] (const std::string& str) {
			UpdateScreenSize( str );
			return true;
		});


        auto imageWindowLayout = new AdvancedGridLayout({0},{30,15,0},10);
        imageWindowLayout->setColStretch(0,1);
        imageWindowLayout->setRowStretch(2,1);

        auto imageWindow = new Window(this, "Slide Canvas");
        imageWindow->setPosition(Vector2i(250, 15));
        imageWindow->setSize(Vector2i(900,700));
        imageWindow->setLayout(imageWindowLayout);
        mSlideCanvas = new SlideCanvas(imageWindow);
        imageWindowLayout->setAnchor(mSlideCanvas, AdvancedGridLayout::Anchor(0,2));

        Widget *tools = new Widget(imageWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));
        imageWindowLayout->setAnchor(tools,AdvancedGridLayout::Anchor(0,0));

        SlideImage *imagetest = new SlideImage(mSlideCanvas,"/home/david/Desktop/Cute Photos/IMG_6253.jpg");
        imagetest->mCanvas = mSlideCanvas;

        imagetest = new SlideImage(mSlideCanvas,"/home/david/Desktop/Cute Photos/P1000252.jpg");
        imagetest->mCanvas = mSlideCanvas;

        Button *b = new Button(tools,"Add JPEG");
        b->setCallback([&] {
        	string file = file_dialog(
                    {  {"jpg", "jpg file"},  {"jpeg", "jpeg file"} }, false);
            cout << "Adding image: " << file.c_str() << endl;
            imagetest = new SlideImage(mSlideCanvas,file);
            imagetest->mCanvas = mSlideCanvas;
        });
        b = new Button(tools,"Add Text");
        b->setCallback([&] {cout<<"Add Text\n";});

        window = new Window(this, "Properties");
        window->setPosition(Vector2i(15, 275));
        window->setLayout(new GroupLayout());


        //This will probably get moved into the specific MediaItem classes somehow
        /*
		new Label(window, "Position:", "sans-bold");
		mImagePosition = new TextBox(window);
		mImagePosition->setEditable(true);
		mImagePosition->setFixedSize(Vector2i(130, 20));
		mImagePosition->setValue("800, 400");
		mImagePosition->setUnits("Px");
		mImagePosition->setDefaultValue("1080");
		mImagePosition->setFontSize(16);
		mImagePosition->setFormat("^[0-9]{1,4}\\s*[,]\\s*[0-9]{1,4}$");

		new Label(window, "Size:", "sans-bold");
		mImageSize = new TextBox(window);
		mImageSize->setEditable(true);
		mImageSize->setFixedSize(Vector2i(130, 20));
		mImageSize->setValue("800 x 400");
		mImageSize->setUnits("Px");
		mImageSize->setDefaultValue("1080");
		mImageSize->setFontSize(16);
		mImageSize->setFormat("^[0-9]{1,4}\\s*[,]\\s*[0-9]{1,4}$");

        new Label(window, "Image File:", "sans-bold");
        new Label(window, "test.jpeg", "sans-bold");
        tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Change Image");
        b->setCallback([&] {

        });

        new Label(window, "Image Scaling", "sans-bold");
        mImageScaling = new ComboBox(window, { "Crop", "Fit", "Fill"});
        mImageScaling->setCallback( [&] (const int i) {
        	if(mSlideCanvas->selectedImage() != NULL)
        		{
        			mSlideCanvas->selectedImage()->mImageMode = i;
        		}
			return true;
		});*/


        performLayout();

    }

    ~ExampleApplication() {

    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {

    	//TODO: Make this event driven

    	char tempString[200];
    	/*
    	if(mSlideCanvas->selectedImage() != NULL){
    		sprintf(&tempString[0],"%d, %d",
    				(uint32_t)(mSlideCanvas->selectedImage()->mCanvasImagePos.x()*mScreenWidth),
    						(uint32_t)(mSlideCanvas->selectedImage()->mCanvasImagePos.y()*mScreenHeight));
    		mImagePosition->setValue(std::string(&tempString[0]));

    		sprintf(&tempString[0],"%d x %d",
    		    				(uint32_t)(mSlideCanvas->selectedImage()->mCanvasImageSize.x()*mScreenWidth),
								(uint32_t)(mSlideCanvas->selectedImage()->mCanvasImageSize.y()*mScreenHeight));
			mImageSize->setValue(std::string(&tempString[0]));
    	}*/

        /* Draw the user interface */
        Screen::draw(ctx);
    }

    virtual void drawContents() {
        using namespace nanogui;
    }

    void UpdateScreenSize( const std::string &str)
    {
    	sscanf(str.c_str(),"%d x %d",&mScreenWidth,&mScreenHeight);
    }


    void UpdateScreenToTextbox(){}
private:
    nanogui::SlideCanvas *mSlideCanvas;
    nanogui::TextBox *mImagePosition, *mImageSize;
    nanogui::TextBox *mScreenSize;
    nanogui::TextBox *mDissolveTime, *mSlideTime;
    nanogui::ComboBox *mImageScaling;

    uint32_t mDissolveTimeMillis, mSlideTimeMillis;
    uint32_t mScreenWidth, mScreenHeight;
    uint32_t mImageWidth, mImageHeight;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
