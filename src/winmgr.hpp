#ifndef WINMGR_HPP
#define WINMGR_HPP

#include <list>
#include <boost/ptr_container/ptr_map.hpp>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdamage.h>
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
	
	typedef std::list<GawmWindow*> TSortedWindows;
	TSortedWindows sortedWindows;
	
	GawmWindowManager();
	~GawmWindowManager();
	
	void render();
	
	bool isKnownWindow(Window window);
	
	GawmWindow *getHighestWindowAtLocation(int lX, int lY);
	
	void insertWindow(Window window, int x, int y, int width, int height);
	
	void eraseWindow(Window window);
	
	void configureWindow(Window window, int newX, int newY, int newWidth, int newHeight);
	
	void setVisibilityOfWindow(Window window, bool visible);
	
	void raiseWindow(Window window);
	
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
