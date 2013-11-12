#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <X11/extensions/Xcomposite.h>

#include "window.hpp"
#include "gawmGl.hpp"

/*       _\|/_
         (o o)
 +----oOO-{_}-OOo----+
 | GawmWindow        |
 +------------------*/
GawmWindow::GawmWindow(Display *display, int screen, Window window, int x, int y, int width, int height):
	display(display),
	screen(screen),
	window(window),
	x(x),
	y(y),
	width(width),
	height(height),
	color(selectRandomColor()),
	hasPixmap(false),
	visible(false)
{
	std::cout << "CreateNotify: Vytvoreno okno " << window << " na " << x << ", " << y
			  << " velikosti " << width << "x" << height << std::endl;

	XCompositeRedirectWindow(display, window, CompositeRedirectManual);
}

GawmWindow::~GawmWindow()
{
	std::cout << "DestroyNotify: Zniceno okno " << window << std::endl;
}

void GawmWindow::configure(int newX, int newY, int newWidth, int newHeight){
	x = newX;
	y = newY;
	width = newWidth;
	height = newHeight;

	std::cout << "ConfigureNotify: Zmeneno okno " << window
				<< " s pozici " << x << ", " << y << " a velikosti "
				<< width << "x" << height << std::endl;

	hasPixmap = false;
}

void GawmWindow::reloadPixmap(){
	if (!hasPixmap && isVisible())
	{
		std::cout << "reloadPixmap(" << display << "," << window << ")" << std::endl;
		XWindowAttributes attribs;
		XGetWindowAttributes(display, window, &attribs);

		int nFbConfigs;
		auto visualid = XVisualIDFromVisual(attribs.visual);
		auto fbConfigs = glXGetFBConfigs(display, screen, &nFbConfigs);
		int i;
		for (i = 0; i < nFbConfigs; i++)
		{
			auto visinfo = glXGetVisualFromFBConfig(display, fbConfigs[i]);
			if (!visinfo || visinfo->visualid != visualid)
				continue;

			int value;
			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_DRAWABLE_TYPE, &value);
			if (!(value & GLX_PIXMAP_BIT))
				continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value);
			if (!(value & GLX_TEXTURE_2D_BIT_EXT))
				continue;

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
			if (value == False)
			{
				glXGetFBConfigAttrib(display, fbConfigs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
				if (value == False)
					continue;
			}

			glXGetFBConfigAttrib(display, fbConfigs[i], GLX_Y_INVERTED_EXT, &value);
			invertedY = (value == True);

			break;
		}

		if (i == nFbConfigs)
		{
			throw std::runtime_error("Nenašel jsem vhodný FBConfig pro pixmapu");
		}

		int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
				GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
				None };

		pixmap = XCompositeNameWindowPixmap(display, window);
		std::cout << "pixmap: " << pixmap << std::endl;
		glxPixmap = glXCreatePixmap(display, fbConfigs[i], pixmap, pixmapAttribs);
		std::cout << "glxPixmap: " << glxPixmap << std::endl;

		glGenTextures (1, &glTexture);

		XSync(display, False);
		hasPixmap = true;
		std::cout << "reload pixmapy uspesny" << std::endl;
	}
}

void GawmWindow::render(){
	reloadPixmap();

	if(isVisible())
	{
		
		// okraje oken
		GLubyte color[] = {200,200,200};
		glBegin(GL_QUADS);
		glColor3ubv(color);
		glVertex2i(x-2, y-10);
		glVertex2i(x-2, y+height+2);
		glVertex2i(x+width+2, y+height+2);
		glVertex2i(x+width+2, y-10);
		glEnd();
		
		
		glBindTexture(GL_TEXTURE_2D, glTexture);
		glXBindTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glBegin(GL_QUADS);
		//glColor3ubv(color);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(x, y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(x, y + height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(x + width, y + height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(x + width, y);
		glEnd();
		//glXReleaseTexImageEXT (display, glxPixmap, GLX_FRONT_LEFT_EXT);
		
	}
}

bool GawmWindow::isVisible()
{
	return visible;
}

void GawmWindow::setVisible(bool visible)
{
	this->visible = visible;
	reloadPixmap();
}

/*       _\|/_
         (o o)
 +----oOO-{_}-OOo----+
 | GLubyte           |
 +------------------*/
const GLubyte* selectRandomColor()
{

	static GLubyte colors[] = 
	{
		0,128,128,
		0,128,255,
		0,255,128,
		0,255,255,
		128,0,128,
		128,0,255,
		128,128,0,
		128,128,128,
		128,128,255,
		128,255,128,
		128,255,255,
		255,0,128,
		255,0,255,
		255,128,0,
		255,128,128,
		255,128,255,
		255,255,0,
		255,255,128,
		255,255,255
	};
	
	return colors + (rand() % 19)*3;
}
