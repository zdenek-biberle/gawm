#ifndef WINMGR_HPP
#define WINMGR_HPP

#include <boost/ptr_container/ptr_map.hpp>
#include <X11/extensions/shape.h>
#include "gawmGl.hpp"
#include "window.hpp"

int xerrorhandler(Display *dsp, XErrorEvent *error);

class GawmWindowManager
{
public:
	
	Display *display;
	int screen;
	Window rootWindow;
	Window overlayWindow;
	Window window;
	GLXFBConfig fbConfig;
	XSetWindowAttributes windowAttribs;
	XWindowAttributes overlayWindowAttribs;
	GLXContext ctx;

	typedef boost::ptr_map<Window, GawmWindow> TKnownWindowsMap;
	TKnownWindowsMap knownWindows;
	
	GawmWindowManager();
	~GawmWindowManager();

	void render();

	bool knowWindow(Window window);
	
private:
	
	void initFbConfig();
	
	void initWindow();
	
	void destroyWindow();
	
	void initGL();
	
	void destroyGL();
	
	void allowInputPassthrough();

	void initKnownWindows();
};

#endif
