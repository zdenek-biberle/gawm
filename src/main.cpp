#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
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
	XGrabButton(wm.display, Button1, AnyModifier, wm.window, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
	XSelectInput(wm.display, wm.window, ButtonPressMask + KeyPressMask);
	
	int damage_event, damage_error; // The event base is important here
	XDamageQueryExtension(wm.display, &damage_event, &damage_error);
	
	bool run = true;
	
	while (run)
	{
		wm.render();
		
		XEvent event;
		XPeekEvent(wm.display, &event);
		while(XPending(wm.display))
		{
			XNextEvent(wm.display, &event);
			if (event.type == CreateNotify)
			{
				XCreateWindowEvent& cwe = event.xcreatewindow;
				
				wm.insertWindow(cwe.window, cwe.x, cwe.y, cwe.width, cwe.height);
				
				//XLowerWindow(wm.display, wm.overlayWindow); // experiment
			}
			else if (event.type == DestroyNotify)
			{
				XDestroyWindowEvent& dwe = event.xdestroywindow;
				std::cout << "Erase ..." << std::endl;
				wm.knownWindows.erase(dwe.window);
				std::cout << "Erase done!" << std::endl;
			}
			else if (event.type == ClientMessage)
			{
				XClientMessageEvent& cme = event.xclient;
				std::cout << "ClientMessage s formatem " << cme.format << "; nevim, co s tim..." << std::endl;
			}
			else if (event.type == ConfigureNotify)
			{
				XConfigureEvent& xce = event.xconfigure;
				if (wm.knownWindows.find(xce.window) == wm.knownWindows.end())
				{
					std::cout << "OH SHIT: o okne " << xce.window << " nic nevime, WTF?" << std::endl;
				}
				else
				{
					wm.configureWindow(xce.window, xce.x, xce.y, xce.width, xce.height);
				}
			}
			else if (event.type == MapNotify)
			{
				wm.setVisibilityOfWindow(event.xmap.window, true);
			}
			else if (event.type == UnmapNotify)
			{
				wm.setVisibilityOfWindow(event.xunmap.window, false);
			}
			else if (event.type == KeyPress || event.type == KeyRelease)
			{
				if (event.xkey.keycode == Escape && event.xkey.state & Mod4Mask)
				{
					std::cout << "Stisknuto Win+Esc = Escape from window manager" << std::endl;
					run = false;
				}
			}
			else if (event.type == ButtonPress)
			{
				if (event.xbutton.button == 1)
				{ // kliknuto levym tlacitkem mysi
					std::cout << "Stisknuto leve mysitko";
					
					GawmWindow *w = wm.getHighestWindowAtLocation(event.xbutton.x_root, event.xbutton.y_root);
					
					std::cout << " na " << event.xbutton.x_root << "x" << event.xbutton.y_root << ", kde je ";
					if(w == NULL){
						std::cout << "plocha" << std::endl;
					}else{
						std::cout << "okno " << w->window << std::endl;
					}
					
					if(w != NULL) wm.raiseWindow(w->window);
				}
			}
			else if (event.type == ReparentNotify)
			{
				XReparentEvent& xre = event.xreparent;
				std::cout << "Reparent okna " << xre.window << " k rodici " << xre.parent << " na " << xre.x << ", " << xre.y << std::endl;
			}
			else if (event.type == damage_event + XDamageNotify)
			{
				auto& dne = *reinterpret_cast<XDamageNotifyEvent*>(&event);
				XDamageSubtract(wm.display, dne.damage, None, None);
				wm.knownWindows.at(dne.drawable).doDamage();
			}
			else
			{
				std::cout << "Dosel mi typ " << event.type << "; nevim, co s tim..." << std::endl;
			}
		}
	}
}
