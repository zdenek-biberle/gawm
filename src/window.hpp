#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <GL/gl.h>

const GLubyte* selectRandomColor();

struct GawmWindow
{
private:
	GawmWindowManager* wm;
	Window window;
	int x;
	int y;
	int width;
	int height;
	const GLubyte* color;
	Pixmap pixmap;
	GLXPixmap glxPixmap;
	GLuint glTexture;
	bool hasPixmap;
	bool visible;
	bool invertedY;

public:
	GawmWindow(GawmWindowManager *wm, Window window, int x, int y, int width, int height):
		wm(wm),
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
		
		XCompositeRedirectWindow(wm->display, window, CompositeRedirectManual);
	}
	
	~GawmWindow()
	{
		std::cout << "DestroyNotify: Zniceno okno " << window << std::endl;
	}
	
	void configure(int newX, int newY, int newWidth, int newHeight){
		x = newX;
		y = newY;
		width = newWidth;
		height = newHeight;
		
		std::cout << "ConfigureNotify: Zmeneno okno " << window
					<< " s pozici " << x << ", " << y << " a velikosti "
					<< width << "x" << height << std::endl;
		
		hasPixmap = false;
	}
	
	void reloadPixmap(){
		if (!hasPixmap && isVisible())
		{
			std::cout << "reloadPixmap(" << wm->display << "," << window << ")" << std::endl;
			XWindowAttributes attribs;
			XGetWindowAttributes(wm->display, window, &attribs);

			int nFbConfigs;
			auto visualid = XVisualIDFromVisual(attribs.visual);
			auto fbConfigs = glXGetFBConfigs(wm->display, wm->screen, &nFbConfigs);
			int i;
			for (i = 0; i < nFbConfigs; i++)
			{
				auto visinfo = glXGetVisualFromFBConfig(wm->display, fbConfigs[i]);
				if (!visinfo || visinfo->visualid != visualid)
					continue;

				int value;
				glXGetFBConfigAttrib(wm->display, fbConfigs[i], GLX_DRAWABLE_TYPE, &value);
				if (!(value & GLX_PIXMAP_BIT))
					continue;

				glXGetFBConfigAttrib(wm->display, fbConfigs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value);
				if (!(value & GLX_TEXTURE_2D_BIT_EXT))
					continue;

				glXGetFBConfigAttrib(wm->display, fbConfigs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
				if (value == False)
				{
					glXGetFBConfigAttrib(wm->display, fbConfigs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
					if (value == False)
						continue;
				}

				glXGetFBConfigAttrib(wm->display, fbConfigs[i], GLX_Y_INVERTED_EXT, &value);
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

			pixmap = XCompositeNameWindowPixmap(wm->display, window);
			std::cout << "pixmap: " << pixmap << std::endl;
			glxPixmap = glXCreatePixmap(wm->display, fbConfigs[i], pixmap, pixmapAttribs);
			std::cout << "glxPixmap: " << glxPixmap << std::endl;

			glGenTextures (1, &glTexture);
			
			XSync(wm->display, False);
			hasPixmap = true;
			std::cout << "reload pixmapy uspesny" << std::endl;
		}
	}
	
	void render(){
		reloadPixmap();
		
		if(isVisible())
		{
			glBindTexture(GL_TEXTURE_2D, glTexture);

			glXBindTexImageEXT(wm->display, glxPixmap, GLX_FRONT_LEFT_EXT, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
			//glXReleaseTexImageEXT (wm->display, glxPixmap, GLX_FRONT_LEFT_EXT);
			
		}
	}
	
	bool isVisible()
	{
		return visible;
	}
	
	void setVisible(bool visible)
	{
		this->visible = visible;
		reloadPixmap();
	}
};

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

#endif
