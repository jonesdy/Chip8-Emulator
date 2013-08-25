CC=g++
CFLAGS=-Wall -c
EXECUTABLE=Chip8
SFML_INCLUDES=-I./$(SFML_FOLDER)/include
SFML_LIBS=-lsfml-system -lsfml-window -lsfml-graphics

all:main.o Chip8.o
	$(CC) -o $(EXECUTABLE) $^ $(SFML_LIBS)

%.o:%.cpp
	$(CC) $(CFLAGS) $(SFML_INCLUDES) $<

clean:
	rm -rf *o $(EXECUTABLE)
