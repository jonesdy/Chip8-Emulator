#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <string>
#include <fstream>
#include <iostream>

class Chip8
{
   // Constants
   static const int CLOCK_SPEED = 60;        // In Hz
   static const int PIXEL_WIDTH = 12;
   static const int PIXEL_HEIGHT = 12;
   static const int PIXELS_X = 64;
   static const int PIXELS_Y = 32;
   static const int WINDOW_WIDTH = PIXEL_WIDTH * PIXELS_X;
   static const int WINDOW_HEIGHT = PIXEL_HEIGHT * PIXELS_Y;
   static const int MEMORY_SIZE = 4096;
   static const int NUM_REGISTERS = 16;
   static const int STACK_SIZE = 16;
   static const int NUM_KEYS = 16;
   static const int PROGRAM_START = 0x200;
   static const int FONTSET_SIZE = 0x50;

   // SFML Stuff
   sf::RenderWindow window;
   sf::RectangleShape pixel;

   // Chip8 Stuff
   unsigned char graphics[PIXELS_X * PIXELS_Y];
   unsigned short opcode;
   unsigned char memory[MEMORY_SIZE];
   unsigned char V[NUM_REGISTERS];     // The registers V0-VE and carry flag
   unsigned short I;                   // Index register
   unsigned short pc;                  // Program counter
   unsigned char delayTimer;
   unsigned char soundTimer;
   unsigned short stack[STACK_SIZE];
   unsigned short sp;                  // Stack pointer
   unsigned char key[NUM_KEYS];        // Hex keypad, 0x0-0xF
   bool drawFlag;

   // Helper functions
   void loadFontSet();
   void drawGraphics();
   void tick();
   void setKeys(sf::Event evt);
   void clearScreen();

   // Debugging functions
   void dumpMemory();
   void dumpKeys();
public:
   Chip8();
   void loadProgram(std::string fileName);
   void run();
   ~Chip8();
};

#endif