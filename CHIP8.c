#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

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
  
   


};

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







int main() {
    CHIP8 chip8;
    
    load_rom("C:\\Users\\Moath\\Documents\\Projects\\CHIP8 EMU\\Space Intercept [Joseph Weisbecker, 1978].ch8", &chip8);

    return 0;
}