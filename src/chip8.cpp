#include "chip8.hpp"
#include <fstream>
#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <cstring>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

uint8_t fontset[FONTSET_SIZE] =
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

Chip8::Chip8():randGen(std::chrono::system_clock::now().time_since_epoch().count()) //setting the seed for random generator in the chip8
{
    pc = START_ADDRESS; //first 512 bytes of the memory in the machine are for the interpreter, program strarts at 0x200
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U); //initializing the random byte

    for(unsigned int i = 0; i<FONTSET_SIZE; i++) // only the 16 built-in characters are used in the beginning bites
    {
        memory[FONTSET_START_ADDRESS+i] = fontset[i];
    }
}

void Chip8::LoadROM(char const* filename)
{
	// Open the file as a stream of binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get size of file and allocate a buffer to hold the contents
		std::streampos size = file.tellg(); //tellg - returns current position in the stream

		// char* buffer = new char[size];
         char* buffer = (char*)malloc(size * sizeof(char));

        // Checking if the allocation was succesfull
        if (buffer == nullptr) {
            std::cerr << "Allocation error!" << std::endl;
        }


		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg); //seekg - Sets the position of the next character to be extracted from the input stream
		file.read(buffer, size); //read - Extracts n characters from the stream and stores them in the array pointed to by s
		file.close();

		// Load the ROM contents into the Chip8's memory, starting at 0x200
		for (long i = 0; i < size; i++)
		{
			memory[START_ADDRESS] = buffer[i];
		}

		// Free the buffer

		// delete[] buffer; //in case of using c++ new (new allows for space allocations for classes and such)
        free(buffer); 
	}
}


void Chip8::OP_00E0() //clear the display
{
	memset(video, 0, sizeof(video)); //fills the memory block with value
}

void Chip8::OP_00EE() //return from the subroutine
{
	--sp;   //The top of the stack has the address of one instruction past the one that called the subroutine
	pc = stack[sp]; //so just set that address as the program counter and all should be good
} 

void Chip8::OP_1nnn() //Jump to location nnn.
{
    pc = (uint16_t)(opcode & 0x0FFFu); //0x0FFFu isolates last 12 bites from opcode (the address, first 4bites mean the operation)
}

void Chip8::OP_2nnn() //Call subroutine at nnn.
{   
    stack[sp] = pc; //sets the current instruction at the top of the stack
    sp++;
    pc = (uint16_t)(opcode & 0x0FFFu);
}

void Chip8::OP_3xkk() //Skip next instruction if register Vx = kk.
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; //isolates the x byte and then shifts 8bits right (2 fields in 16)
	uint8_t kk = opcode & 0x00FFu; //isolates the kk bytes, no need for shift

    if (registers[Vx] == kk ){pc+=2;} //next instruction is in pc++, so we just add another one
}

void Chip8::OP_4xkk() //Skip next instruction if register Vx != kk.
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t kk = opcode & 0x00FFu; 

    if (registers[Vx] != kk ){pc+=2;}
}

void Chip8::OP_5xy0() //skips if register[x] == register[y]
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; //isolates the x byte and then shifts 8bits right 
    uint8_t Vy = (opcode & 0x00F0u) >> 4u; //isolates the y byte and then shifts 4bits right 

    if (registers[Vx]==registers[Vy]){pc+=2;}
}

void Chip8::OP_6xkk() // saves kk into register Vx
{
    uint8_t kk = (opcode & 0x00FFu);
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx]=kk;

}

void Chip8::OP_7xkk() //Set Vx = Vx + kk
{
    uint8_t kk = (opcode & 0x00FFu);
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx]+=kk;
}

void Chip8::OP_8xy0() //Sets Vy as the value of Vx
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx]=registers[Vy];
}

void Chip8::OP_8xy1() //or
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2() //and
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3() //Vx xor Vy
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4() //Vx + Vy
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint16_t sum = registers[Vx] + registers[Vy];
	if (sum>255u)
	{
		registers[0xF]=1u;
	}
	else 
	{
		registers[0xF]=0u;
	}

	registers[Vx] = (uint8_t)(sum & 0xFFu);
}

void Chip8::OP_8xy5() //Vx - Vy
{
    uint8_t Vy = (opcode & 0x00F0u)>> 4u;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF]=1u;
	}
	else 
	{
		registers[0xF]=0u;
	}
	registers[Vx] -= registers[Vy];	
	
}

void Chip8::OP_8xy6() //shift right
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF]= (registers[Vx] & 0x01u);
	registers[Vx] >>= 1;	
}

void Chip8::OP_8xy7() //Vy - Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1u;
	}
	else
	{
		registers[0xF] = 0u;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE() //shift left
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void Chip8::OP_9xy0() //skip
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn() //The value of register I is set to nnn. (I is index)
{
	index = (opcode & 0x0FFFu);
}

void Chip8::OP_Bnnn() //Jump to location nnn + V0.
{
	uint16_t address = opcode & 0x0FFFu;
	pc = registers[0] + address;
}

void Chip8::OP_Cxkk() //Set Vx = random byte AND kk.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn() //draw sprites
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t n = opcode & 0x000Fu;

	// modulo ensures wrapping at the screen, idk if sprites should wrap immiedetaly too
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;


	for (unsigned int row = 0; row < n; row++)
	{
		uint8_t spriteByte = memory[index + row]; //reads the sprite from memory

		for (unsigned int col = 0; col < 8; col++) //the sprites are all 8 pixels wide (and to 15 pixels high)
		{	
			uint8_t spritePixel = spriteByte & (0b00000001 << col); //shift throught the columns

			//for video i need row position x 64 (width) and the rest is column pos (one number for storing both coords)
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)]; 
			if (spritePixel) //if the pixel is on
			{
				// checks the collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1u;
				}
				else
				{
					registers[0xF] = 0u;
				}

				// Sprites are XORed onto the existing screen (we get visibility on both white and black backgrounds)
				*screenPixel ^= 0xFFFFFFFF;
			}
			else
			{
				//if the sprite pixel is off nothing happens
			}
			
			
		}
	}
}

void Chip8::OP_Ex9E() //skip if key is pressed
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
 
	if (keypad[registers[Vx]]) //will this check if the key is pressed?
	{
		pc += 2;
	}
}

void Chip8::OP_Ex9E() //skip if key is not pressed
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
 
	if (!keypad[registers[Vx]]) //will this check if the key is pressed?
	{
		pc += 2;
	}
}

void Chip8::OP_Fx07() //delay timer in Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[Vx] = delayTimer;
}

//key ones are potentially a problem, remember for debugging
void Chip8::OP_Fx0A() //All execution stops until a key is pressed, then the value of that key is stored in Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for(uint8_t i = 0; i<16; i++)
	{
		if (keypad[i])
		{
			registers[Vx] = i;
			break; 
		}
		if (i==15){i=0;}
	}

}

void Chip8::OP_Fx15() //sets the delay timer equal to the value of Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delayTimer = registers[Vx];
}

void Chip8::OP_Fx18() //sets the sound timer equal to the value of Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() //Set I = I + Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index += registers[Vx];
}

void Chip8::OP_Fx29() //Set I = location of sprite for digit Vx. (8x5 number sprites)
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];
	index += FONTSET_START_ADDRESS + (value*5);
}

void Chip8::OP_Fx33() //Store BCD representation of Vx in memory locations I, I+1, and I+2.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	memory[index+2] = Vx %10; //ones digit
	memory[index+1] = (Vx/10) %10; //tens digit
	memory[index] = (Vx/100) %10; //hundreds digit
	
}

void Chip8::OP_Fx55() //Store registers V0 through Vx in memory starting at location I.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; i++)
	{
		memory[index+i] = registers[i];
	}

}

void Chip8::OP_Fx65() //Read registers V0 through Vx from memory starting at location I.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; i++) //reads values from memory starting at location I into registers V0 through Vx
	{
		registers[i] = memory[index + i];
	}

}







