//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../test_program.cpp
// Last Modified: Tue Jun 02, 2020  10:42PM
// LOC: 641
// Filesize: 28826 bytes

// Initial Emulator projects 1 through 10 ////////////////////////////////////
//
// Up to page 1-12 in "Commodore 64 Assembly Language Programming" by 
// Derek Bush and Peter Holmes.
//
// 16 instructions covered so far:
//
//     LDX        LDY        
//     LDXIM      LDYIM
//     STX        STY
//
//     LDAIM
//     ADC
//     STA
//
//     TAX        TAY
//     TXA        TYA
//
//     CLD
//     CLC 
//
//     RTS




// ENGINE CODE (BEGIN)  //////////////////////////////////////////////////////
#include "engine_india.h"

void user_starting_loop(void) {}
void user_keyboard_key_up_handler(SDL_Keycode kc) {}
void user_keyboard_key_down_handler(SDL_Keycode kc) {}
void user_keyboard_alpha_numeric_handler(SDL_Keycode kc) {}
void user_create_all_textures(void) {}
void user_destroy_all_textures(void) {}
void user_gamepad_button_handler(SDL_Event e) {}
void user_update_sprites(void) {}
void user_collision_detection(void) {}
void user_render_graphics(void) {}
void user_ending_loop(void) {}
void user_shutdown(void) {}
// ENGINE CODE (END) /////////////////////////////////////////////////////////




// EMULATOR CODE (BEGIN)  ////////////////////////////////////////////////////
#include <stdio.h>
#include <math.h>     // for pow() function
#include <string.h>   // for strcpy() function
#include <stdlib.h>   // for atoi() function

// Global helpers
char string_store[256];  // for copying literal strings inside functions

// Hardware constants
const int MEMORY_SIZE = pow(2,16);  // 65,536 bytes

// Hardware
typedef struct {
    unsigned char   a;
    unsigned char   x;
    unsigned char   y;
    unsigned short pc; //Program Counter;
    unsigned char   s; //Stack Pointer
    unsigned char   p; //Processor Status Register: N V - B D I Z C
} CPU;

// Operations on Hardware
void initialize_cpu(CPU* c) {
    c->a = 0;
    c->x = 0;
    c->y = 0;
    c->p = 0;
    c->s = 0;
    c->pc = 0;
}

// Functions
void initialize_memory(unsigned char *m);
unsigned long fetch_decode_execute(CPU* c, unsigned char *m);
void print_binary(size_t const size, void const * const ptr);
void print_cpu_register_content(CPU* c); 
void print_memory_disassembled(unsigned char *m, unsigned short start_address);
void format_16bit_int(char *n_string, unsigned char *m, unsigned short s); 
unsigned short assemble_file_into_memory(char* filename, unsigned char* m); 
void test_1(unsigned char m, unsigned char n);
// EMULATOR CODE (END)     ////////////////////////////////////////////////////





int main(int argc, char* argv[]) {

    // ENGINE CODE ///////////////////
    keyboard_cursor_enabled = true; 
    show_spin_cycle = true;
    initialize_engine();
    // END ENGINE CODE ///////////////
    
   
    unsigned long instructions_executed = 0;
    CPU cpu;
    unsigned char memory[MEMORY_SIZE];
    initialize_cpu(&cpu);
    initialize_memory(memory);
    
    // Read in assembly language source code file
    printf("\n ARGUMENT COUNT: %d\n", argc);

    unsigned short s; // starting address in RAM (16-bit address)
    if (argc > 1) {
        printf(" FILE FOUND: %s\n", argv[1]);
        s = assemble_file_into_memory(argv[1], memory);
        printf(" FILE CONTENTS ASSEMBLED AND LOADED INTO RAM AT LOCATION: %d\n", s);
        print_memory_disassembled(memory, s);
    }
    else {
        printf(" NO FILE FOUND (ADD AS ARGUMENT)\n");
        s = 828;
        printf(" DEFAULT PROGRAM LOADED INTO RAM AT LOCATION: %d\n", s);
        memory[s]    = 0xA2;  //LDXIM 1
        memory[s+1]  = 0x01;
        memory[s+2]  = 0x8E;  //STX   900
        memory[s+3]  = 0x84;
        memory[s+4]  = 0x03;
        memory[s+5]  = 0xA9;  //LDAIM 2
        memory[s+6]  = 0x02; 
        memory[s+7]  = 0x6D;  //ADC   900
        memory[s+8]  = 0x84;
        memory[s+9]  = 0x03;
        memory[s+10] = 0x8D;  //STA   901
        memory[s+11] = 0x85;
        memory[s+12] = 0x03;
        memory[s+13] = 0x60;  //RTS
        print_memory_disassembled(memory, s);
    }

    cpu.pc = s; 
    printf("\n CPU pc register (Program Counter) set to %d\n", s);

    ////////////////////////////////////////////////////////////
    printf(" CPU now running Fetch-Decode-Execute cycle...\n");
        instructions_executed = 
                fetch_decode_execute(&cpu, memory);
    printf(" Fetch-Decode-Execute cycle completed. \n");
    printf(" Instructions executed: %d\n", instructions_executed);
    ////////////////////////////////////////////////////////////

    printf("\n FINAL CPU CONTENTS: \n");
    print_cpu_register_content(&cpu);


    // RUN ENGINE ////////////////
    main_game_loop();
    SDL_Quit();
    shutdown_engine();
    //////////////////////////////


    return 0;
}







void initialize_memory(unsigned char *m) {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        m[i] = 0;
    }
}

unsigned long fetch_decode_execute(CPU* c, unsigned char *m) {

    //This function puts the CPU in charge, as it pulls instructions one
    //by one from RAM and executes them (this is a Turing Machine) until it
    //encounters a RET instruction, upon which this function exits by 
    //returning a count of the total number of instructions executed.
    //
    //Arguments:
    //
    //    c -> CPU
    //    m -> RAM

    //decode helpers
    unsigned char   low_byte = 0;
    unsigned char  high_byte = 0;
    unsigned short   address = 0;
    unsigned char temp_value = 0;
    unsigned short  temp_sum = 0;
    unsigned char  num_bytes = 0;   // size of instruction, used to increment 
                                    // program counter

    unsigned long instruction_count = 0;     // to count instructions executed 

    while(1) {

        switch( m[c->pc] ) {

            case 0xAE: //LDX Load the x index register with the contents of
                       //memory (absolute)
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                c->x = m[address];
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->x > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->x == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 3;
                break;
            case 0xA2: //LDXIM Load x register with immediate value (0-255)
                c->x = m[c->pc + 1];
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->x > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->x == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 2;
                break;
            case 0x8E: //STX Store contents of x register at specified memory 
                       //location (using absolute addressing)
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                m[address] = c->x;
                num_bytes = 3;
                break;
            case 0xAC: //LDY Load the y index register with the contents of
                       //memory (absolute)
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                c->y = m[address];
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->y > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->y == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 3;
                break;
            case 0xA0: //LDYIM Load y register with immediate value (0-255)
                c->y = m[c->pc + 1];
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->y > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->y == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 2;
                break;
            case 0x8C: //STY Store contents of y register at specified memory 
                       //location (using absolute addressing)
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                m[address] = c->y;
                num_bytes = 3;
                break;
            case 0xA9: //LDA Load accumulator with immediate value (0-255)
                c->a = m[c->pc + 1];
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->a > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->a == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 2;
                break;
            case 0x6D: //ADC Add to accumulator the contents of specified 
                       //address (absolute), including the carry bit in P
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                temp_sum = m[address] + c->a + (0x01 & c->p);
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if((unsigned char)temp_sum > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update V (Overflow Flag)
                    if(m[address] > 127 && c->a > 127 && (unsigned char)temp_sum < 128) {
                        c->p = (0x40 | c->p); // overflow, SET V
                    } else if(m[address] < 128 && c->a < 128 && (unsigned char)temp_sum > 127) {
                        c->p = (0x40 | c->p); // overflow, SET V
                    } else {
                        c->p = (0xBF & c->p); // no overflow, CLEAR V
                    }
                    // update Z (Zero Flag)
                    if(temp_sum == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                    // update C (Carry Flag)
                    if(temp_sum > 255) {
                        c->p = (0x01 | c->p); // carry, SET C
                    } else {
                        c->p = (0xFE & c->p); // no carry, CLEAR C
                    }
                c->a = (unsigned char)temp_sum;
                num_bytes = 3;
                break;
            case 0x8D: //STA Store contents of accumulator at specified memory 
                       //location using absolute addressing.
                low_byte = m[c->pc + 1];
                high_byte = m[c->pc + 2];
                address = high_byte*256 + low_byte;
                m[address] = c->a;
                num_bytes = 3;
                break;
            case 0xD8: //CLD Clear decimal mode
                c->p = (0xF7 & c->p); 
                num_bytes = 1;
                break;
            case 0x18: //CLC Clear carry flag 
                c->p = (0xFE & c->p); 
                num_bytes = 1;
                break;
            case 0x60: //RTS Return From Subroutine
                c->pc++;
                instruction_count++;
                return instruction_count;
                break;
            case 0xAA: //TAX Transfer contents of A to X index register
                c->x = c->a;
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->x > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->x == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 1;
                break;
            case 0x8A: //TXA Transfer contents of X to A (accumulator) 
                c->a = c->x;
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->a > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->a == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 1;
                break;
            case 0xA8: //TAY Transfer contents of A to Y index register
                c->y = c->a;
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->y > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->y == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 1;
                break;
            case 0x98: //TYA Transfer contents of Y to A (accumulator) 
                c->a = c->y;
                //Update p (Processor Status Register): N V - B D I Z C
                    // update N (Negative Flag)
                    if(c->a > 127) {
                        c->p = (0x80 | c->p); // negative result, SET N
                    } else {
                        c->p = (0x7F & c->p); // not negative, CLEAR N
                    }
                    // update Z (Zero Flag)
                    if(c->a == 0) {
                        c->p = (0x02 | c->p); // zero, SET Z
                    } else {
                        c->p = (0xFD & c->p); // not zero, CLEAR Z
                    }
                num_bytes = 1;
                break;
            default:
                num_bytes = 1;
                break;
        }
                
        c->pc += num_bytes;

        instruction_count++;
    }

    return instruction_count;
}

//Output helpers to help see what's going on inside the machine/////////////////
void print_binary(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;

    for(int i = size-1; i >= 0; i--) {
        
        for(int j = 7; j >= 0; j--) {
            
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
}

void print_cpu_register_content(CPU* c) {

    printf("\n");

    printf("  A:         ");
    print_binary(sizeof(c->a), &c->a);
    printf("   (unsigned: %5d)      (signed: %3d)\n", c->a, (signed char)c->a);

    printf("  X:         ");
    print_binary(sizeof(c->x), &c->x);
    printf("   (unsigned: %5d)      (signed: %3d)\n", c->x, (signed char)c->x);

    printf("  Y:         ");
    print_binary(sizeof(c->y), &c->y);
    printf("   (unsigned: %5d)      (signed: %3d)\n", c->y, (signed char)c->y);
    
    printf(" PC: ");
    print_binary(sizeof(c->pc), &c->pc);
    printf("   (unsigned: %5d)\n", c->pc);

    printf("  S:         ");
    print_binary(sizeof(c->s), &c->s);
    printf("   (unsigned: %5d)\n", c->s);
    
    printf("  P:         ");
    print_binary(sizeof(c->p), &c->p);
    printf("\n             NV-BDIZC\n");
    printf("\n");
}

void print_memory_disassembled(unsigned char *m, unsigned short start_address) {

    // This would be an OS function available at the command line.
    // > MEMORY 4588  

    //decode helpers
    unsigned char low_byte = 0;
    unsigned char high_byte = 0;
    unsigned short address = 0;
    unsigned char temp_value = 0;

    unsigned short end_address = start_address + 100;

    for (int i = start_address; i < end_address; i++) {

        temp_value = m[i];

        switch(temp_value) {

            case 0xAE:
                strcpy(string_store, "LDX");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0xA2:
                strcpy(string_store, "LDXIM");
                temp_value = m[i+1];
                printf("\n RAM %d: %s %d", i, string_store, temp_value);
                i += 1;
                break;
            case 0x8E:
                strcpy(string_store, "STX");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0xAC:
                strcpy(string_store, "LDY");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0xA0:
                strcpy(string_store, "LDYIM");
                temp_value = m[i+1];
                printf("\n RAM %d: %s %d", i, string_store, temp_value);
                i += 1;
                break;
            case 0x8C:
                strcpy(string_store, "STY");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0xA9:
                strcpy(string_store, "LDAIM");
                temp_value = m[i+1];
                printf("\n RAM %d: %s %d", i, string_store, temp_value);
                i += 1;
                break;
            case 0x6D:
                strcpy(string_store, "ADC");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0x8D:
                strcpy(string_store, "STA");
                low_byte = m[i+1];
                high_byte = m[i+2];
                address = high_byte*256 + low_byte;
                printf("\n RAM %d: %s %d", i, string_store, address);
                i += 2;
                break;
            case 0xD8:
                strcpy(string_store, "CLD");
                printf("\n RAM %d: %s", i, string_store);
                break;
            case 0x18:
                strcpy(string_store, "CLC");
                printf("\n RAM %d: %s", i, string_store);
                break;
            case 0x60:
                strcpy(string_store, "RTS");
                printf("\n RAM %d: %s", i, string_store);
                i = end_address;
                break;
            case 0xAA:
                strcpy(string_store, "TAX");
                printf("\n RAM %d: %s", i, string_store);
                break;
            case 0x8A:
                strcpy(string_store, "TXA");
                printf("\n RAM %d: %s", i, string_store);
                break;
            case 0xA8:
                strcpy(string_store, "TAY");
                printf("\n RAM %d: %s", i, string_store);
                break;
            case 0x98:
                strcpy(string_store, "TYA");
                printf("\n RAM %d: %s", i, string_store);
                break;
            default:
                strcpy(string_store, "???");
                printf("\n RAM %d: %s", i, string_store);
        }
    }

    printf("\n");
}

void format_16bit_int(char *n_string, unsigned char *m, 
        unsigned short s) {

    // This function takes a string and a memory address.
    // The string contains a number (range 0 - 65535). 
    // The function converts this to a numerical data type,
    // then splits the number into high-order and low-order
    // bytes (2 bytes total) and stores them at 'address'
    // and 'address+1' (storing first the low order then the
    // high order byte).
    
    int i = atoi(n_string);

    unsigned char high_byte = (int)(i/256);
    unsigned char low_byte = i % 256;

    m[s+0] = low_byte;
    m[s+1] = high_byte;
}

unsigned short assemble_file_into_memory(char* filename, unsigned char* m) {

    //This function is an assembler. It reads a text file that contains assembly
    //instructions, and then translates that to 6502 machine code and puts that
    //code into main memory (along with all that code's data and memory
    //references).

    // Two Psuedo-Op-Codes exist:
    // the first line of any .asm file should be the starting address to
    // load the program at. 
    // The second psuedo code is END, which signals EOF.
    		
    FILE *file_ptr;
    char full_line[1000];
    char* token = NULL;
    unsigned short start = 0;

    file_ptr = fopen(filename, "r");

    // First read starting memory address
    fgets(full_line, 1000, file_ptr);
    start = atoi(full_line);

    // Second, set index to start address
    unsigned short index = start;

    // Loop through file, one full-line at a time
    while (fgets(full_line, 1000, file_ptr) != NULL) {

        token = strtok(full_line, " ");

        // Tokenize current line
        while (token) {
           
            if (strcmp(token, "LDX") == 0) {
                m[index] = 0xAE;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "LDXIM") == 0) {
                m[index] = 0xA2;
                index++;
                token = strtok(NULL, " ");
                m[index] = atoi(token);
                index++;
            }
            else if (strcmp(token, "STX") == 0) {
                m[index] = 0x8E;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "LDY") == 0) {
                m[index] = 0xAC;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "LDYIM") == 0) {
                m[index] = 0xA0;
                index++;
                token = strtok(NULL, " ");
                m[index] = atoi(token);
                index++;
            }
            else if (strcmp(token, "STY") == 0) {
                m[index] = 0x8C;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "LDAIM") == 0) {
                m[index] = 0xA9;
                index++;
                token = strtok(NULL, " ");
                m[index] = atoi(token);
                index++;
            }
            else if (strcmp(token, "ADC") == 0) {
                m[index] = 0x6D;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "STA") == 0) {
                m[index] = 0x8D;
                index++;
                token = strtok(NULL, " ");
                format_16bit_int(token, m, index);
                index += 2;
            }
            else if (strcmp(token, "CLC\n") == 0) {
                m[index] = 0x18;
                index++;
            }
            else if (strcmp(token, "CLD\n") == 0) {
                m[index] = 0xD8;
                index++;
            }
            else if (strcmp(token, "RTS\n") == 0) {
                m[index] = 0x60;
                index++;
            }
            else if (strcmp(token, "END\n") == 0) {
                fclose(file_ptr);
                return start;
            }
            else if (strcmp(token, "TAX\n") == 0) {
                m[index] = 0xAA;
                index++;
            }
            else if (strcmp(token, "TXA\n") == 0) {
                m[index] = 0x8A;
                index++;
            }
            else if (strcmp(token, "TAY\n") == 0) {
                m[index] = 0xA8;
                index++;
            }
            else if (strcmp(token, "TYA\n") == 0) {
                m[index] = 0x98;
                index++;
            }
            else {
                printf("\n Unidentified Op-Code Found in File: %s\n", full_line);
            }
        
            token = strtok(NULL, " ");
        }
    }

    fclose(file_ptr);

    return start;
}

void test_1(unsigned char m, unsigned char n) {

    unsigned char result = m + n;
    printf("M: (unsigned %3d)  (signed %3d)   N: (unsigned %3d)  (signed %3d)    M+N:  (unsigned %3d)   (signed %3d)\n", 
            m, (signed char)m,  n, (signed char)n, result, (signed char)result);
    int flag = (m ^ result) & (n ^ result) & 0x80;
    printf("flag: %d ", flag);
    if(m>127 && n>127 && result<128)
        printf("OVERFLOW\n");
    else if(m<128 && n<128 && result>127)
        printf("OVERFLOW\n");
    else
        printf("\n");
    printf("\n");
}
