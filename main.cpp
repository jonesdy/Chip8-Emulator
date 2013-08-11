#include "Chip8.hpp"

int main()
{

   Chip8 chip8;

   chip8.loadProgram("PONG");

   chip8.run();

   return 0;

}