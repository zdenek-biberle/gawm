#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

#include "debug.hpp"
#include "window.hpp"
#include "gawmGl.hpp"
#include "utils.hpp"

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
    destroyPixmap();
	// XDamageDestroy(display, damage); // tohle asi nepotrebujeme, Xka to znici samy pri zniceni okna, asi
}

void GawmWindow::configure(int newX, int newY, int newWidth, int newHeight)
{
    if (width != newWidth || height != newHeight)
    {
        hasPixmap = false;
    }
    
	x = newX;
	y = newY;
	width = newWidth;
	height = newHeight;

	dbg_w_conf << "ConfigureNotify: Zmeneno okno " << window
				<< " s pozici " << x << ", " << y << " a velikosti "
				<< width << "x" << height << std::endl;
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
        UTILS_SCOPE_EXIT([&]{XFree(fbConfigs);});
		int i;
		for (i = 0; i < nFbConfigs; i++)
		{
			auto visinfo = glXGetVisualFromFBConfig(display, fbConfigs[i]);
            UTILS_SCOPE_EXIT([&]{XFree(visinfo);});
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
		
		// barvy
		GLubyte colorBorder[] = {0,0,0};
		GLubyte colorClose[] = {250,100,100};
		GLubyte colorCloseBorder[] = {0,0,0};
		GLubyte colorDecoration[] = {150,150,150};
		GLubyte colorBackground[] = {255,255,255};
		
		// okraje dekorace
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(colorBorder);
		glVertex2i(x-borderLeft-1, y-borderTop-1);
		glVertex2i(x-borderLeft-1, y+height+borderBottom+1);
		glVertex2i(x+width+borderRight+2, y+height+borderBottom+1);
		glVertex2i(x+width+borderRight+2, y-borderTop-1);
		glEnd();
		
		// pozadi dekorace
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(colorDecoration);
		glVertex2i(x-borderLeft, y-borderTop);
		glVertex2i(x-borderLeft, y+height+borderBottom);
		glVertex2i(x+width+borderRight+1, y+height+borderBottom);
		glVertex2i(x+width+borderRight+1, y-borderTop);
		glEnd();
		
		// okraje zaviraciho tlacitka
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(colorCloseBorder);
		glVertex2i(x+width-closeWidth-1, y-borderTop-1);
		glVertex2i(x+width+1, y-borderTop-1);
		glVertex2i(x+width+1, y-borderTop+closeHeight+1);
		glVertex2i(x+width-closeWidth-1, y-borderTop+closeHeight+1);
		glEnd();
		
		// zaviraci tlacitko
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(colorClose);
		glVertex2i(x+width-closeWidth, y-borderTop);
		glVertex2i(x+width, y-borderTop);
		glVertex2i(x+width, y-borderTop+closeHeight);
		glVertex2i(x+width-closeWidth, y-borderTop+closeHeight);
		glEnd();
		
		// obsah okna
		glBindTexture(GL_TEXTURE_2D, glTexture);
		glXBindTexImageEXT(display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr); // FIXME: Způsobuje leaky!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glBegin(GL_QUADS); // FIXME: Způsobuje leaky!
		glColor3ubv(colorBackground);
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
	destroyPixmap();
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

bool GawmWindow::closePoint(int pX, int pY)
{	
	return (x+width-closeWidth <= pX && pX <= x+width) &&
	       (y-borderTop <= pY && pY < y-borderTop+closeHeight);
}

void GawmWindow::destroyPixmap()
{
    if (hasPixmap)
    {
        glXDestroyPixmap(display, glxPixmap);
        glDeleteTextures(1, &glTexture);
        hasPixmap = false;
    }
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
