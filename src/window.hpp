#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

struct GawmWindow
{
public:
	Display * const display;
	const int screen;
	const Window window;
	
	const static int borderTop = 10;
	const static int borderRight = 2;
	const static int borderBottom = 2;
	const static int borderLeft = 2;

	int x;
	int y;
	int width;
	int height;

private:
	const GLubyte* color;
	Pixmap pixmap;
	GLXPixmap glxPixmap;
	GLuint glTexture;
	bool hasPixmap;
	bool visible;
	bool invertedY;
	Damage damage;

public:
	GawmWindow(Display *display, int screen, Window window, int x, int y, int width, int height);
	~GawmWindow();
	
	void configure(int newX, int newY, int newWidth, int newHeight);
	
	void reloadPixmap();
	
	bool containsPoint(int pX, int pY); // bod v okne nebo jeho dekoraci
	
	bool handlePoint(int pX, int pY); // bod v dekoraci umoznujici presun okna
	
	void render();
	
	bool isVisible();
	
	void setVisible(bool visible);
	
	Window getWindow();
	
	void doDamage();
	
};

const GLubyte* selectRandomColor();

#endif
