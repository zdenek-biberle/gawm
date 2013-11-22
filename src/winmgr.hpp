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
	int maxX;
	int maxY;
	
	typedef boost::ptr_map<Window, GawmWindow> TKnownWindowsMap;
	TKnownWindowsMap knownWindows;
	
	typedef std::list<GawmWindow*> TSortedWindows;
	TSortedWindows sortedWindows;
	
	double zoom = 1.0;
	static constexpr double zoom_const = 1.03;
	
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
	
	inline void zoomIn(int x, int y){
		
		if(zoom > 20.0) return; // maximalni priblizeni
		
		zoom *= zoom_const;
		
		// celá plocha se posune o rozdíl staré a nové pozice kurzoru myši
		moveDesktop( x - x * zoom_const, y - y * zoom_const);
		
		dbg_e_buttonPress << "zoom = " << zoom << std::endl;
		
	}
	
	inline void zoomOut(int x, int y){
		
		if(zoom < 0.03) return; // maximalni oddaleni
		
		zoom /= zoom_const;
		
		// celá plocha se posune o rozdíl staré a nové pozice kurzoru myši
		moveDesktop( x - x / zoom_const, y - y / zoom_const);
		
		dbg_e_buttonPress << "zoom = " << zoom << std::endl;
		
	}
	
	inline int reverseConvertX(int x){
		return x/zoom;
	}
	
	inline int reverseConvertY(int y){
		return y/zoom;
	}
	
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
