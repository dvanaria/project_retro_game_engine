//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../test_program.cpp
// Last Modified: Sat Jan 11, 2020  03:05PM
// LOC: 51
// Filesize: 2130 bytes
//  

#include "engine_hotel.h"

int main(int argc, char* args[]) {
    
    
   
    keyboard_cursor_enabled = false; 

    print_to_textgrid("Press alpha-numeric keys", 3, 10);

    initialize_engine();
    //define_text_window(2,2,5,5);
    draw_text_window_border(RED);

    int x = text_rect[3][7].x;
    int y = text_rect[3][7].y;
    printf(" graphics coordinates of cell 7,3 (x,y): %i, %i\n", x);
    fflush(stdout);
 


    main_game_loop();
    shutdown_engine();

    return 0;
}

void user_starting_loop(void) {}

void user_keyboard_key_up_handler(SDL_Keycode kc) {
    // called on any key press (alpha-numeric or control key)
    printf("user keyboard key up called\n");
    fflush(stdout);
}
void user_keyboard_key_down_handler(SDL_Keycode kc) {
    // called on any key press (alpha-numeric or control key)
    printf("user keyboard key down called\n");
    fflush(stdout);
}

void user_keyboard_alpha_numeric_handler(SDL_Keycode kc) {

    // called when engine game loop receives an SDL_TEXTINPUT event
    printf("user keyboard key alpha/numeric called\n");
    fflush(stdout);

    if(kc == SDLK_a) {

        for(int i = 0; i < 10; i++) {
            print_to_textgrid("A was pressed!",5+i,10);
        }

    } else {

        int rx = rand() % 40;
        int ry = rand() % 40;

        print_to_textgrid("RANDOM", rx, ry);
    }
}

void user_create_all_textures(void) {}
void user_destroy_all_textures(void) {}
void user_gamepad_button_handler(SDL_Event e) {}
void user_update_sprites(void) {}
void user_collision_detection(void) {}
void user_render_graphics(void) {}

void user_ending_loop(void) {
    if(keyboard_control_key_down[LEFT_ARROW] == true) {
        printf("LEFT ARROW\n");
        fflush(stdout);
    }
}

void user_shutdown(void) {}
