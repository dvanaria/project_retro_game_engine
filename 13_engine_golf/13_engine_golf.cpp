//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: 13_engine_golf.cpp
// Last Modified: Fri Oct 18, 2019  01:28AM
// LOC: 237
// Filesize: 10403 bytes
//  

#include "engine_golf.h"

//////////////////////////////////
//USER PROGRAM
//

COLORS temp_color;
int temp_row, temp_col;
SDL_Rect temp_rect;
const int         MAX_SPRITES_IN_LIST = 20;
struct Sprite*    sprite_list[MAX_SPRITES_IN_LIST];
struct Sprite*    background_sprite = NULL;
struct Sprite*    gamepad_sprite = NULL;
struct Sprite*    keyboard_sprite = NULL;
const int         MAX_SOUND_EFFECTS_IN_LIST = 4;
Mix_Chunk *       sound_effect_list[MAX_SOUND_EFFECTS_IN_LIST];
const int         MAX_MUSIC_IN_LIST = 1;
Mix_Music *       music_list[MAX_MUSIC_IN_LIST];

bool user_music_and_sound_files() {

    bool success = true;

    //Load music
    music_list[0] = Mix_LoadMUS("../sound/beat.wav");
    
    //Load sound effects
    sound_effect_list[0] = Mix_LoadWAV("../sound/scratch.wav");
    sound_effect_list[1] = Mix_LoadWAV("../sound/high.wav");
    sound_effect_list[2] = Mix_LoadWAV("../sound/medium.wav");
    sound_effect_list[3] = Mix_LoadWAV("../sound/low.wav");

    for(int i = 0; i < MAX_MUSIC_IN_LIST; i++) {
        if(music_list[i] == NULL) {
            printf("Failed to load sound file %d: SDL_mixer Error: %s\n", 
                i, Mix_GetError());
            success = false;
        }
    }

    for(int i = 0; i < MAX_SOUND_EFFECTS_IN_LIST; i++) {
        if(sound_effect_list[i] == NULL) {
            printf("Failed to load sound file %d: SDL_mixer Error: %s\n", 
                i, Mix_GetError());
            success = false;
        }
    }

    return success;
}

void user_keyboard_handler_key_up(SDL_Keycode kc) {
}

void user_keyboard_handler_key_down(SDL_Keycode kc) {

    int cc;

    switch(kc) {

        case SDLK_m:  // change graphics mode (Windowed vs. Fullscreen)

            if(current_graphics_mode == FULLSCREEN_MODE) {
                current_graphics_mode = WINDOWED_MODE;
            } else {
                current_graphics_mode = FULLSCREEN_MODE;
            }

            destroyAllTextures();
            buildWindowAndRenderer();
            createAllTextures();

        break;

        case SDLK_s:

            if(current_graphics_mode == WINDOWED_MODE) {

                current_scale_factor++;
                if(current_scale_factor > MAX_SCALE_FACTOR)
                    current_scale_factor = 1;

                destroyAllTextures();
                buildWindowAndRenderer();
                createAllTextures();
            }

        break;

        case SDLK_1:
            cc = rand() % COLOR_COUNT;
            background_layer_color = static_cast<COLORS>(cc);
            printf(" Background color changed to %s\n", 
                    COLOR_NAME[cc]);
            fflush(stdout);
            Mix_PlayChannel(-1, sound_effect_list[2], 0); //first free channel
        break;
        
        case SDLK_2:
            if(background_sprite->visible == false) {
                background_sprite->visible = true;
            } else {
                background_sprite->visible = false;
            }
        break;

        case SDLK_3:
            for(int i = 0; i < MAX_SPRITES_IN_LIST; i++) {
                if(sprite_list[i]->visible == false) {
                    sprite_list[i]->visible = true;
                } else {
                    sprite_list[i]->visible = false;
                }
            }
        break;

        case SDLK_4:
            for(int i = 0; i < 100; i++) {
                temp_col = rand() % TEXTGRID_WIDTH;
                temp_row = rand() % TEXTGRID_HEIGHT;
                temp_color = static_cast<COLORS>(rand() % COLOR_COUNT);
                textgrid_background[temp_row][temp_col] = temp_color;
            }
            Mix_PlayChannel(-1, sound_effect_list[3], 0); //first free channel
        break;

        case SDLK_5:
            if(text_visible == true) 
                text_visible = false;
            else
                text_visible = true;
            Mix_PlayChannel(-1, sound_effect_list[1], 0); //first free channel
        break;

        default:
        break;
    }
}

void user_render_graphics(void) {

    //Render SOLID BACKGROUND LAYER
    SDL_SetRenderDrawColor(windowRenderer, 
            r_val[background_layer_color],
            g_val[background_layer_color],
            b_val[background_layer_color],
            0xFF);
    SDL_RenderFillRect(windowRenderer, &game_screen_rect);

    //Render SPRITES 
    if(gamepad_sprite->visible == true) {
        SDL_RenderCopyEx(windowRenderer, 
                gamepad_sprite->texture, 
                NULL,
                &gamepad_sprite->rect,
                gamepad_sprite->angle,  //rotate
                NULL,
                SDL_FLIP_NONE);
    }
    if(keyboard_sprite->visible == true) {
        SDL_RenderCopy(windowRenderer, 
                keyboard_sprite->texture, 
                NULL,
                &keyboard_sprite->rect);
    }
    if(background_sprite->visible == true) {
        SDL_RenderCopy(windowRenderer, 
                background_sprite->texture, 
                NULL,
                &background_sprite->rect);
    }
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++) {
        if(sprite_list[i]->visible == true) {
            SDL_RenderCopy(windowRenderer, 
                    sprite_list[i]->texture, 
                    NULL,
                    &sprite_list[i]->rect);
        }
    }
}

void user_set_up_graphics(void) {

    //set up graphics now
    background_layer_color = BLUE;
    printf(" GRAPHICS: Background color set to %s\n", 
            COLOR_NAME[background_layer_color]);

    //Create sprite objects (only needs to be done once) 
    background_sprite = createSprite("../graphics/background_sprite.bmp");
    gamepad_sprite = createSprite("../graphics/gamepad_sprite.png");
    gamepad_sprite->dx = 0;
    gamepad_sprite->dy = 0;
    keyboard_sprite = createSprite("../graphics/keyboard_sprite.png");
    keyboard_sprite->dx = 0;
    keyboard_sprite->dy = 0;
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        sprite_list[i] = createSprite(
                "../graphics/game_object_sprite.png");
}

void user_move_sprites(void) {

    moveSprite(background_sprite);

    if(gamepad_dpad_west == true)
        moveSprite(gamepad_sprite, -2, 0);
    if(gamepad_dpad_east == true)
        moveSprite(gamepad_sprite,  2, 0);
    if(gamepad_dpad_north == true)
        moveSprite(gamepad_sprite,  0, -2);
    if(gamepad_dpad_south == true)
        moveSprite(gamepad_sprite,  0,  2);

    if(gamepad_button_b == true)
        gamepad_sprite->angle = ((int)gamepad_sprite->angle + 2) % 360;
    if(gamepad_button_y == true)
        gamepad_sprite->angle = ((int)gamepad_sprite->angle - 2) % 360;

    if(keyboard_arrow_west == true)
        moveSprite(keyboard_sprite, -2, 0);
    if(keyboard_arrow_east == true)
        moveSprite(keyboard_sprite,  2, 0);
    if(keyboard_arrow_north == true)
        moveSprite(keyboard_sprite,  0, -2);
    if(keyboard_arrow_south == true)
        moveSprite(keyboard_sprite,  0,  2);

    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        moveSprite(sprite_list[i]);
}

void user_shutdown(void) {

    for(int i = 0; i < MAX_SOUND_EFFECTS_IN_LIST; i++) {
        Mix_FreeChunk(sound_effect_list[i]);
        sound_effect_list[i] = NULL;
    }
    for(int i = 0; i < MAX_MUSIC_IN_LIST; i++) {
        Mix_FreeMusic(music_list[i]);
        music_list[i] = NULL;
    }

    SDL_DestroyTexture(gamepad_sprite->texture);
    SDL_DestroyTexture(keyboard_sprite->texture);
    SDL_DestroyTexture(background_sprite->texture);
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++) 
        SDL_DestroyTexture(sprite_list[i]->texture);

    free(gamepad_sprite);
    free(keyboard_sprite);
    free(background_sprite);
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++) 
        free(sprite_list[i]);
}

void user_create_all_textures(void) {
    
    createSpriteTexture(background_sprite, 
            "../graphics/background_sprite.bmp");
    createSpriteTexture(gamepad_sprite,
           "../graphics/gamepad_sprite.png");
    createSpriteTexture(keyboard_sprite,
            "../graphics/keyboard_sprite.png");
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        createSpriteTexture(sprite_list[i],
                "../graphics/game_object_sprite.png");
    
    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");
}

void user_gamepad_button_handler(SDL_Event e) {

    printf(" >>> SDL_JOYBUTTONDOWN: button %d\n",
            input_event.cbutton.button);

    if(e.cbutton.button == 2)
        gamepad_button_y = true;

    if(e.cbutton.button == 0) 
        gamepad_button_b = true;

    if(e.cbutton.button == 3) {
        if(Mix_PlayingMusic() == 0) {
            //Play the music
            Mix_PlayMusic(music_list[0], -1);
        }
        gamepad_button_x = true;
    }

    if(e.cbutton.button == 1) {
        gamepad_button_a = true;
        Mix_HaltMusic();
    }
}

void user_destroy_all_textures(void) {

    destroySpriteTexture(background_sprite);
    destroySpriteTexture(gamepad_sprite);
    destroySpriteTexture(keyboard_sprite);

    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        destroySpriteTexture(sprite_list[i]);

    SDL_DestroyTexture(glyph_sheet);
}

void user_collision_detection(void) {

    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++) {
        for(int k = i+1; k < MAX_SPRITES_IN_LIST; k++) {
            if(SDL_HasIntersection(
                        &sprite_list[i]->rect, 
                        &sprite_list[k]->rect)) {

                sprite_list[i]->dx = -sprite_list[i]->dx;
                sprite_list[i]->dy = -sprite_list[i]->dy;
                moveSprite(sprite_list[i]);
            }
        }
    }
}
        

int main(int argc, char* args[]) {

    main_game_loop();

    return 0;
}
