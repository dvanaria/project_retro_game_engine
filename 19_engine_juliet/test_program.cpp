#include "engine_juliet.h"

void user_starting_loop(void) {}
void user_create_all_textures(void) {}
void user_destroy_all_textures(void) {}
void user_mouse_motion_handler(Sint32 x, Sint32 y) {}
void user_keyboard_alpha_numeric_handler(SDL_Keycode kc) {}
void user_update_sprites(void) {}
void user_collision_detection(void) {}
void user_render_graphics(void) {}
void user_ending_loop(void) {}
void user_shutdown(void) {}



void game_action_a(int action) {
    printf("!! game action a : %d\n", action);
}
void game_action_b(int action) {
    printf("!! game action b : %d\n", action);
}
void game_action_c(int action) {
    printf("!! game action c : %d\n", action);
}



int main(int argc, char* argv[]) {

        printf("The name of color #4 is %s\n", get_color_name(4));
        printf("RED has the following enumeration value: %d\n", RED);

    initialize_engine();

        // set up specific in-game controls (configurable!)
        action_pointer_gamepad[SDL_CONTROLLER_BUTTON_A] = game_action_a;
        action_pointer_mouse[SDL_BUTTON_LEFT] = game_action_b;
        action_pointer_keyboard[SDL_SCANCODE_C] = game_action_c;

        keyboard_enabled = true;
        mouse_enabled = true;
        gamepad_enabled = true;

    main_game_loop();
    shutdown_engine();

    return 0;
}
