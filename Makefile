gawm: main.cpp
	# Jsem linej
	g++ -o gawm -std=c++11 main.cpp -lGL -lX11 -lXcomposite

run: gawm
	xinit ./gawm -- /usr/bin/Xephyr :2 &
