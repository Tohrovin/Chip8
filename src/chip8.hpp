#include <cstdint>
#include <random>

class Chip8
{
public:
    Chip8();
    void LoadROM(const char* file);
    
private:

	uint8_t registers[16]{};
	uint8_t memory[4096]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t sp{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t keypad[16]{};
	uint32_t video[64 * 32]{}; //64 will give second row and first col, 128: second row, first col and so on
	uint16_t opcode{};

    std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;

            //register functions

            // Do nothing
            void OP_NULL() {};
            // CLS
            void OP_00E0(); //clear the display
            // RET
            void OP_00EE(); //Return from a subroutine.
            // JP address
            void OP_1nnn(); //jump to address nnn
            // CALL address
            void OP_2nnn(); //Call subroutine at nnn.
            // SE Vx, byte
            void OP_3xkk(); //skips if register[x] == kk
            // SNE Vx, byte
            void OP_4xkk(); // skips if register[x] != kk
            // SE Vx, Vy
            void OP_5xy0(); //skips if register[x] == register[y]
            // LD Vx, byte
            void OP_6xkk(); // bytes kk are saved into register Vx.
            // ADD Vx, byte
            void OP_7xkk(); // register Vx += kk
            // LD Vx, Vy
            void OP_8xy0(); // Set Vx = Vy.
            // OR Vx, Vy
            void OP_8xy1(); //Set Vx = Vx OR Vy.
            // AND Vx, Vy
            void OP_8xy2();
            // XOR Vx, Vy
            void OP_8xy3();
            // ADD Vx, Vy
            void OP_8xy4(); //Set Vx = Vx + Vy, set VF = carry.
            // SUB Vx, Vy
            void OP_8xy5(); //Set Vx = Vx - Vy, set VF = NOT borrow.
            // SHR Vx
            void OP_8xy6(); //If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
            // SUBN Vx, Vy
            void OP_8xy7(); //Set Vx = Vy - Vx, set VF = NOT borrow.
            // SHL Vx
            void OP_8xyE(); //left shift
            // SNE Vx, Vy
            void OP_9xy0(); //skip if reg Vx != reg Vy
            // LD I, address
            void OP_Annn();  //Set I = nnn
            // JP V0, address
            void OP_Bnnn(); //Jump to location nnn + V0.
            // RND Vx, byte
            void OP_Cxkk(); //Set Vx = random byte AND kk.
            // DRW Vx, Vy, height
            void OP_Dxyn(); //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            // SKP Vx
            void OP_Ex9E(); //Skip next instruction if key with the value of Vx is pressed.

            // SKNP Vx
            void OP_ExA1(); //Skip next instruction if key with the value of Vx is not pressed.
            // LD Vx, DT
            void OP_Fx07(); //Set Vx = delay timer value.
            // LD Vx, K
            void OP_Fx0A(); //Wait for a key press, store the value of the key in Vx.
            // LD DT, Vx
            void OP_Fx15(); //Set delay timer = Vx.
            // LD ST, Vx
            void OP_Fx18(); //Set sound timer = Vx.
            // ADD I, Vx
            void OP_Fx1E(); //Set I = I + Vx.
            // LD F, Vx
            void OP_Fx29(); //Set I = location of sprite for digit Vx.
            // LD B, Vx
            void OP_Fx33(); //Store BCD representation of Vx in memory locations I, I+1, and I+2.
            // LD [I], Vx
            void OP_Fx55(); //Store registers V0 through Vx in memory starting at location I.
            // LD Vx, [I]
            void OP_Fx65(); //Read registers V0 through Vx from memory starting at location I.




};