#include "Chip8.hpp"

Chip8::Chip8()
{

   // Set up the SFML stuff
   window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chip8 Emulator");
   window.setFramerateLimit(CLOCK_SPEED);
   pixel.setSize(sf::Vector2f(PIXEL_WIDTH, PIXEL_HEIGHT));
   pixel.setFillColor(sf::Color::White);

   // Initialize
   clearScreen();
   opcode = 0;
   for(int i = 0; i < MEMORY_SIZE; i++)
      memory[i] = 0;
   for(int i = 0; i < NUM_REGISTERS; i++)
      V[i] = 0;
   I = 0;
   pc = PROGRAM_START;
   delayTimer = 0;
   soundTimer = 0;
   for(int i = 0; i < STACK_SIZE; i++)
      stack[i] = 0;
   sp = 0;
   for(int i = 0; i < NUM_KEYS; i++)
      key[i] = 0;
   drawFlag = false;
   srand(time(NULL));

   // Load fontset
   loadFontSet();

}

void Chip8::loadFontSet()
{

   unsigned char chip8_fontset[FONTSET_SIZE] =
   {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
   };

   for(int i = 0; i < FONTSET_SIZE; i++)
      memory[i] = chip8_fontset[i];

}

void Chip8::drawGraphics()
{

   window.clear();

   for(int i = 0; i < PIXELS_X; i++)
   {
      for(int j = 0; j < PIXELS_Y; j++)
      {
         if(graphics[i + (j * PIXELS_X)])
         {
            pixel.setPosition(sf::Vector2f(i * PIXEL_WIDTH, j * PIXEL_HEIGHT));
            window.draw(pixel);
         }
      }
   }

   window.display();

   drawFlag = false;

}

void Chip8::tick()
{

   // Fetch opcode
   opcode = memory[pc] << 8 | memory[pc + 1];

   // Decode opcode
   switch(opcode & 0xF000)
   {
   case 0x0000:
      {
         switch(opcode & 0x000F)
         {
         case 0x0000:   // 0x00E0: Clear screen
            clearScreen();
            break;
         case 0x000E:   // 0x00EE: Returns from subroutine
            pc = stack[--sp] + 2;
            break;
         default:
            {
               std::cout<<"Unknown opcode: 0x"<<std::hex<<opcode<<"\n";
            }
         }
         break;
      }
   case 0x1000:   // 0x1NNN: Jump to address NNN
      {
         pc = opcode & 0x0FFF;
         break;
      }
   case 0x2000:   // 0x2NNN: Call subroutine at NNN
      {
         stack[sp++] = pc;
         pc = opcode & 0x0FFF;
         break;
      }
   case 0x3000:   // 0x3XNN: Skips the instruction if VX equals NN
      {
         unsigned char num = opcode & 0x00FF;
         if(V[(opcode & 0x0F00) >> 8] == num)
            pc += 4;
         else
            pc += 2;
         break;
      }
   case 0x4000:   // 0x4XNN: Skips the next instruction if VX doesn't equal NN
      {
         unsigned char num = opcode & 0x00FF;
         if(V[(opcode & 0x0F00) >> 8] == num)
            pc += 2;
         else
            pc += 4;
         break;
      }
   case 0x6000:   // 0x6XNN: Sets VX to NN
      {
         V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
         pc += 2;
         break;
      }
   case 0x7000:   // 0x7XNN: Adds NN to VX
      {
         unsigned char add = opcode & 0x00FF;
         V[(opcode & 0x0F00) >> 8] += add;
         pc += 2;
         break;
      }
   case 0x8000:
      {
         switch(opcode & 0x000F)
         {
         case 0x0000:   // 0x8XY0: Sets VX to the value of VY
            {
               V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
               pc += 2;
               break;
            }
         case 0x0002:   // 0x8XY2: Sets VX to VX and VY
            {
               V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
               pc += 2;
               break;
            }
         case 0x0004:   // 0x8XY4: Add VX to VY and set VF if needed
            {
               if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))   // Value at VY is larger than (max - value at VX)
                  V[0xF] = 1;
               else
                  V[0xF] = 0;
               V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
               pc += 2;
               break;
            }
         case 0x0005:   // 0x8XY5: VY is subtracted from VX.  VF is set to 0 when there's a borrow, and 1 when there isn't
            {
               if(V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 8])      // VX < VY, borrow
                  V[0xF] = 0;
               else
                  V[0xF] = 1;
               V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 8];
               pc += 2;
               break;
            }
         case 0x0007:   // 0x8XY7: Sets VX to VY minus VX.  VF is set to 0 when there's a borrow, and 1 when there isn't
            {
               if(V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8])      // VY < VX, borrow
                  V[0xF] = 0;
               else
                  V[0xF] = 1;
               V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
               pc += 2;
               break;
            }
         default:
            {
               std::cout<<"Unknown opcode: 0x"<<std::hex<<opcode<<"\n";
            }
         }
         break;
      }
   case 0xA000:   // 0xANNN: Sets I to the address NNN
      {
         I = opcode & 0x0FFF;
         pc += 2;
         break;
      }
   case 0xC000:   // 0xCXNN: Sets VX to a random number and NN
      {
         unsigned char andNum = opcode & 0x00FF;
         unsigned char randNum = rand();
         V[(opcode & 0x0F00) >> 8] = andNum & randNum;
         pc += 2;
         break;
      }
   case 0xD000:   // 0xDXYN: Draw sprite at coordinates VX, VY with a width of 8 pixels and height of N.  Sprite starts at I
      {
         unsigned short x = V[(opcode & 0x0F00) >> 8];
         unsigned short y = V[(opcode & 0x00F0) >> 4];
         unsigned short height = opcode & 0x000F;
         unsigned short pix;

         V[0xF] = 0;
         for(int yline = 0; yline < height; yline++)
         {
            pix = memory[I + yline];
            for(int xline = 0; xline < 8; xline++)
            {
               if((pix & (0x80 >> xline)) != 0)
               {
                  if(graphics[(x + xline + ((y + yline) * PIXELS_X))] == 1)
                     V[0xF] = 1;
                  graphics[x + xline + ((y + yline) * PIXELS_X)] ^= 1;
               }
            }
         }

         drawFlag = true;
         pc += 2;
         break;
      }
   case 0xE000:
      {
         switch(opcode & 0x00FF)
         {
         case 0x009E:   // 0xEX9E skips the next instruction if the key stored in VX is pressed
            {
               if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                  pc += 4;
               else
                  pc += 2;
               break;
            }
         case 0x00A1:   // 0xEX9E skips the next instruction if the key stored in VX isn't pressed
            {
               if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                  pc += 2;
               else
                  pc += 4;
               break;
            }
         default:
            {
               std::cout<<"Unknown opcode: 0x"<<std::hex<<opcode<<"\n";
            }
         }
         break;
      }
   case 0xF000:
      {
         switch(opcode & 0x00FF)
         {
         case 0x0007:   // 0xFX07: Sets VX to the value of the delay timer
            {
               V[(opcode & 0x0F00) >> 8] = delayTimer;
               pc += 2;
               break;
            }
         case 0x0015:   // 0xFX15: Sets the delay timer to VX
            {
               delayTimer = V[(opcode & 0x0F00) >> 8];
               pc += 2;
               break;
            }
         case 0x0018:   // 0xFX18: Sets the sound timer to VX
            {
               soundTimer = V[(opcode & 0x0F00) >> 8];
               pc += 2;
               break;
            }
         case 0x0029:   // 0xFX29: Sets I to the location of the sprite for the character in VX
            {
               unsigned char ch = V[(opcode & 0x0F00) >> 8];
               I = ch * 10;      // 10 bytes per character
               pc += 2;
               break;
            }
         case 0x0033:   // 0xFX33: Stores the binary-coded decimal representation of VX at I, I + 1, and I + 2
            {
               memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
               memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
               memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
               pc += 2;
               break;
            }
         case 0x0065:   // 0xFX65: Fills V0 to VX with values from memory starting at address I
            {
               unsigned char numRegs = (opcode & 0x0F00) >> 8;
               for(int i = 0; i < numRegs; i++)
                  V[i] = memory[I + (i * 2)];
               pc += 2;
               break;
            }
         default:
            {
               std::cout<<"Unknown opcode: 0x"<<std::hex<<opcode<<"\n";
            }
         }
         break;
      }
   default:
      {
         std::cout<<"Unknown opcode: 0x"<<std::hex<<opcode<<"\n";
      }
   }

   // Update timers
   if(delayTimer > 0)
      delayTimer--;

   if(soundTimer > 0)
   {
      if(soundTimer == 1)
         std::cout<<"\a";
      soundTimer--;
   }

}

void Chip8::setKeys()
{

   key[1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
   key[2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
   key[3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
   key[0xC] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);
   key[4] = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
   key[5] = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
   key[6] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
   key[0xD] = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
   key[7] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
   key[8] = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
   key[9] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
   key[0xE] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
   key[0xA] = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
   key[0] = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
   key[0xB] = sf::Keyboard::isKeyPressed(sf::Keyboard::C);
   key[0xF] = sf::Keyboard::isKeyPressed(sf::Keyboard::V);

}

void Chip8::clearScreen()
{

   for(int i = 0; i < PIXELS_X * PIXELS_Y; i++)
      graphics[i] = 0;

}

void Chip8::dumpMemory()
{

   for(int i = 0; i < MEMORY_SIZE; i++)
      std::cout<<std::hex<<(int)memory[i];

   std::cout<<"\n";

}

void Chip8::dumpRegisters()
{

   for(int i = 0; i < NUM_REGISTERS; i++)
      std::cout<<std::hex<<(int)V[i];

   std::cout<<"\n";

}

void Chip8::dumpKeys()
{

   for(int i = 0; i < NUM_KEYS; i++)
      std::cout<<(int)key[i];

   std::cout<<"\n";

}

void Chip8::loadProgram(std::string fileName)
{

   std::ifstream input(fileName, std::ifstream::binary);

   if(input)
   {
      // Get the length of the file
      input.seekg(0, input.end);
      int length = input.tellg();
      input.seekg(0, input.beg);

      // Make our buffer
      unsigned char *buffer = new unsigned char[length];

      // Read from the file
      input.read((char*)buffer, length);

      if(!input)
         std::cout<<"Error reading file: only "<<input.gcount()<<" could be read\n";

      // Move the program into memory
      for(int i = 0; i < length; i++)
         memory[i + PROGRAM_START] = buffer[i];

      input.close();
      delete[] buffer;
   }
   else
      std::cout<<"Error reading file: "<<fileName<<"\n";

}

void Chip8::run()
{

   sf::Clock clock;
   clock.restart();

   const sf::Time FRAMETIME = sf::microseconds(1000000.f / CLOCK_SPEED);      // No in-class initialization allowed?

   while(window.isOpen())
   {
      sf::Event event;
      while(window.pollEvent(event))
      {
         // Request for closing the window
         if(event.type == sf::Event::Closed)
            window.close();
      }

      // The clock cycle for the CPU needs to be 60Hz
      sf::Time timeElapsed = clock.restart();
      if(timeElapsed > FRAMETIME)
         sf::sleep(FRAMETIME - timeElapsed);

      // Emulate
      tick();

      // Draw the graphics
      if(drawFlag)
         drawGraphics();

      // Store key states
      setKeys();
   }

}

Chip8::~Chip8()
{
}
