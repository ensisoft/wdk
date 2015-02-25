
CFLAGS=-DLINUX -std=c++11 -g -O0
INCS=-I..
LIBS=-lX11 -lxcb -lXau -lXxf86vm -lXext -lXrandr
GLES=-lEGL -lGLESv2
GLGL=-lGL

sources = $(wildcard *.cpp) $(wildcard X11/*.cpp)
headers = $(wildcard *.h)
glx = $(wildcard GLX/*.cpp)
egl = $(wildcard EGL/*.cpp)

all: events triangle-opengl-gl	triangle-opengl-es

events: sample/events.cpp 
	g++ -o events $(CFLAGS) $(INCS) sample/events.cpp $(sources) $(LIBS)

triangle-opengl-gl: sample/triangle.cpp
	g++ -o triangle-opengl-gl $(CFLAGS) $(INCS) -DWDK_DESKTOP sample/triangle.cpp $(sources) $(glx) $(LIBS) $(GLGL)

triangle-opengl-es: sample/triangle.cpp
	g++ -o triangle-opengl-es $(CFLAGS) $(INCS) -DWDK_MOBILE -DSAMPLE_GLES sample/triangle.cpp $(sources) $(egl) $(LIBS) $(GLES)

clean:
	rm events triangle-opengl-gl triangle-opengl-es
