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
		XNextEvent(wm.display, &event);
// 		while(XPending(wm.display))
		{
			if (event.type == CreateNotify)
			{
				XCreateWindowEvent& cwe = event.xcreatewindow;
				wm.knownWindows.insert(cwe.window, new GawmWindow(wm.display, wm.screen, cwe.window, cwe.x, cwe.y, cwe.width, cwe.height));

				XLowerWindow(wm.display, wm.overlayWindow); // experiment
			}
			else if (event.type == DestroyNotify)
			{
				XDestroyWindowEvent& dwe = event.xdestroywindow;
				if ( wm.knowWindow(dwe.window) ) {
					std::cout << "Erase ..." << std::endl;
					wm.knownWindows.erase(dwe.window);
				}
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
				if (wm.knowWindow(xce.window))
				{
					wm.knownWindows.at(xce.window).configure(xce.x,xce.y,xce.width,xce.height);
				}
			}
			else if (event.type == MapNotify)
			{
				if (wm.knowWindow(event.xmap.window))
				{
					wm.knownWindows.at(event.xmap.window).setVisible(true);
				}
			}
			else if (event.type == UnmapNotify)
			{
				if ( wm.knowWindow(event.xunmap.window) )
				{
					wm.knownWindows.at(event.xunmap.window).setVisible(false);
				}
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
					std::cout << "nope.avi" << std::endl;
					//std::cout << "Vyzdvihuji okno " << event.xbutton.subwindow << std::endl;
					//XRaiseWindow(wm.display, event.xbutton.window); // vyzdvihnout okno na ktere se kliklo
					//XLowerWindow(wm.display, wm.overlayWindow);
				}
			}
			else if (event.type == damage_event + XDamageNotify)
			{
				auto& dne = *reinterpret_cast<XDamageNotifyEvent*>(&event);
				std::cout << "XDamageNotifyEvent okna " << dne.drawable << std::endl;
				XDamageSubtract(wm.display, dne.damage, None, None);
			}
			else
			{
				std::cout << "Dosel mi typ " << event.type << "; nevim, co s tim..." << std::endl;
			}
		}
	}
}
