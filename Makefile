DIR = obj
OBJECTS = asciianim.o main.o utils.o
OBJ := $(foreach obj, $(OBJECTS), $(DIR)/$(obj))

TARGET := AsciiPlayer
FLAGS  := -Wall -O3 -std=c++11
OPENCV_FLAGS := `pkg-config --cflags --libs opencv4`
LIBS   := -lncurses $(OPENCV_FLAGS)
CC = g++

vpath %.c ./

all: create_folder $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) -o $@ $(OBJ) $(FLAGS) $(LIBS)

$(DIR)/%.o: %.cpp
	@$(CC) -o $@ -c $^ $(FLAGS) $(LIBS)

create_folder:
	@mkdir -p $(DIR)

clean:
	@rm -f $(DIR)/*.o $(TARGET)

.PHONY: all clean create_folder
