#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

typedef struct {
    uint8_t reg[16];
    uint8_t mem[4096];
    uint16_t pc;
    uint8_t sp; //stack pointer
    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[16];
    uint32_t video_gfx[64 * 32];
    uint16_t opcode;
    uint32_t randnum;
} CHIP8;

//start adresses 
const unsigned instr_start_adre = 0x200;
const unsigned font_start_adre = 0x50;
const unsigned int font_sz = 80;
//load instruction data from ROM
void load_rom(char const *filename, CHIP8 *chip8) {
    FILE *fptr = fopen(filename, "rb");
    if (fptr == NULL) {
        printf("Failed to get data from rom %s\n", filename);
        exit(1);
    }
    fseek(fptr, 0, SEEK_END);
    long buff_size = ftell(fptr);
    rewind(fptr);

    fread(&chip8->mem[0x200], sizeof(uint8_t), buff_size, fptr);
    fclose(fptr);
    // Test File Reading by print
    for (long i = 0; i < buff_size; i++) {
        printf("%02X ", chip8->mem[0x200 + i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}    




  
//Get Rand num with time seed
uint8_t rand_time(CHIP8 *c){
    c-> randnum = c->randnum * 1103515245 + 12345;
     return (c->randnum >> 16) & 0xFF;
  
   


}

//font array 
uint8_t fontbytes[80]={
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

// init all chip 8 mem values   
void init_chip(CHIP8 *chip8){
    chip8->pc = instr_start_adre;


    for (unsigned int  i= 0 ; i < font_sz  ; ++i ){
        chip8 ->mem[font_start_adre + i ] = fontbytes[i];
    };



    chip8->randnum = (uint8_t)time(NULL);
    if (chip8->randnum == 0) chip8->randnum = 1; 
};

//Set all the OpCode Translation func
//00E0:Clear Display

void OP_00E0(CHIP8 *c){
    memset(c ->video_gfx , 0,sizeof(c->video_gfx));

};

//00EE:RET
void OP_00EE(CHIP8 *c){
    --c-> sp ;
    c->pc = c->stack[c-> sp];
   
}
//1nnn:Jump to adress and set pc to to nnn
void OP_1nnn(CHIP8 *c){
    uint16_t addr = c->opcode &  0x0FFFu;
    c -> pc = addr;

}
//2nnn call addr
void OP_2nnn(CHIP8 *c){
    uint16_t addr = c->opcode &  0x0FFFu;
    c->stack[c->sp] = c->pc;
    ++c->sp;
    c->pc = addr;
    

}
///3xkk skip instruc
void OP_3xkk(CHIP8 *c){
    uint8_t Vx = (c-> opcode & 0x0F00u) >> 8u;
    uint8_t byte_fetch = c->opcode & 0x00FFu;
    if (c->reg[Vx] == byte_fetch){
        c->pc += 2;
    }


}
//4xkk same as last ins but reg[Vx] != kk
void OP_4xkk(CHIP8 *c){
    uint8_t Vx = (c-> opcode & 0x0F00u) >> 8u;
    uint8_t byte_fetch = c->opcode & 0x00FFu;
    if (c->reg[Vx] != byte_fetch){
        c->pc += 2;
    }


}
//5xy0 again but this time when reg[Vx] == reg[vy]
 void OP_5xy0(CHIP8 *c){
    uint8_t Vx = (c-> opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (c->opcode & 0x00F0u) >> 4u;
    if (c->reg[Vx] == c->reg[Vy]){
        c->pc += 2;
    }


}
//6xkk set reg[Vx] = byte
void OP_6xkk(CHIP8 *c){
    uint8_t Vx = (c-> opcode & 0x0F00u ) >> 8u;
    uint8_t byte_fetch = c->opcode & 0x00FFu;
    c->reg[Vx] = byte_fetch;
}
//7xkk add Vx to Byte
void OP_7xkk(CHIP8 *c){
    uint8_t Vx = (c-> opcode & 0x0F00u ) >> 8u;
    uint8_t byte_fetch = c->opcode & 0x00FFu;
    c -> reg[Vx] += byte_fetch;
}
//8xy0 set Vx = Vy
void OP_8xy0(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    c->reg[Vx] = c->reg[Vy];

}
//8xy1  bitwise or on Vx an Vy store Vx
void OP_8xy1(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    c-> reg[Vx] |= c-> reg[Vy];
}
//8xy2 bitwise and on Vx an Vy store Vx
void OP_8xy2(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    c->reg[Vx] &= c->reg[Vy];

}
//8xy3 bitwise XOR on Vx an Vy store Vx
void OP_8xy3(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    c->reg[Vx] ^= c->reg[Vy];
}
//8xy4 Add Vx,Vy Vf =carry if value more than 1 byte 
void OP_8xy4(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    uint16_t sum  = c -> reg[Vx]+c->reg[Vy];
    if (sum > 255u){
        c->reg[0xF] = 1;
    }
    else{
        c->reg[0xF] = 0;

    }    
    c -> reg[Vx] =sum & 0xFFu;
}
//8xy5 sub Vx,Vy Vf =borrow  if value less than 1 byte
void OP_8xy5(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    if (c->reg[Vx]>c->reg[Vy]){
        c->reg[0xF] = 1;
    }else{
        c->reg[0xF] = 0;
    }
    c->reg[Vx]-=c->reg[Vy];
}


int main() {
    CHIP8 chip8;
    
    load_rom("C:\\Users\\Moath\\Documents\\Projects\\CHIP8 EMU\\Space Intercept [Joseph Weisbecker, 1978].ch8", &chip8);

    return 0;
}