


CFLAGS=-DLINUX -std=c++11
INC=-I../../ -I../
LIBS=-lX11 -lxcb -lXau -lXxf86vm -lXext
GLES=-lEGL -lGLESv2
GL=-lGL

simple: simple.cpp
	g++ -o simple $(CFLAGS) $(INC) simple.cpp ../X11/window.cpp ../X11/keyboard.cpp ../X11/keysym2ucs.cpp ../X11/display.cpp $(LIBS)

gl1: triangle.cpp
	g++ -o gl1 $(CFLAGS) -DMAKE_GL1 $(INC) triangle.cpp   ../X11/window.cpp ../X11/keyboard.cpp ../X11/keysym2ucs.cpp ../X11/display.cpp ../X11/context.cpp $(LIBS) $(GL)

gl2: triangle.cpp
	g++ -o gl2 $(CFLAGS) -DMAKE_GL2 $(INC) triangle.cpp   ../X11/window.cpp ../X11/keyboard.cpp ../X11/keysym2ucs.cpp ../X11/display.cpp ../X11/context.cpp $(LIBS) $(GL)

gles: triangle.cpp
	g++ -o gles $(CFLAGS) -DMAKE_GL_ES $(INC) triangle.cpp   ../X11/window.cpp ../X11/keyboard.cpp ../X11/keysym2ucs.cpp ../X11/display.cpp ../EGL/context.cpp $(LIBS) $(GLES)

clean:
	rm gl1 gl2 gles simple