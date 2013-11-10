#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

struct GawmWindow
{
private:
	Display *display;
	int screen;
	Window window;
	int x;
	int y;
	int width;
	int height;
	const GLubyte* color;
	Pixmap pixmap;
	GLXPixmap glxPixmap;
	GLuint glTexture;
	bool hasPixmap;
	bool visible;
	bool invertedY;

public:
	GawmWindow(Display *display, int screen, Window window, int x, int y, int width, int height);
	~GawmWindow();
	
	void configure(int newX, int newY, int newWidth, int newHeight);
	
	void reloadPixmap();
	
	void render();
	
	bool isVisible();
	
	void setVisible(bool visible);
};

const GLubyte* selectRandomColor();

#endif
