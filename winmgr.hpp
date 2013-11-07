#ifndef WINMGR_HPP
#define WINMGR_HPP

class GawmWindowManager
{
public:
	
	Display *display;
	Window rootWindow;
	Window overlayWindow;
	Window window;
	GLXFBConfig fbConfig;
	XSetWindowAttributes windowAttribs;
	XWindowAttributes overlayWindowAttribs;
	GLXContext ctx;
	
	GawmWindowManager()
	{
		display = XOpenDisplay(nullptr);
		rootWindow = DefaultRootWindow(display);
		overlayWindow = XCompositeGetOverlayWindow(display, rootWindow);
		initFbConfig();
		initWindow();
		XSelectInput(display, rootWindow, SubstructureNotifyMask);
		initGL();
	}
	
	void initFbConfig(){
		
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
		auto fbConfigs = glXChooseFBConfig(display, DefaultScreen(display), glDisplayAttribs, &configCount);
		if (!fbConfigs)
		{
			throw std::runtime_error("Nenalezen config framebufferu, nevim co delat");
		}
		fbConfig = fbConfigs[0]; // proste berem prvni konfiguraci
		XFree(fbConfigs);
		
	}
	
	void initWindow(){
		
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
	
	void destroyWindow(){
		XFreeColormap(display, windowAttribs.colormap);
		XDestroyWindow(display, window);
	}
	
	void initGL(){
		
		// Vytvorime OGL kontext
		auto ctx = glXCreateNewContext(display, fbConfig, GLX_RGBA_TYPE, nullptr, True);
		XSync(display, False);
		if (!ctx)
		{
			throw std::runtime_error("Nepodarilo se vytvorit OpenGL kontext");
		}
		glXMakeCurrent(display, window, ctx);
		glTranslated(-1.0, -1.0, 0.0);
		glScaled(1.0 / overlayWindowAttribs.width, 1.0 / overlayWindowAttribs.height, 0.5);
		
	}
	
	void destroyGL(){
		glXDestroyContext(display, ctx);
	}
	
	~GawmWindowManager()
	{
		destroyGL();
		destroyWindow();
		XCompositeReleaseOverlayWindow(display, rootWindow);
		XCloseDisplay(display);
	}
	
};

#endif
