#ifndef WINDOW_HPP
#define WINDOW_HPP

const GLubyte* selectRandomColor();

struct GawmWindow
{
private:
	GawmWindowManager* wm;
	Window window;
	int x;
	int y;
	int width;
	int height;
	const GLubyte* color;
	Pixmap pixmap;
	bool hasPixmap;
	bool visible;

public:
	GawmWindow(GawmWindowManager *wm, Window window, int x, int y, int width, int height):
		wm(wm),
		window(window),
		x(x),
		y(y),
		width(width),
		height(height),
		color(selectRandomColor()),
		hasPixmap(false),
		visible(false)
	{
		std::cout << "CreateNotify: Vytvoreno okno " << window << " na " << x << ", " << y
		          << " velikosti " << width << "x" << height << std::endl;
		
		XCompositeRedirectWindow(wm->display, window, CompositeRedirectManual);
		
		reloadPixmap();
	}
	
	~GawmWindow()
	{
		std::cout << "DestroyNotify: Zniceno okno " << window << std::endl;
	}
	
	void configure(int newX, int newY, int newWidth, int newHeight){
		x = newX;
		y = newY;
		width = newWidth;
		height = newHeight;
		
		std::cout << "ConfigureNotify: Zmeneno okno " << window
					<< " s pozici " << x << ", " << y << " a velikosti "
					<< width << "x" << height << std::endl;
		
		reloadPixmap();
	}
	
	void reloadPixmap(){
		std::cout << "reload(" << wm->display << "," << window << ")" << std::endl;
		if (isVisible())
		{
			pixmap = XCompositeNameWindowPixmap(wm->display, window);
			XSync(wm->display, False);
			hasPixmap = true;
		}
		std::cout << "reloadnuto" << std::endl;
	}
	
	// Tohle predelat na VBO nebo neco takovyho
	void render(){
		if(isVisible())
		{
			glColor3ubv(color);
			glVertex2i(x, y);
			glVertex2i(x, y + height);
			glVertex2i(x + width, y + height);
			glVertex2i(x + width, y);
		}
	}
	
	bool isVisible()
	{
		return visible;
	}
	
	void setVisible(bool visible)
	{
		this->visible = visible;
		reloadPixmap();
	}
};

const GLubyte* selectRandomColor()
{

	static GLubyte colors[] = 
	{
		0,128,128,
		0,128,255,
		0,255,128,
		0,255,255,
		128,0,128,
		128,0,255,
		128,128,0,
		128,128,128,
		128,128,255,
		128,255,128,
		128,255,255,
		255,0,128,
		255,0,255,
		255,128,0,
		255,128,128,
		255,128,255,
		255,255,0,
		255,255,128,
		255,255,255
	};
	
	return colors + (rand() % 19)*3;
}

#endif
