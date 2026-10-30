#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <SDL3/SDL.h>




#define VIDEO_WIDTH  64
#define VIDEO_HEIGHT 32

typedef struct CHIP8 CHIP8;
typedef struct platform platform;


typedef void (*chip8func) (CHIP8 *c);


struct platform{
    SDL_Window *window ;
    SDL_Texture *texture;
    SDL_Renderer *Renderer;
};




struct CHIP8{
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




    //tables for func pointers
    chip8func table[0xF + 1];
    chip8func table0[0xE + 1];
    chip8func table1[0xE + 1];
    chip8func table8[0xE + 1];
    chip8func tableE[0xE + 1];
    chip8func tableF[0x65 + 1];
};




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


//Set all the OpCode Translation func
//00E0:Clear Display

void OP_00E0(CHIP8 *c){
    memset(c ->video_gfx , 0,sizeof(c->video_gfx));

}

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
//CxKK set reg Vx to random byte & NN
void OP_CxKK(CHIP8 *c){
    uint8_t Vx = (c->opcode & 0x0F00u)>>8u;
    uint8_t byte = c->opcode & 0x00FFu;
    c->reg[Vx]=rand_time(c) & byte;
}
//Dxyn display the sprites on the 8 by 8 screen using mem Vx to Vy id 2 pixels collide Vf=colision 
void OP_Dxyn(CHIP8 *c){
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

    //finder setup sub dispatcher
    void Table0(CHIP8 *c){
        c->table0[c->opcode & 0x000Fu](c);
    }
    void Table8(CHIP8 *c){
        c->table8[c->opcode & 0x000Fu](c);
    }
    void TableE(CHIP8 *c){
        c->tableE[c->opcode  & 0x000Fu](c);
    }
    void TableF(CHIP8 *c){
        c->tableF[c->opcode &  0x00FFu](c);
    }
    void OP_NULL(CHIP8 *c){(void)c;}


//FDE Cycle 
void Cycle(CHIP8 *c){
    //Fetch
    c->opcode = (c->mem[c->pc] << 8u ) | c->mem[c->pc+1];
    //inc pc
    c->pc += 2 ;

    //decode 
    c->table[(c->opcode & 0xF000u) >> 12u](c);

    //dec the delay timer
    if (c->delay_timer>0){
        --c->delay_timer;
    }
    //dec sound timer
    if (c->sound_timer>0){
        --c->sound_timer;
    }

}
// init all chip 8 mem values   
void init_chip(CHIP8 *c){
    c->pc = instr_start_adre;


    for (unsigned int  i= 0 ; i < font_sz  ; ++i ){
        c ->mem[font_start_adre + i ] = fontbytes[i];
    };



    c->randnum = (uint8_t)time(NULL);
    if (c->randnum == 0) c->randnum = 1; 

    //pointer table setup 
    c->table[0x0] = Table0; 
    c->table[0x1] =OP_1nnn;
    c->table[0x2] =OP_2nnn;
    c->table[0x3] =OP_3xkk;
    c->table[0x4] =OP_4xkk;
    c->table[0x5] =OP_5xy0;
    c->table[0x6] =OP_6xkk;
    c->table[0x7] =OP_7xkk;
    c->table[0x8] =Table8;
    c->table[0x9] =OP_9xy0;
    c->table[0xA] =OP_ANNN;
    c->table[0xB] =OP_BNNN;
    c->table[0xC] =OP_CxKK;
    c->table[0xD] =OP_Dxyn;
    c->table[0xE] =TableE;
    c->table[0xF] =TableF;

    for (size_t i = 0 ; i <= 0xE ; i++){
        c->table0[i] = OP_NULL;
        c->table8[i] = OP_NULL;
        c->tableE[i] = OP_NULL;
    }



    c->table0[0x0] = OP_00E0;
    c->table0[0xE] = OP_00EE;

    c->table8[0x0] =OP_8xy0;
    c->table8[0x1] =OP_8xy1;
    c->table8[0x2] =OP_8xy2;
    c->table8[0x3] =OP_8xy3;
    c->table8[0x4] =OP_8xy4;
    c->table8[0x5] =OP_8xy5;
    c->table8[0x6] =OP_8xy6;
    c->table8[0x7] =OP_8xy7;
    c->table8[0xE] =OP_8xyE;


    c->tableE[0x1] = OP_ExA1;
    c->tableE[0xE] = OP_Ex9E;


    for (size_t i = 0 ; i <= 0x65 ; i++){
        c->tableF[i]=OP_NULL;
    }


    c->tableF[0x07]=OP_Fx07;
    c->tableF[0x0A]=OP_Fx0A;
    c->tableF[0x15]=OP_Fx15;
    c->tableF[0x18]=OP_Fx18;
    c->tableF[0x1E]=OP_Fx1E;
    c->tableF[0x29]=OP_Fx29;
    c->tableF[0x33]=OP_Fx33;
    c->tableF[0x55]=OP_Fx55;
    c->tableF[0x65]=OP_Fx65;





}



bool Platform_ini(platform *p , const char title , int windoww , int windowh , int textw , int texth){
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
       SDL_Log("SDL init failure : %s", SDL_GetError());
       return false;
    }
    p->window = SDL_CreateWindow(title , windoww , windowh , 0);
    if(!p->window){
         SDL_Log("Window failure : %s", SDL_GetError());
         return false;
    }
    p->Renderer = SDL_CreateRenderer(p->window , NULL);
    if (!p->Renderer){
        SDL_Log("Renderer failure : %s", SDL_GetError());
    }
    p->texture = SDL_CreateTexture(p->Renderer , SDL_PIXELFORMAT_RGBA8888 , SDL_TEXTUREACCESS_STREAMING , textw , texth);
    if(p->texture){
        SDL_Log("Texture loading failure : %s", SDL_GetError());
        return false;
    }


    SDL_SetTextureScaleMode(p->texture , SDL_SCALEMODE_NEAREST);
    return true;




};

void exitcleanplat(platform *p){
    SDL_DestroyTexture(p->texture);
    SDL_DestroyRenderer(p->Renderer);
    SDL_DestroyWindow(p->window);
    SDL_Quit();
}

void update_pt(platform *p ,const void *buffer , int pitch){
    SDL_UpdateTexture(p->texture , NULL , buffer , pitch);
    SDL_RenderClear(p->Renderer);
    SDL_RenderTexture(p->Renderer , p->texture , NULL , NULL);
    SDL_RenderPresent(p->Renderer);
}

