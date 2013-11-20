#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

#include "debug.hpp"
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
	dbg_w_create << "CreateNotify: Vytvoreno okno " << window << " na " << x << ", " << y
			  << " velikosti " << width << "x" << height << std::endl;
	
	damage = XDamageCreate(display, window, XDamageReportNonEmpty);
}

GawmWindow::~GawmWindow()
{
	dbg_w_destroy << "DestroyNotify: Zniceno okno " << window << std::endl;
	XDamageDestroy(display, damage);
}

void GawmWindow::configure(int newX, int newY, int newWidth, int newHeight){
	x = newX;
	y = newY;
	width = newWidth;
	height = newHeight;

	dbg_w_conf << "ConfigureNotify: Zmeneno okno " << window
				<< " s pozici " << x << ", " << y << " a velikosti "
				<< width << "x" << height << std::endl;

	hasPixmap = false;
}

void GawmWindow::reloadPixmap(){
	if (!hasPixmap && isVisible())
	{
		dbg_w_pixmap << "reloadPixmap(" << display << "," << window << ")" << std::endl;
		XWindowAttributes attribs;
		XGetWindowAttributes(display, window, &attribs);

		int nFbConfigs;
		auto visualid = XVisualIDFromVisual(attribs.visual);
		auto fbConfigs = glXGetFBConfigs(display, screen, &nFbConfigs); // FIXME: Způsobuje leaky!
		int i;
		for (i = 0; i < nFbConfigs; i++)
		{
			auto visinfo = glXGetVisualFromFBConfig(display, fbConfigs[i]); // FIXME: Způsobuje leaky!
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

		XWindowAttributes wAttr;
		int status = XGetWindowAttributes(display, window, &wAttr);
		if (status == 0)
		{
			throw std::runtime_error("Nelze získat atributy okna");
		}
		
		if (wAttr.map_state == IsViewable)
		{
			pixmap = XCompositeNameWindowPixmap(display, window);
			dbg_w_pixmap << "pixmap: " << pixmap << std::endl;
			glxPixmap = glXCreatePixmap(display, fbConfigs[i], pixmap, pixmapAttribs); // FIXME: Způsobuje leaky!
			dbg_w_pixmap << "glxPixmap: " << glxPixmap << std::endl;

			glGenTextures (1, &glTexture); // FIXME: Způsobuje leaky!

			XSync(display, False);
			hasPixmap = true;
			dbg_w_pixmap << "reload pixmapy uspesny" << std::endl;
		}
		else
		{
			cerr_line << "reload pixmapy neprobehl, okno neni viewable, tohle by asi nemelo nastat" << std::endl;
		}
	}
}

void GawmWindow::render(double zoom){
	

	if(isVisible())
	{
		glPushMatrix();
		glScaled(zoom, zoom, 1.0);
		
		reloadPixmap();
		
		// pozadi dekorace
		glBindTexture(GL_TEXTURE_2D, 0);
		GLubyte color[] = {150,150,150};
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(color);
		glVertex2i(x-borderLeft, y-borderTop);
		glVertex2i(x-borderLeft, y+height+borderBottom);
		glVertex2i(x+width+borderRight, y+height+borderBottom);
		glVertex2i(x+width+borderRight, y-borderTop);
		glEnd();
		
		// okraje dekorace
		GLubyte color2[] = {0,0,0};
		glBegin(GL_LINES);
		glColor3ubv(color2);
		glVertex2i(x-borderLeft, y-borderTop);
		glVertex2i(x-borderLeft, y+height+borderBottom);
		glVertex2i(x-borderLeft, y+height+borderBottom);
		glVertex2i(x+width+borderRight, y+height+borderBottom);
		glVertex2i(x+width+borderRight, y+height+borderBottom);
		glVertex2i(x+width+borderRight, y-borderTop);
		glVertex2i(x+width+borderRight, y-borderTop);
		glVertex2i(x-borderLeft, y-borderTop);
		glEnd();
		
		// obsah okna
		glBindTexture(GL_TEXTURE_2D, glTexture);
		glXBindTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr); // FIXME: Způsobuje leaky!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		GLubyte color3[] = {255,255,255};
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(color3);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(x, y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(x, y + height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(x + width, y + height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(x + width, y);
		glEnd();
		glXReleaseTexImageEXT (display, glxPixmap, GLX_FRONT_LEFT_EXT);
		
		glPopMatrix();
	}
}

bool GawmWindow::isVisible()
{
	return visible;
}

void GawmWindow::setVisible(bool visible)
{
	this->visible = visible;
}

void GawmWindow::doDamage()
{
	hasPixmap = false;
}

bool GawmWindow::containsPoint(int pX, int pY)
{	
	return (x-borderLeft <= pX && pX <= x+width+borderRight) &&
	       (y-borderTop  <= pY && pY <= y+height+borderBottom);
}

bool GawmWindow::handlePoint(int pX, int pY)
{	
	return (x-borderLeft <= pX && pX <= x+width+borderRight) &&
	       (y-borderTop  <= pY && pY < y);
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
