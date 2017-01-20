main.bin: svrlib.h svrutil.h svrutil.cpp svrlib_main.cpp
	g++ -std=c++11 -o svrlib_main.bin svrlib.h svrutil.h svrutil.cpp svrlib_main.cpp
