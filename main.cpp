#include "Chip8.hpp"

int main(int argc, char* argv[])
{

   Chip8 chip8;

   if(argc <= 1)
      chip8.loadProgram("PONG");
   else
      chip8.loadProgram(argv[1]);

   chip8.run();

   return 0;

}
