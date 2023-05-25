CXX := g++
OPENCV_FLAGS := `pkg-config --cflags --libs opencv4`
LDFLAGS := -lncurses $(OPENCV_FLAGS) -std=c++11

all:
	$(CXX) main.cpp $(LDFLAGS) -o AsciiPlayer
