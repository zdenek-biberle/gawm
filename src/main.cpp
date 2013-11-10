#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

#include "utils.hpp"
#include "winmgr.hpp"
#include "window.hpp"

int main()
{
	srand(time(nullptr));
	
	GawmWindowManager wm;
	
	// Odchytavani klaves pro Window manager
	KeyCode Escape = XKeysymToKeycode(wm.display, XStringToKeysym("Escape"));
	XGrabKey(wm.display, Escape, Mod4Mask, wm.window, True, GrabModeSync, GrabModeSync); // Mod4Mask / AnyModifier
	XSelectInput(wm.display, wm.window, KeyPressMask);
	
	while (true)
	{
		
		wm.render();
		
		XEvent event;
		XNextEvent(wm.display, &event);
// 		while(XPending(wm.display))
		{
			switch (event.type)
			{
				case CreateNotify:
				{
					XCreateWindowEvent& cwe = event.xcreatewindow;
					wm.knownWindows.insert(cwe.window, new GawmWindow(wm.display, wm.screen, cwe.window, cwe.x, cwe.y, cwe.width, cwe.height));
				}
				break;
				
				case DestroyNotify:
				{
					XDestroyWindowEvent& dwe = event.xdestroywindow;
					if ( wm.knowWindow(dwe.window) ) {
						std::cout << "Erase ..." << std::endl;
						wm.knownWindows.erase(dwe.window);
					}
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
					if ( wm.knowWindow(xce.window) ) {
						wm.knownWindows.at(xce.window).configure(xce.x,xce.y,xce.width,xce.height);
					}
				}
				break;
				
				case MapNotify:
				{
					if ( wm.knowWindow(event.xmap.window) ) {
						wm.knownWindows.at(event.xmap.window).setVisible(true);
					}
				}
				break;
				
				case UnmapNotify:
				{
					if ( wm.knowWindow(event.xunmap.window) ) {
						wm.knownWindows.at(event.xunmap.window).setVisible(false);
					}
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
