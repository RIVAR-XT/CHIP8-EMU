#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>



#define VIDEO_WIDTH  64
#define VIDEO_HEIGHT 32


typedef struct {
    uint8_t reg[16];
    uint8_t mem[4096];
    uint16_t pc;
    uint16_t index;
    uint8_t sp; //stack pointer
    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[16];
    uint8_t Height;
    uint8_t Width;
    uint32_t video_gfx[VIDEO_WIDTH * VIDEO_HEIGHT];
    uint16_t opcode;
    uint32_t randnum; //seed for equa
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
    for (long i = 0; i < buff_size; ++i) {
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
//8xy6 set reg Vx to Vx SHR 1 and if least sig bit is 1 Vf should be raised
void OP_8xy6(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
    c-> reg[0xF] = (c -> reg[Vx] & 0x1u );
    c->reg[Vx] >>= 1 ;
	
}
//8xy7 set vx = Vy - Vx and Vf is flagged when Vx>= Vy
void OP_8xy7(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;

    if (c-> reg[Vx] > c-> reg[Vy]){
        c->reg[0xF] = 1;
    }else{
        c->reg[0xF] = 0;
    }
    c->reg[Vx] = c->reg[Vy] - c->reg[Vx];
}
//8xyE LSB Vx by 1 and set Vf to 1 if the least sig bit prior to the shift was up
void OP_8xyE(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
    c->reg[0xF] = (c-> reg[Vx] & 0x80u) >> 7u;
    c->reg[Vx] <<= 1;
}
//9xy0 skip to the next instruction if Vx != Vy
void OP_9xy0(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    if (c-> reg[Vx] != c-> reg[Vy]){
        c->pc += 2;

    }    
}
//ANNN set adreess I to NNN
void OP_ANNN(CHIP8 *c){
    uint16_t address = c->opcode & 0x0FFFu ;
    c-> index = address;
}
//Bnnn jump to instruc in adress nnn+V0
void OP_BNNN(CHIP8 *c){
    uint16_t adress = c->opcode & 0x0FFFu;
    c->pc = c->reg[0]+adress;
}
//CxNN set reg Vx to random byte & NN
void OP_CxNN(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u)>>8u;
    uint8_t byte = c->opcode & 0x00FFu;
    c->reg[Vx]=rand_time(c) & byte;
}
//Dxyn display the sprites on the 8 by 8 screen using mem Vx to Vy id 2 pixels collide Vf=colision 
void Dxyn(CHIP8 *c){
    uint8_t Vx = (c ->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (c ->opcode & 0x00F0u) >> 4u;
    uint8_t height = c->opcode & 0x000Fu;

    uint8_t xpos = c->reg[Vx] % VIDEO_WIDTH;
    uint8_t ypos = c->reg[Vy] % VIDEO_HEIGHT;


    c->reg[0xF] = 0;




    for (unsigned int row = 0  ; row < height ; ++row ){
        uint8_t spritebyte = c->mem[c->index + row];
        for (unsigned int col = 0 ; col < 8 ; ++col  ){
            uint8_t spritepixel = spritebyte & (0x80u>>col);
            uint32_t* gfx_pixel = &c->video_gfx[(ypos + row) * VIDEO_WIDTH +(xpos +col)];

            if (spritepixel){
                if (*gfx_pixel==0xFFFFFFFF){
                    c->reg[0xF] =1;

                }

                *gfx_pixel ^=  0xFFFFFFFF;
            }
        }
    }
}

//Ex9E skip to next ins if key is pressed   
void OP_Ex9E(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    uint8_t key = c->reg[Vx];
    if (c->keypad[key]){
        c->pc += 2;
    }

}
//ExA1 skip next ins if key value is false or not pressed
void OP_ExA1(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    uint8_t key = c->reg[Vx];

    if (!c->keypad[key]){
        c->pc+=2;
    }
}
//Fx07 load Vx , Delay timer
void OP_Fx07(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    c->reg[Vx]=c->delay_timer;
}
//Fx0A load key reg into Vx key
void OP_Fx0A(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    if (c->keypad[0]){
        c->reg[Vx]=0;

    }else if(c->keypad[1]){
        c->reg[Vx]=1;

    }else if (c->keypad[2]){
        c->reg[Vx]=2;

    }else if (c->keypad[3]){
        c->reg[Vx]=3;
        
    }else if (c->keypad[4]){
        c->reg[Vx]=4;

    }else if (c->keypad[5]){
        c->reg[Vx]=5;

    }else if (c->keypad[6]){
        c->reg[Vx]=6;

    }else if (c->keypad[7]){
        c->reg[Vx]=7;

    }else if (c->keypad[8]){
        c->reg[Vx]=8;

    }else if (c->keypad[9]){
        c->reg[Vx]=9;

    }else if (c->keypad[10]){
        c->reg[Vx]=10;

    }else if (c->keypad[11]){
        c->reg[Vx]=11;

    }else if (c->keypad[12]){
        c->reg[Vx]=12;

    }else if (c->keypad[13]){
        c->reg[Vx]=13;

    }else if (c->keypad[14]){
        c->reg[Vx]=14;

    }else if (c->keypad[15])
    {
        c->reg[Vx]=15;
    }else{
        c->pc -=2;
    }

    

}
//Fx15 set DT= Vx
void OP_Fx15(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    c->delay_timer = c->reg[Vx];
}
//Fx18 set ST = Vx
void OP_Fx18(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    c->sound_timer = c->reg[Vx];

}
//Fx1E Add Index, Vx
void OP_Fx1E(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    c->index += c->reg[Vx];

}
//Fx29 load F , Vx
void OP_Fx29(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    uint8_t digit = c->reg[Vx];

    c->index = font_start_adre +(5*digit);
}
//Fx33 store decimal values of Vx in i , I+1 , I=2
void OP_Fx33(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    uint8_t valueVx = c->reg[Vx];

    c->mem[c->index+2]=valueVx % 10;
    valueVx /=10;
    c->mem[c->index+1]=valueVx % 10 ;
    valueVx/=10;
    c->mem[c->index] = valueVx % 10 ;

}
//Fx55 LD reg[V0]->reg[Vx] starting at i 
void OP_Fx55(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;


    for (uint8_t i = 0 ; i<=Vx ; ++i){
        c->reg[i] = c->mem[c->index+i];

    }
}
// Fx65 rad through reg[v0]->reg[Vx] from mem to loc I
void OP_Fx65(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u) >> 8u;
    for (uint8_t i = 0 ; i <= Vx ; ++i){
        c->reg[i]= c->mem[c->index +i];
    }
}











int main() {
    CHIP8 chip8;
    
    load_rom("C:\\Users\\Moath\\Documents\\Projects\\CHIP8 EMU\\Space Intercept [Joseph Weisbecker, 1978].ch8", &chip8);

    return 0;
}