display=:2

gawm: Makefile main.cpp utils.hpp window.hpp winmgr.hpp
	# Jsem proste Biba
	g++ -o gawm -std=c++11 main.cpp -lGL -lX11 -lXcomposite -g3

run: gawm
	killall gawm 2>/dev/null; xinit ./gawm -- /usr/bin/Xephyr $(display) &

test: run
	sleep 1
	DISPLAY=$(display) xterm
