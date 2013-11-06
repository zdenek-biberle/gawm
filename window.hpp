#ifndef WINDOW_HPP
#define WINDOW_HPP

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

struct GawmWindow
{
	
	Window window;
	int x;
	int y;
	int width;
	int height;
	const GLubyte* color;
	
	GawmWindow():
		window(0),
		x(-1),
		y(-1),
		width(-1),
		height(-1),
		color(nullptr)
	{}
	
	GawmWindow(Window window, int x, int y, int width, int height):
		window(window),
		x(x),
		y(y),
		width(width),
		height(height)
	{
		color = selectRandomColor();
		
		std::cout << "CreateNotify: Vytvoreno okno " << window << " na " << x << ", " << y
		          << " velikosti " << width << "x" << height << std::endl;
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
	}
	
	// Tohle predelat na VBO nebo neco takovyho
	void render(){
		glColor3ubv(color);
		glVertex2i(x, y);
		glVertex2i(x, y + height);
		glVertex2i(x + width, y + height);
		glVertex2i(x + width, y);
	}
	
};

#endif
