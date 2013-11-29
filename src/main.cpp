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
	XGrabKey(wm.display, Escape, AnyModifier, wm.rootWindow, True, GrabModeAsync, GrabModeAsync); 
	XGrabButton(wm.display, Button1, Mod4Mask, wm.rootWindow, True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(wm.display, Button4, Mod4Mask, wm.rootWindow, True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(wm.display, Button5, Mod4Mask, wm.rootWindow, True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
	XSelectInput(wm.display, wm.rootWindow, SubstructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
	
	int damage_event, damage_error; // The event base is important here
	XDamageQueryExtension(wm.display, &damage_event, &damage_error);
	
	bool run = true;
	
	GawmWindow *draggedWindow = NULL;
	int dragStartX, dragStartY;
	
	while (run)
	{
		XEvent event;
		XPeekEvent(wm.display, &event);
		XGrabServer(wm.display);
		
		while(XPending(wm.display))
		{
			XNextEvent(wm.display, &event);
			if (event.type == CreateNotify)
			{
				XCreateWindowEvent& cwe = event.xcreatewindow;
				wm.insertWindow(cwe.window, cwe.x, cwe.y, cwe.width, cwe.height);
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
			}
			else if (event.type == ButtonPress)
			{
				int x = wm.reverseConvertX(event.xbutton.x_root);
				int y = wm.reverseConvertY(event.xbutton.y_root);
				
				GawmWindow *w = wm.getHighestWindowAtLocation(x, y);
				
				dbg_e_buttonPress << "Stisknuto/uvolneno mysitko na " << x << "x" << y << ", kde je ";
				if(w == NULL){
					dbg_e_buttonPress << "plocha" << std::endl;
				}else{
					dbg_e_buttonPress << "okno " << w->window << std::endl;
				}
				
				if (event.xbutton.state & Mod4Mask)
				{ 
					// neni-li nad oknem, nebo je stisknuto Win, prace nad plochou
					if(event.type == ButtonPress && event.xbutton.button == Button1) // zahajeni posunu plochy
					{
						dbg_e_buttonPress << "posun plochy" << std::endl;
						draggedWindow = (GawmWindow*) &wm; // pretahovanym oknem je window manager
						dragStartX = x;
						dragStartY = y;
						const unsigned int eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | EnterWindowMask;
						XGrabPointer(wm.display, wm.rootWindow, True, eventMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
					}
					if(event.type == ButtonPress && event.xbutton.button == Button4) // scroll nahoru
					{
						dbg_e_buttonPress << "prizoomovani" << std::endl;
						wm.zoomIn(x, y);
					}
					if(event.type == ButtonPress && event.xbutton.button == Button5) // scroll dolu
					{
						dbg_e_buttonPress << "odzoomovani" << std::endl;
						wm.zoomOut(x, y);
					}
				}
				else if (w != nullptr)
				{ 
					// prace v okne, pokud je nad oknem a zaroven neni stisknuto Win
					
					// zavreni okna, bylo-li kliknuto na zaviraci tlacitko
					if(event.type == ButtonPress && w->closePoint(x, y)){
						dbg_e_buttonPress << "zavirani okna " << w->window << std::endl;
					}
					
					// zahajeni pretahovani okna, bylo-li kliknuto na dekoraci nebo s Alt
					if(event.type == ButtonPress && (w->handlePoint(x, y) || (event.xbutton.state & Mod1Mask))){
						dbg_e_buttonPress << "zahajeno pretahovani okna " << w->window << std::endl;
						draggedWindow = w;
						dragStartX = x;
						dragStartY = y;
						const unsigned int eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | EnterWindowMask;
						XGrabPointer(wm.display, wm.rootWindow, True, eventMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
					}
					
					// preneseni okna do popredi
					if(event.type == ButtonPress && (event.xbutton.button == Button1 || event.xbutton.button == Button2 || event.xbutton.button == Button3)){
						wm.raiseWindow(w->window);
						XSetInputFocus(wm.display, w->window, RevertToPointerRoot, CurrentTime);
					}
				}
			}
			else if (event.type == ButtonRelease)
			{
				draggedWindow = NULL;
				XUngrabPointer(wm.display, CurrentTime);
			}
			else if (event.type == MotionNotify)
			{
				int x = wm.reverseConvertX(event.xmotion.x_root);
				int y = wm.reverseConvertY(event.xmotion.y_root);
				
				
				if(draggedWindow == (GawmWindow*)&wm){ // presun plochy
					int xdiff = x - dragStartX;
					int ydiff = y - dragStartY;

					wm.moveDesktop(xdiff, ydiff);

					dragStartX = x;
					dragStartY = y;
				}
				else if(draggedWindow != NULL){ // presun okna
					XWindowAttributes attr;
					XGetWindowAttributes(wm.display, draggedWindow->window, &attr);
					signed int xdiff = x - dragStartX;
					signed int ydiff = y - dragStartY;
					
					dbg_e_motion << "presun okna " << draggedWindow->window << " z " << dragStartX << "," << dragStartY;
					dbg_e_motion << " na " << attr.x+xdiff << "," << attr.y+ydiff << std::endl;
					wm.moveResizeWindow(draggedWindow, attr.x+xdiff, attr.y+ydiff, attr.width, attr.height);
					
					dragStartX = x;
					dragStartY = y;
				}
				
				// posun pri krajich - zatim jen primitivni
				if(event.xmotion.x_root == 0){
					wm.moveDesktop(5, 0);
					dragStartX += 5;
				}
				if(event.xmotion.x_root == wm.maxX){
					wm.moveDesktop(-5, 0);
					dragStartX -= 5;
				}
				if(event.xmotion.y_root == 0){
					wm.moveDesktop(0, 5);
					dragStartY += 5;
				}
				if(event.xmotion.y_root == wm.maxY){
					wm.moveDesktop(0, -5);
					dragStartY -= 5;
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
                dbg_e_damage << "Damage na " << dne.area.x << "," << dne.area.y << " o velikosti " << dne.area.width << "x" << dne.area.height << std::endl;
			}
			else
			{
				cerr_line << "Dosel mi typ " << event.type << "; nevim, co s tim..." << std::endl;
			}
		}
		
		wm.render();
		
		XUngrabServer(wm.display);
	}

	return 0;
}
