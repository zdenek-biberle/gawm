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

#include "debug.hpp"
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
	const unsigned int event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask;
	XGrabPointer(wm.display, wm.overlayWindow, True, event_mask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	XSelectInput(wm.display, wm.window, ButtonPressMask + KeyPressMask);
	
	int damage_event, damage_error; // The event base is important here
	XDamageQueryExtension(wm.display, &damage_event, &damage_error);
	
	bool run = true;
	
	GawmWindow *draggedWindow = NULL;
	int dragStartX, dragStartY;
	
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
				dbg_w_destroy << "Erase ..." << std::endl;
				wm.knownWindows.erase(dwe.window);
				dbg_w_destroy << "Erase done!" << std::endl;
			}
			else if (event.type == ClientMessage)
			{
				XClientMessageEvent& cme = event.xclient;
				cerr_line << "ClientMessage s formatem " << cme.format << "; nevim, co s tim..." << std::endl;
			}
			else if (event.type == ConfigureNotify)
			{
				XConfigureEvent& xce = event.xconfigure;
				if (wm.knownWindows.find(xce.window) == wm.knownWindows.end())
				{
					cerr_line << "OH SHIT: o okne " << xce.window << " nic nevime, WTF?" << std::endl;
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
					dbg_out << "Stisknuto Win+Esc = Escape from window manager" << std::endl;
					run = false;
				}
				else
				{
					// klavesy ktere nezajimaji WM jsou zaslany aktivnimu (nejvyssimu) oknu
					GawmWindow *w = wm.getHighestWindow();
					
					dbg_e_keyPress << "Klavesa ";
					if(w == NULL){
						dbg_e_keyPress << "plocha" << std::endl;
					}else{
						dbg_e_keyPress << "okno " << w->window << std::endl;
					}
					
					if(w != NULL){
						XSendEvent(wm.display, w->window, False, 0, &event);
					}
				}
			}
			else if (event.type == ButtonPress || event.type == ButtonRelease)
			{
				GawmWindow *w = wm.getHighestWindowAtLocation(event.xbutton.x_root, event.xbutton.y_root);
				
				dbg_e_buttonPress << "Stisknuto/uvolneno mysitko na " << event.xbutton.x_root << "x" << event.xbutton.y_root << ", kde je ";
				if(w == NULL){
					dbg_e_buttonPress << "plocha" << std::endl;
				}else{
					dbg_e_buttonPress << "okno " << w->window << std::endl;
				}
				
				if(w != NULL){
					wm.raiseWindow(w->window);
					XSendEvent(wm.display, w->window, False, 0, &event);
					
					if(event.type == ButtonPress && w->handlePoint(event.xbutton.x_root, event.xbutton.y_root)){
						dbg_e_buttonPress << "zahajeno pretahovani okna " << w->window << std::endl;
						draggedWindow = w;
						dragStartX = event.xbutton.x_root;
						dragStartY = event.xbutton.y_root;
					}
				}
				
				if(event.type == ButtonRelease){
					draggedWindow = NULL;
				}
			}
			else if (event.type == MotionNotify)
			{
				// presun okna
				if(draggedWindow != NULL){
					XWindowAttributes attr;
					XGetWindowAttributes(wm.display, draggedWindow->window, &attr);
					signed int xdiff = event.xmotion.x_root - dragStartX;
					signed int ydiff = event.xmotion.y_root - dragStartY;
					
					dbg_e_motion << "presun okna " << draggedWindow->window << " z " << dragStartX << "," << dragStartY;
					dbg_e_motion << " na " << draggedWindow->x+xdiff << "," << draggedWindow->y+ydiff << std::endl;
					wm.moveResizeWindow(draggedWindow, attr.x+xdiff, attr.y+ydiff, attr.width, attr.height);
					
					dragStartX = event.xmotion.x_root;
					dragStartY = event.xmotion.y_root;
				}
			}
			else if (event.type == ReparentNotify)
			{
				XReparentEvent& xre = event.xreparent;
				dbg_e_reparent << "Reparent okna " << xre.window << " k rodici " << xre.parent << " na " << xre.x << ", " << xre.y << std::endl;
			}
			else if (event.type == damage_event + XDamageNotify)
			{
				auto& dne = *reinterpret_cast<XDamageNotifyEvent*>(&event);
				XDamageSubtract(wm.display, dne.damage, None, None);
				wm.knownWindows.at(dne.drawable).doDamage();
			}
			else
			{
				cerr_line << "Dosel mi typ " << event.type << "; nevim, co s tim..." << std::endl;
			}
		}
	}

	return 0;
}
