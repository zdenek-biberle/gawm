gawm: main.cpp
	# Jsem linej
	g++ -o gawm -std=c++11 main.cpp -lGL -lX11 -lXcomposite

run: gawm
	killall gawm 2>/dev/null; xinit ./gawm -- /usr/bin/Xephyr :2 &

