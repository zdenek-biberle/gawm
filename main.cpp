#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <map>
#include <cstdbool>

#include "utils.hpp"

struct GawmWindow
{
	GawmWindow():
		x(-1),
		y(-1),
		width(-1),
		height(-1),
		color(nullptr)
	{}
	
	int x;
	int y;
	int width;
	int height;
	const GLubyte* color;
};

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

const GLubyte* selectRandomColor()
{
	return colors + (rand() % 19)*3;
}

int main()
{
	srand(time(nullptr));
	
	auto display = XOpenDisplay(nullptr);
	UTILS_SCOPE_EXIT([&]{XCloseDisplay(display);});
	auto rootWindow = DefaultRootWindow(display);
	auto overlayWindow = XCompositeGetOverlayWindow(display, rootWindow);
	UTILS_SCOPE_EXIT([&]{XCompositeReleaseOverlayWindow(display, rootWindow);});
	
	int glDisplayAttribs[] = 
	{
		GLX_DRAWABLE_TYPE,	GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,	GLX_RGBA_BIT,
		GLX_RED_SIZE,		8,
		GLX_GREEN_SIZE, 	8,
		GLX_BLUE_SIZE, 		8,
		GLX_ALPHA_SIZE, 	8,
		GLX_DEPTH_SIZE, 	24,
		GLX_STENCIL_SIZE, 	8,
		GLX_DOUBLEBUFFER, 	True,
		None
	};
	
	// Najdeme vhodnou konfiguraci framebufferu
	int configCount;
	auto fbConfigs = glXChooseFBConfig(display, DefaultScreen(display), glDisplayAttribs, &configCount);
	if (!fbConfigs)
	{
		throw std::runtime_error("Nenalezen config framebufferu, nevim co delat");
	}
	
	auto fbConfig = fbConfigs[0]; // proste berem prvni konfiguraci
	XFree(fbConfigs);
	
	// Podle konfigurace vytvorime okno
	auto visualInfo = glXGetVisualFromFBConfig(display, fbConfig);
	XSetWindowAttributes windowAttribs;
	windowAttribs.background_pixmap = None;
	windowAttribs.border_pixel = 0;
	windowAttribs.colormap = XCreateColormap(display, overlayWindow, visualInfo->visual, AllocNone);
	UTILS_SCOPE_EXIT([&]{XFreeColormap(display, windowAttribs.colormap);});
	
	XWindowAttributes overlayWindowAttribs;
	XGetWindowAttributes(display, overlayWindow, &overlayWindowAttribs);
	
	auto window = XCreateWindow(
		display, overlayWindow, 
		0, 0, 
		overlayWindowAttribs.width, overlayWindowAttribs.height, 
		0, visualInfo->depth, InputOutput, visualInfo->visual,
		CWBorderPixel | CWColormap, &windowAttribs);
	if (!window)
	{
		throw std::runtime_error("Nelze vytvorit okno. Bug?");
	}
	UTILS_SCOPE_EXIT([&]{XDestroyWindow(display, window);});
	
	XFree(visualInfo);
	
	XStoreName(display, window, "OH GOD GAWM");
	XMapWindow(display, window);
	XSelectInput(display, rootWindow, SubstructureNotifyMask);
	
	// Vytvorime OGL kontext
	auto ctx = glXCreateNewContext(display, fbConfig, GLX_RGBA_TYPE, nullptr, True);
	XSync(display, False);
	if (!ctx)
	{
		throw std::runtime_error("Nepodarilo se vytvorit OpenGL kontext");
	}
	UTILS_SCOPE_EXIT([&]{glXDestroyContext(display, ctx);});
	glXMakeCurrent(display, window, ctx);
	glTranslated(-1.0, -1.0, 0.0);
	glScaled(1.0 / overlayWindowAttribs.width, 1.0 / overlayWindowAttribs.height, 0.5);
	
	std::map<Window, GawmWindow> knownWindows;
	
	// Odchytavani klaves pro Window manager
	KeyCode Escape = XKeysymToKeycode(display, XStringToKeysym("Escape"));
	XGrabKey(display, Escape, Mod4Mask, window, true, GrabModeSync, GrabModeSync); // Mod4Mask / AnyModifier
	XSelectInput(display, window, KeyPressMask);
	
	while (true)
	{
		glClearColor(0.25, 0.25, 0.25, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Tohle predelat na VBO nebo neco takovyho
		glBegin(GL_QUADS);
		for(auto& knownWindow : knownWindows)
		{
			GawmWindow& gawmWindow = knownWindow.second;
			glColor3ubv(gawmWindow.color);
			glVertex2i(gawmWindow.x, gawmWindow.y);
			glVertex2i(gawmWindow.x, gawmWindow.y + gawmWindow.height);
			glVertex2i(gawmWindow.x + gawmWindow.width, gawmWindow.y + gawmWindow.height);
			glVertex2i(gawmWindow.x + gawmWindow.width, gawmWindow.y);
		}
		glEnd();
		
		glXSwapBuffers(display, window);
		//sleep(1);
		
		XEvent event;
		XNextEvent(display, &event);
		switch (event.type)
		{
			case CreateNotify:
			{
				XCreateWindowEvent& cwe = event.xcreatewindow;
				GawmWindow& gawmWindow = knownWindows[cwe.window];
				gawmWindow.x = cwe.x;
				gawmWindow.y = cwe.y;
				gawmWindow.width = cwe.width;
				gawmWindow.height = cwe.height;
				gawmWindow.color = selectRandomColor();
				std::cout << "CreateNotify: Vytvoreno okno " << cwe.window << " na " 
					<< cwe.x << ", " << cwe.y
					<< " velikosti " << cwe.width << "x" << cwe.height
					<< std::endl;
			}
			break;
			
			case DestroyNotify:
			{
				XDestroyWindowEvent& dwe = event.xdestroywindow;
				knownWindows.erase(dwe.window);
				std::cout << "DestroyNotify: Zniceno okno " << dwe.window << std::endl;
			}
			break;
			
			case ClientMessage:
			{
				XClientMessageEvent& cme = event.xclient;
				std::cout << "ClientMessage s formatem " << cme.format << "; nevim, co s tim..." << std::endl;
				
			}
			break;
			
			case ConfigureNotify:
			{
				XConfigureEvent& xce = event.xconfigure;
				GawmWindow& gawmWindow = knownWindows[xce.window];
				gawmWindow.x = xce.x;
				gawmWindow.y = xce.y;
				gawmWindow.width = xce.width;
				gawmWindow.height = xce.height;
				if (gawmWindow.color == nullptr)
				{
					gawmWindow.color = selectRandomColor();
				}
				std::cout << "ConfigureNotify: Zmeneno okno " << xce.window
					<< " s pozici " << xce.x << ", " << xce.y << " a velikosti "
					<< xce.width << "x" << xce.height << std::endl;
			}
			break;
			
			case KeyPress: case KeyRelease:
			{
				if(event.xkey.keycode == Escape && event.xkey.state & Mod4Mask){
					std::cout << "Stisknuto Win+Esc = Escape from window manager" << std::endl;
					exit(0);
				}
			}
			break;
			
			default:
				std::cout << "Dosel mi typ " << event.type << "; nevim, co s tim..." << std::endl;
				break;
		}
	}
}
