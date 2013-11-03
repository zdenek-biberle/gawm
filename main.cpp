#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>

#include "utils.hpp"

int main()
{
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
		0, visualInfo->depth, InputOutput, visualInfo->visual, CWBorderPixel | CWColormap, &windowAttribs);
	if (!window)
	{
		throw std::runtime_error("Nelze vytvorit okno. Bug?");
	}
	UTILS_SCOPE_EXIT([&]{XDestroyWindow(display, window);});
	
	XFree(visualInfo);
	
	XStoreName(display, window, "OH GOD GAWM");
	XMapWindow(display, window);
	
	// Vytvorime OGL kontext
	auto ctx = glXCreateNewContext(display, fbConfig, GLX_RGBA_TYPE, nullptr, True);
	XSync(display, False);
	if (!ctx)
	{
		throw std::runtime_error("Nepodarilo se vytvorit OpenGL kontext");
	}
	UTILS_SCOPE_EXIT([&]{glXDestroyContext(display, ctx);});
	
	glXMakeCurrent(display, window, ctx);
	
	while (true)
	{
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glXSwapBuffers(display, window);
		sleep(1);
	}
}
