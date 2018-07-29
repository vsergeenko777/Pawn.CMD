OUTFILE = "pawncmd.so"
 
COMPILE_FLAGS=-m32 -c -O3 -w -Ilib
LINK_FLAGS=-m32 -shared -O3 -static-libstdc++ -lboost_locale -lboost_system

all:
	gcc $(COMPILE_FLAGS) lib/SDK/amx/*.h
	g++ $(COMPILE_FLAGS) -std=c++11 lib/SDK/*.cpp		
	g++ $(COMPILE_FLAGS) -std=c++11 src/*.cpp
	g++ -o $(OUTFILE) *.o $(LINK_FLAGS)
	rm *.o
	strip -s $(OUTFILE)
