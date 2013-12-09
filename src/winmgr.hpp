#ifndef WINMGR_HPP
#define WINMGR_HPP

#include <list>
#include <boost/ptr_container/ptr_map.hpp>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdamage.h>
#include "debug.hpp"
#include "gawmGl.hpp"
#include "window.hpp"

int xerrorhandler(Display *dsp, XErrorEvent *error);
bool hadError();

class GawmWindowManager
{
public:
	
	Display *display;
	int screen;
	Window rootWindow;
	Window overlayWindow;
	Window outputWindow;
	GLXFBConfig fbConfig;
	XSetWindowAttributes windowAttribs;
	XWindowAttributes overlayWindowAttribs;
	GLXContext ctx;
	int maxX;
	int maxY;
	KeyCode escapeKey;
	
	typedef boost::ptr_map<Window, GawmWindow> TKnownWindowsMap;
	TKnownWindowsMap knownWindows;
	
	typedef std::list<GawmWindow*> TSortedWindows;
	TSortedWindows sortedWindows;
	
	int zoomLevel = 3;
	double zoom = 1.0;
	
	GawmWindowManager();
	~GawmWindowManager();
	
	void render();
	
	bool isKnownWindow(Window window);
	
	GawmWindow *getHighestWindow();
	
	GawmWindow *getHighestWindowAtLocation(int lX, int lY);
	
	void insertWindow(Window window, int x, int y, int width, int height);
	
	void eraseWindow(Window window);
	
	void configureWindow(Window window, int newX, int newY, int newWidth, int newHeight);
	
	void setVisibilityOfWindow(Window window, bool visible);
	
	void raiseWindow(Window window);
	
	void moveResizeWindow(GawmWindow *window, int newX, int newY, int newWidth, int newHeight);

	void moveDesktop(int xdiff, int ydiff);
	
	void zoomIn(int x, int y);
	
	void zoomOut(int x, int y);
	
	void zoomTo(int level, int x, int y);
	
	inline int reverseConvertX(int x){
		return x/zoom;
	}
	
	inline int reverseConvertY(int y){
		return y/zoom;
	}
	
private:
	void setupEvents();
	
	void initFbConfig();
	
	void initWindow();
	
	void destroyWindow();
	
	void initGL();
	
	void destroyGL();
	
	void allowInputPassthrough(Window window);
	
	void initKnownWindows();
};

#endif
