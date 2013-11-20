#include <stdexcept>
#include <iostream>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xcomposite.h>

#include "debug.hpp"
#include "winmgr.hpp"

int xerrorhandler(Display *dsp, XErrorEvent *error)
{
	char errorstring[128];
	XGetErrorText(dsp, error->error_code, errorstring, 128);
 
	cerr_line << "Xka umrely: " << errorstring << std::endl;
	throw std::runtime_error("Xka umrely");
}

GawmWindowManager::GawmWindowManager()
{
	display = XOpenDisplay(nullptr);
	if (!display)
	{
		throw std::runtime_error("Nepodarilo se otevrit display!");
	}

	XSynchronize(display, True); // Synchronizace s xserverem pro debugování
	XSetErrorHandler(xerrorhandler);

	screen = DefaultScreen(display);
	rootWindow = DefaultRootWindow(display);
	overlayWindow = XCompositeGetOverlayWindow(display, rootWindow);
	
	XCompositeRedirectSubwindows(display, rootWindow, CompositeRedirectManual);
	initFbConfig();
	initWindow();
	XSelectInput(display, rootWindow, SubstructureNotifyMask);
	initKnownWindows();
	initGL();
	allowInputPassthrough();
	
	dbg_out << "GawmWindowManager: Screen: " << screen << ", rootWindow: " << rootWindow << ", overlayWindow: " << overlayWindow
			<< ", GL window: " << window << std::endl;
}

GawmWindowManager::~GawmWindowManager()
{
	destroyGL();
	destroyWindow();
	XCompositeReleaseOverlayWindow(display, rootWindow);
	XCloseDisplay(display);
}

void GawmWindowManager::render()
{

	// pozadi plochy
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);

	for (auto it=sortedWindows.rbegin(); it!=sortedWindows.rend(); ++it)
	{
		(*it)->render();
	}

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		cerr_line << "GL error: " << err << std::endl;
	}

	glXSwapBuffers(display, window);

}

void GawmWindowManager::initFbConfig()
{

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

	int configCount;

	auto fbConfigs = glXChooseFBConfig(display, screen, glDisplayAttribs, &configCount);
	if (!fbConfigs)
	{
		throw std::runtime_error("Nenalezena zadna konfigurace framebufferu odpovidajici atributum glDisplayAttribs! (Nepodporovana graficka karta?)");
	}
	fbConfig = fbConfigs[0]; // proste berem prvni konfiguraci
	XFree(fbConfigs);

}

void GawmWindowManager::initWindow()
{
	// Podle konfigurace vytvorime okno
	auto visualInfo = glXGetVisualFromFBConfig(display, fbConfig);
	XSetWindowAttributes windowAttribs;
	windowAttribs.background_pixmap = None;
	windowAttribs.border_pixel = 0;
	windowAttribs.colormap = XCreateColormap(display, overlayWindow, visualInfo->visual, AllocNone);

	XGetWindowAttributes(display, overlayWindow, &overlayWindowAttribs);

	window = XCreateWindow(
		display, overlayWindow,
		0, 0,
		overlayWindowAttribs.width, overlayWindowAttribs.height,
		0, visualInfo->depth, InputOutput, visualInfo->visual,
		CWBorderPixel | CWColormap, &windowAttribs);
	if (!window)
	{
		throw std::runtime_error("Nelze vytvorit okno. Bug?");
	}

	XFree(visualInfo);

	XStoreName(display, window, "OH GOD GAWM");
	XMapWindow(display, window);

}

void GawmWindowManager::destroyWindow()
{
	XFreeColormap(display, windowAttribs.colormap);
	XDestroyWindow(display, window);
}

void GawmWindowManager::initGL()
{
	// Vytvoreni OpenGL kontextu
	auto ctx = glXCreateNewContext(display, fbConfig, GLX_RGBA_TYPE, nullptr, True);
	XSync(display, False);
	if (!ctx)
	{
		throw std::runtime_error("Nepodarilo se vytvorit OpenGL kontext!");
	}
	glXMakeCurrent(display, window, ctx);
	initGlFunctions();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(-1.0, 1.0, 0.0);
	glScaled( 2.0 / overlayWindowAttribs.width, -2.0 / overlayWindowAttribs.height, 1.0);
}

void GawmWindowManager::destroyGL()
{
	glXDestroyContext(display, ctx);
}

void GawmWindowManager::allowInputPassthrough()
{
	XserverRegion region = XFixesCreateRegion(display, NULL, 0);
	XFixesSetWindowShapeRegion(display, window, ShapeBounding, 0, 0, 0);
	//XFixesSetWindowShapeRegion(display, window, ShapeInput, 0, 0, region); // FIXME: Pokud je toto odkomentováno, přestanou se odchytávat klávesy.
	XFixesDestroyRegion(display, region);
	
	// experiment2
	//XGrabPointer(display, overlayWindow, True /*owner_events - proverit*/, 0/*event_mask*/, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	//XGrabButton(display, AnyButton, AnyModifier, overlayWindow, True /*owner_events - proverit*/, 0/*event_mask*/, GrabModeAsync, GrabModeAsync, None, None);
	
}

bool GawmWindowManager::isKnownWindow(Window window)
{
	TKnownWindowsMap::iterator it = knownWindows.find(window);
	if ( it != knownWindows.end() ) {
		return true;
	}
	else {
		return false;
	}
}

GawmWindow* GawmWindowManager::getHighestWindow()
{
	if(!sortedWindows.empty()){
		return sortedWindows.front();
	}else{
		return NULL;
	}
}

GawmWindow* GawmWindowManager::getHighestWindowAtLocation(int lX, int lY)
{
	for (auto sortedWindow : sortedWindows)
	{
		if(sortedWindow->containsPoint(lX, lY))
		{
			return sortedWindow;
		}
	}
	return NULL;
}

void GawmWindowManager::insertWindow(Window window, int x, int y, int width, int height)
{
	GawmWindow *w = new GawmWindow(display, screen, window, x, y, width, height);
	knownWindows.insert(window, w);
	sortedWindows.push_front(w);
}

void GawmWindowManager::eraseWindow(Window window)
{
	GawmWindow *w = &knownWindows.at(window);
	knownWindows.erase(window);
	sortedWindows.remove(w);
}

void GawmWindowManager::configureWindow(Window window, int newX, int newY, int newWidth, int newHeight)
{
	knownWindows.at(window).configure(newX, newY, newWidth, newHeight);
}

void GawmWindowManager::setVisibilityOfWindow(Window window, bool visible)
{
	knownWindows.at(window).setVisible(visible);
}

void GawmWindowManager::raiseWindow(Window window)
{
	XRaiseWindow(display, window);
	GawmWindow *gw = &knownWindows.at(window);
	sortedWindows.remove(gw);
	sortedWindows.push_front(gw);
}

void GawmWindowManager::moveResizeWindow(GawmWindow *window, int newX, int newY, int newWidth, int newHeight)
{
	XMoveResizeWindow(display, window->window, newX, newY, newWidth, newHeight);
}

void GawmWindowManager::initKnownWindows()
{
	Window root;
	Window parent;
	Window *children;
	Status status;
	unsigned nNumChildren;

	status = XQueryTree(display, rootWindow, &root, &parent, &children, &nNumChildren);
	if (status == 0)
	{
		// Nemohu získat strom oken, přerušuji.
		return;
	}

	if (nNumChildren == 0)
	{
		// Kořeň nemá žádné děcka.
		return;
	}

	for (unsigned i = 0; i < nNumChildren; i++)
	{
		if (children[i] == overlayWindow)
		{
			cerr_line << "Jedno z deti roota je overlay, to je asi spatne" << std::endl;
		}
		
		XWindowAttributes w_attr;

		status = XGetWindowAttributes(display, children[i], &w_attr);
		if (status == 0)
		{
			// Nemohu získat geometrii okna, pokračuji dalším.
			cerr_line << "Okno je " << children[i] << " a nemá geometrii" << std::endl;
			continue;
		}

		// Přidáme potomka Xek do mapy známých oken...
		knownWindows.insert(children[i], new GawmWindow(display, screen, children[i],
														w_attr.x, w_attr.y,
														w_attr.width+2*w_attr.border_width, w_attr.height+2*w_attr.border_width));
		// ... a zviditeníme ho, pokud je IsViewable
		if (w_attr.map_state == IsViewable)
		{
			knownWindows.at(children[i]).setVisible(true);
		}
	}

	XFree(children);
}
