CXX := g++
LDFLAGS := -lncurses

all:
	$(CXX) main.c $(LDFLAGS)
