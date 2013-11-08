#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <boost/ptr_container/ptr_map.hpp>

#include "utils.hpp"
#include "winmgr.hpp"
#include "window.hpp"

void render(GawmWindowManager* wm, boost::ptr_map<long unsigned int, GawmWindow>* knownWindows){
	
	// pozadi plochy
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	
	for(auto knownWindow : *knownWindows)
	{
		knownWindow.second->render();
	}
	
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL error: " << err << std::endl;
	}
	
	glXSwapBuffers(wm->display, wm->window);
	
}

int main()
{
	srand(time(nullptr));
	
	GawmWindowManager wm;
	
	boost::ptr_map<Window, GawmWindow> knownWindows;
	
	// Odchytavani klaves pro Window manager
	KeyCode Escape = XKeysymToKeycode(wm.display, XStringToKeysym("Escape"));
	XGrabKey(wm.display, Escape, Mod4Mask, wm.window, True, GrabModeSync, GrabModeSync); // Mod4Mask / AnyModifier
	XSelectInput(wm.display, wm.window, KeyPressMask);
	
	while (true)
	{
		
		render(&wm, &knownWindows);
		
		XEvent event;
		while(XCheckWindowEvent(wm.display, wm.rootWindow, SubstructureNotifyMask, &event) == True)
		{
			switch (event.type)
			{
				case CreateNotify:
				{
					XCreateWindowEvent& cwe = event.xcreatewindow;
					knownWindows.insert(cwe.window, new GawmWindow(&wm, cwe.window, cwe.x, cwe.y, cwe.width, cwe.height));
				}
				break;
				
				case DestroyNotify:
				{
					
					XDestroyWindowEvent& dwe = event.xdestroywindow;
					knownWindows.erase(dwe.window);
					std::cout << "Erase done!" << std::endl;
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
					knownWindows.at(xce.window).configure(xce.x,xce.y,xce.width,xce.height);
				}
				break;
				
				case MapNotify:
				{
					knownWindows.at(event.xmap.window).setVisible(true);
				}
				break;
				
				case UnmapNotify:
				{
					knownWindows.at(event.xunmap.window).setVisible(false);
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
}
