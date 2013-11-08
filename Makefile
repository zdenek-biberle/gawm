# -*- coding: utf-8 -*-
#
# Makefile pro projekt GAWM
#

# Jméno přeloženého programu
program=gawm

# Číslo displeje pro Xephyr
display=:2

# Úroveň množství debugovacích informací
LDB=-g3 -DDEBUG

# Seznam ostatních souborů.
OTHER=Makefile

# Překladač C
CPP=g++
CXX=$(CPP)

# Link
LINK=-lGLEW -lGL -lX11 -lXcomposite

# Makra
# MACROS=-D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED

# Nepovinné parametry překladače
BRUTAL=-Wall -Wextra -Werror -Wno-unused-variable
CXXFLAGS=-std=c++11 $(STRICT) -pedantic $(MACROS)

SOURCES=main gawmGl

$(program): $(addprefix obj/,$(addsuffix .o,$(SOURCES)))
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LINK)

obj/%.o: src/%.cpp
	mkdir -p dep obj # Adresare nejsou v gitu
	$(CXX) -MMD -MP -MF dep/$*.d -c -o $@ $< $(CXXFLAGS) -DNDEBUG

obj/dbg/%.o: src/%.cpp
	mkdir -p dep/dbg obj/dbg # Adresare nejsou v gitu
	$(CXX) -MMD -MP -MF dep/dbg/$*.d -c -o $@ $< $(CXXFLAGS) $(LDB)

-include $(addprefix dep/,$(addsuffix .d,$(SOURCES)))

#######################################################################
.PHONY: build strict clean pack test run valgrind kdbg debug

# Zkompiluje program (výchozí)
build: $(program)

strict:
	make "STRICT=$(BRUTAL)"

# Spustí testy
run: $(program)
	xinit ./$(program) -- /usr/bin/Xephyr $(display) -screen 800x600 &

test: run
	sleep 3
	DISPLAY=$(display) xterm

valgrind: debug
	/usr/bin/Xephyr $(display) & \
	xephyr_p=$$!; \
	DISPLAY=$(display) valgrind --tool=memcheck --leak-check=yes --show-reachable=yes './$(program)-dbg'; \
	kill $$xephyr_p;

gdb: debug
	/usr/bin/Xephyr $(display) & \
	xephyr_p=$$!; \
	DISPLAY=$(display) gdb './$(program)-dbg'; \
	kill $$xephyr_p;

kdbg: debug
	xinit './$(program)-dbg' -- /usr/bin/Xephyr $(display) & \
	xinit_p=$$!; \
	bash ./debug-kdbg.sh; \
	kill $$xinit_p;

#  Debug
#  *****

debug: $(addprefix obj/dbg/,$(addsuffix .o,$(SOURCES)))
	$(CXX) -o '$(program)-dbg' $^ $(CXXFLAGS) $(BRUTAL) $(LDB) $(LINK)

clean:
	rm -f obj/*.o obj/dbg/*.o dep/*.d dep/dbg/*.d './$(program)' './$(program)-dbg'
	rm -f pack/$(program) pack/*.*

pack:
	make -C doc
	rm -f pack/$(program) pack/*.*
	bash -c 'cp src/*[^~] pack/; cp doc/$(program).pdf pack/dokumentace.pdf; cd pack; tar -zcvf ../xbiber00.tgz *'
