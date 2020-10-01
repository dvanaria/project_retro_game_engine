//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../../14_game/14_game.cpp
// Last Modified: Sun Nov 10, 2019  10:07PM
// LOC: 689
// Filesize: 32938 bytes
//  
// Game: Escape Station Mazeon (Berzerk 2)
//
//     "It's like Berzerk, only better."
//
//         1. No Evil Auto (or reincarnated as a roving auto-mazeon)
//
//         2. Larger, persistent maze (the "station") the player needs to
//            explore.
//
//         3. Game goals introduced: power down the station to open the door
//            to the escape pod, find the key to the escape ship, fly away
//            from Station Mazeon!
//
//         4. HP instead of instant death. Running into walls or getting hit
//            by lazers doesn't kill you, it saps your hp. 
//
//         5. Robots (auto-mazeons) are "deactivated", not destroyed. If you
//            touch one, your hp gains in power and the robot becomes "dead".
//
//         6. Big gameplay change: moving in any 8 directions only points your
//            laser staff in the intended direction. To move you need to hold
//            down a separate button. This is like modern FPS games. You walk
//            with the WASD keys in those modern games, why not old school
//            video games? Having to move before you could aim and shoot was
//            a major annoyance of mine when playing the original Berzerk.
//
//         7. Main weapon (for both the player and the auto-mazeons) is the
//            laser staff. It helps when aiming, or when dodging robot fire.
//
//         8. There are a finite number of robots. There is a special roving
//            auto-mazeon (called Otto) that roams from room to room, and you
//            have to deactivate it to get the access key to the escape pod.
//
//         9. The world is persistent. The doors are one-way and seal behind
//            you as in the original, but the rooms are connected and can be
//            returned to.
//
//         10. Because room generation is randomized, the player may find them
//             selves in an un-winnable game/maze. Because of this, a new item
//             is necessary. A bomb or something. A wall breaker. 
//
//     This game was developed with engine_golf, which was heavily modified
//     during development and so became engine_hotel. The core engine is kept
//     in a separate file, engine_hotel.h.
//
//     Berzerk was developed by Stern Electronics Inc. in 1980. It was created
//     and designed by Alan McNeil, an employee of Universal Research 
//     Laboratories (a division of Stern Electronics at the time). Evil Otto
//     refers to Dave Otto, security chief at McNeil's former employer Dave 
//     Nutting Associates.
//     
//     Easter Eggs / References:
//
//         URL (or Universal Research Laboratories)
//         Alan McNeil
//         Dave Otto (security guard) or roving auto-mazeon.
//         Stern Electronics Inc.
//         Dave Nutting Associates
//
//    Programming Tasks:
//
// (done) 1. Fix spawn points to be inside room.
//               25,45 (upper left limit)
//               428,260 (lower left limit)
//
// (done) 2. Somehow prevent robots from spawning inside walls.
//
// (done) 3. Implement Active, Deactivated, Dead states of robots, they are
//           never removed from arrays.
//
//        4. Graphics: laser staff, make sure there's a center pixel
//
//        5. animate walking sprites
//
//        6. Robot AI (when to move and in what direction and how far)
//
//        7. Have robots reverse direction before walking through a doorway
//           (no leaving the rooms, mazeons!)
//
//        8. Make larger game world with all rooms connected, and outside
//           edges having no exits (except for that all important one)

#include "engine_hotel.h"

// Constants for "map"
const int MAP_ROWS         = 7;
const int MAP_COLS         = 11;
char map[MAP_ROWS][MAP_COLS];
const char VERTICAL_WALL   = '|'; 
const char HORIZONTAL_WALL = '-'; 
const char SPACE           = ' ';
const char PILLAR          = 'o';
const char OPEN_DOOR       = 'x';
const int CHANCE_PASSAGE   = 64;  // percent
void room_generation(void);

// Actual maze will be built from background text cells
COLORS WALL_COLOR          = BLUE;
const int WALL_LENGTH      = 10;
const int MAZE_OFFSET_X    = 2;  // textgrid coordinate
const int MAZE_OFFSET_Y    = 4;
const int DOORWAY_SOUTH = 0;
const int DOORWAY_NORTH = 1;
const int DOORWAY_EAST  = 2;
const int DOORWAY_WEST  = 3;
int doorway_threshold[4];
const int X_COORD = 0;
const int Y_COORD = 1;
void project_map_to_textgrid(void);

// Robots
const int         MAX_ROBOTS_IN_LIST = 8;
struct Sprite*    robot_list[MAX_ROBOTS_IN_LIST];
const int         ACTIVE      = 0;  // robot active 
const int         DEACTIVATED = 1;  // still some juice left!
const int         DEAD        = 2;  // scrap metal only
const int         ROBOT_SPAWN_MIN_X = 45;
const int         ROBOT_SPAWN_MAX_X = 408;
const int         ROBOT_SPAWN_MIN_Y = 65;
const int         ROBOT_SPAWN_MAX_Y = 240;
void damage_robot(struct Sprite* s, int hp_damage);
void reset_robot_AI(struct Sprite* s);

// Player (Green Stick Man)
struct Sprite*    player_sprite = NULL;
int player_spawn_point[4][2]; // [doorway number], [x,y] 
const int         NEUTRAL     = 0; // column indexes of sprite sheet
const int         SHOOT_E     = 1; // _index_col
const int         SHOOT_NE    = 2;
const int         SHOOT_N     = 3;
const int         SHOOT_NW    = 4;
const int         SHOOT_W     = 5;
const int         SHOOT_SW    = 6;
const int         SHOOT_S     = 7;
const int         SHOOT_SE    = 8;
void damage_player(int d);

// Bullets (laser bolts?)
struct Bullet {
    COLORS color;
    SDL_Point one;
    SDL_Point two;
    int dx, dy;
    int life;  // in frames
    int speed;  // number of pixels to move per frame
};
const int BULLET_LIST_SIZE = 200; // max on-screen bullets possible 
const int BULLET_LENGTH = 7;
struct Bullet* bullet_list[BULLET_LIST_SIZE];
void initialize_bullet_list(void);
void add_bullet(int dir);

// Sound effects
const int         MAX_SOUND_EFFECTS_IN_LIST = 4;
Mix_Chunk *       sound_effect_list[MAX_SOUND_EFFECTS_IN_LIST];
const int         MAX_MUSIC_IN_LIST = 1;
Mix_Music *       music_list[MAX_MUSIC_IN_LIST];

// Collision functions
bool overlap_test(struct Sprite* s);
void bounce_object(struct Sprite* s);
int JOLT = 2; // number of pixels to jump on collision



// USER DEFINED FUNCTIONS ////////////////////////////////////////////////////

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

void initialize_sprite(struct Sprite* e) {

    e->dx = 0;
    e->dy = 0;
    e->animation_sheet_cell_rect.x = 0;
    e->animation_sheet_cell_rect.y = 0;
    e->animation_sheet_cell_rect.w = 20;
    e->animation_sheet_cell_rect.h = 36;
    e->ANIMATION_SHEET_NUM_COLUMNS = 9;
    e->ANIMATION_SHEET_NUM_ROWS    = 3;
    e->animation_sheet_index_col = 0;
    e->animation_sheet_index_row = 0;
        
    e->render_target_rect.x = 0; // updated every frame
    e->render_target_rect.y = 0;
    e->render_target_rect.w = 
        e->animation_sheet_cell_rect.w;
    e->render_target_rect.h = 
        e->animation_sheet_cell_rect.h;

    e->WEAPON_RESET = 17;
    e->weapon_cooldown = 0;
}

void user_set_up_graphics(void) {

    text_visible = true;

    background_layer_color = BLACK;
    WALL_COLOR = BLUE;

    room_generation();
    project_map_to_textgrid();

    // create player
    player_sprite = createSprite("../graphics/player.png",
            "../graphics/player_animation_sheet.png");
    initialize_sprite(player_sprite);
    int spawn_location = rand() % 4;
    player_sprite->body_rect.x = player_spawn_point[spawn_location][X_COORD];
    player_sprite->body_rect.y = player_spawn_point[spawn_location][Y_COORD];
    player_sprite->animation_speed = 2;

    // spawn all robots  
    int spawn_range_x = ROBOT_SPAWN_MAX_X - ROBOT_SPAWN_MIN_X; 
    int spawn_range_y = ROBOT_SPAWN_MAX_Y - ROBOT_SPAWN_MIN_Y; 
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {

        robot_list[i] = createSprite(
                "../graphics/robot.png",
               "../graphics/robot_animation_sheet.png");
        initialize_sprite(robot_list[i]);
       
        // respawn until not overlapping any
        do {
            robot_list[i]->body_rect.x = (rand() % spawn_range_x) + ROBOT_SPAWN_MIN_X; 
            robot_list[i]->body_rect.y = (rand() % spawn_range_y) + ROBOT_SPAWN_MIN_Y; 

        } while(overlap_test(robot_list[i]) == true);

        // start robots standing still
        robot_list[i]->dx = 0;
        robot_list[i]->dy = 0;

        reset_robot_AI(robot_list[i]);
    
        robot_list[i]->animation_speed = 4;
    }

    printToTextgrid("BERZERK 2 (ESCAPE FROM STATION MAZEON)", 1, 2);
}

void user_starting_loop(void) {
}

void user_keyboard_handler_key_up(SDL_Keycode kc) {
}

void user_keyboard_handler_key_down(SDL_Keycode kc) {

    int cc;

    switch(kc) {

        case SDLK_1:
            cc = rand() % COLOR_COUNT;
            background_layer_color = static_cast<COLORS>(cc);
            printf(" Background color changed to %s\n", 
                    COLOR_NAME[cc]);
            fflush(stdout);
            Mix_PlayChannel(-1, sound_effect_list[2], 0); //first free channel
        break;
        
        case SDLK_2:
            cc = rand() % COLOR_COUNT;
            WALL_COLOR = static_cast<COLORS>(cc);
            printf(" Wall color changed to %s\n", 
                    COLOR_NAME[WALL_COLOR]);
            fflush(stdout);
            Mix_PlayChannel(-1, sound_effect_list[2], 0); //first free channel
            project_map_to_textgrid();
        break;
        
        default:
        break;
    }
}

void user_create_all_textures(void) {
   
    // load player texture 
    createSpriteTexture(player_sprite,
           "../graphics/player.png",
           "../graphics/player_animation_sheet.png");

    // load robot textures
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        createSpriteTexture(robot_list[i],
                "../graphics/robot.png",
               "../graphics/robot_animation_sheet.png");
    }
   
    // load font sheet texture 
    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");
}

void user_destroy_all_textures(void) {

    destroySpriteTexture(player_sprite);

    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        destroySpriteTexture(robot_list[i]);
    }

    SDL_DestroyTexture(glyph_sheet);
}

void user_gamepad_button_handler(SDL_Event e) {

    if(e.cbutton.button == 2)
        gamepad_button_y = true;

    if(e.cbutton.button == 0) 
        gamepad_button_b = true;

    if(e.cbutton.button == 3) 
        gamepad_button_x = true;

    if(e.cbutton.button == 1)
        gamepad_button_a = true;
}

void user_update_sprites(void) {
   
    // robot AI 
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        robot_list[i]->ai_counter--;
        if(robot_list[i]->ai_counter == robot_list[i]->AI_POINT_1) {
            while(robot_list[i]->dx == 0 && robot_list[i]->dy == 0) {
                if(rand()%2 == 0)
                   robot_list[i]->dx = ((rand()%3) - 1) * 2;
                else
                   robot_list[i]->dy = ((rand()%3) - 1) * 2;
            }
        }
        else if(robot_list[i]->ai_counter == robot_list[i]->AI_POINT_2) {
            robot_list[i]->dx = 0;
            robot_list[i]->dy = 0;
            reset_robot_AI(robot_list[i]);
        }
    }

    // player orientation and movement
    if(gamepad_button_b == true) {

        if(gamepad_dpad_west == true)
            player_sprite->dx = -1;
        if(gamepad_dpad_east == true)
            player_sprite->dx = 1;
        if(gamepad_dpad_north == true)
            player_sprite->dy = -1;
        if(gamepad_dpad_south == true)
            player_sprite->dy = 1;

        if(keyboard_arrow_west == true)
            player_sprite->dx = -1;
        if(keyboard_arrow_east == true)
            player_sprite->dx = 1;
        if(keyboard_arrow_north == true)
            player_sprite->dy = -1;
        if(keyboard_arrow_south == true)
            player_sprite->dy = 1;

    } else {

        player_sprite->dx = 0;
        player_sprite->dy = 0;
 
     /*	
        // change orientation only    
        if(gamepad_dpad_west == true)
        if(gamepad_dpad_east == true)
        if(gamepad_dpad_north == true)
        if(gamepad_dpad_south == true)

        if(keyboard_arrow_west == true)
        if(keyboard_arrow_east == true)
        if(keyboard_arrow_north == true)
        if(keyboard_arrow_south == true)
	*/
    }
  
    if(player_sprite->weapon_cooldown > 0) {
        player_sprite->weapon_cooldown--;
    }

    if(player_sprite->animation_sheet_index_col != NEUTRAL &&
            gamepad_button_y == true) {
        if(player_sprite->weapon_cooldown == 0) {
            add_bullet(player_sprite->animation_sheet_index_col);
            player_sprite->weapon_cooldown = 
                player_sprite->WEAPON_RESET;
        }
    }

    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {

        if(robot_list[i]->state == ACTIVE) {

            if(robot_list[i]->body_rect.x > doorway_threshold[DOORWAY_EAST]) {
                bounce_object(robot_list[i]);
            } else if(robot_list[i]->body_rect.x < doorway_threshold[DOORWAY_WEST]) {
                bounce_object(robot_list[i]);
            } else if(robot_list[i]->body_rect.y > doorway_threshold[DOORWAY_SOUTH]) {
                bounce_object(robot_list[i]);
            } else if(robot_list[i]->body_rect.y < doorway_threshold[DOORWAY_NORTH]) {
                bounce_object(robot_list[i]);
            }
                
            moveSprite(robot_list[i]);
        }
    }
}

void user_collision_detection(void) {

    //Check for collisions, jolt object back if collision occurs and 
    //implement damage

    // check bullet hits
    for(int i = 0; i < BULLET_LIST_SIZE; i++) {

        // wall? 
        if(bullet_list[i] != NULL) {
           
            for(cell_row = 0; cell_row < TEXTGRID_HEIGHT; cell_row++) {
                for(cell_col = 0; cell_col < TEXTGRID_WIDTH; cell_col++) {

                    if(bullet_list[i] != NULL) {
                        if(textgrid_background[cell_row][cell_col] == 
                                WALL_COLOR) {
                            if(SDL_PointInRect(
                                        &bullet_list[i]->one,
                                        &text_rect[cell_row][cell_col])) {
                                free(bullet_list[i]);
                                bullet_list[i] = NULL;
                                break;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
        }
        
        // robot ?
        if(bullet_list[i] != NULL) {

            for(int b = 0; b < MAX_ROBOTS_IN_LIST; b++) {
                if(robot_list[b]->state == ACTIVE) {
                    if(SDL_PointInRect(
                                &bullet_list[i]->one,
                                &robot_list[b]->body_rect)) {
                        robot_list[b]->body_rect.x += (bullet_list[i]->dx % 3);
                        robot_list[b]->body_rect.y += (bullet_list[i]->dy % 3);
                        damage_robot(robot_list[b], 1);
                        robot_list[b]->ai_counter = robot_list[b]->AI_POINT_1+1;
                        free(bullet_list[i]);
                        bullet_list[i] = NULL;
                        break;
                    }
                }
            }
        }

    }
    
    // check every robot
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        if(overlap_test(robot_list[i])) {
            bounce_object(robot_list[i]);    
            //damage_robot(robot_list[i], 1);
            reset_robot_AI(robot_list[i]);
        }
    }

    // check player
    if(overlap_test(player_sprite)) {
        bounce_object(player_sprite);
        //damage_player(1);
    }
}

void handle_sprite_animation(struct Sprite* e) {

    if(e->dx > 0 && e->dy == 0)
        e->animation_sheet_index_col = SHOOT_E;
    else if(e->dx > 0 && e->dy > 0)
        e->animation_sheet_index_col = SHOOT_SE;
    else if(e->dx == 0 && e->dy > 0)
        e->animation_sheet_index_col = SHOOT_S;
    else if(e->dx < 0 && e->dy == 0)
        e->animation_sheet_index_col = SHOOT_W;
    else if(e->dx < 0 && e->dy < 0)
        e->animation_sheet_index_col = SHOOT_NW;
    else if(e->dx == 0 && e->dy < 0)
        e->animation_sheet_index_col = SHOOT_N;
    else if(e->dx > 0 && e->dy < 0)
        e->animation_sheet_index_col = SHOOT_NE;
    else if(e->dx < 0 && e->dy > 0)
        e->animation_sheet_index_col = SHOOT_SW;
    else
        e->animation_sheet_index_col = NEUTRAL;

    if(frame_count % 4 == 0) {
        e->animation_sheet_index_row++;
        if(e->animation_sheet_index_row >= 
                e->ANIMATION_SHEET_NUM_ROWS)
            e->animation_sheet_index_row = 0;
    }

    //Switch to correct animation frame (player)
    e->animation_sheet_cell_rect.x =
        e->animation_sheet_index_col *
        e->animation_sheet_cell_rect.w;
    e->animation_sheet_cell_rect.y =
        e->animation_sheet_index_row *
        e->animation_sheet_cell_rect.h;

    //Correct offset of animation sprite over boundary sprite
    e->render_target_rect.x = e->body_rect.x - 4;
    e->render_target_rect.y = e->body_rect.y - 4;

    //Render animation sprite onto actual player location
    SDL_RenderCopy(window_renderer, 
            e->animation_sheet, 
            &e->animation_sheet_cell_rect,
            &e->render_target_rect);
}

void user_render_graphics(void) {
   
    //Render player 
    handle_sprite_animation(player_sprite);
    
    //Render robots
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        handle_sprite_animation(robot_list[i]);
    }
   
    for(int i = 0; i < BULLET_LIST_SIZE; i++) {
        if(bullet_list[i] != NULL) {
            SDL_SetRenderDrawColor(window_renderer, 
                    r_val[CYAN],
                    g_val[CYAN],
                    b_val[CYAN],
                    0xFF);
            SDL_RenderDrawLine(window_renderer,
                    bullet_list[i]->one.x,
                    bullet_list[i]->one.y,
                    bullet_list[i]->one.x + bullet_list[i]->dx,
                    bullet_list[i]->one.y + bullet_list[i]->dy);
            SDL_RenderDrawLine(window_renderer,
                    bullet_list[i]->two.x,
                    bullet_list[i]->two.y,
                    bullet_list[i]->two.x + bullet_list[i]->dx,
                    bullet_list[i]->two.y + bullet_list[i]->dy);
           
            if(frame_count % bullet_list[i]->speed == 0) { 
                bullet_list[i]->one.x += bullet_list[i]->dx;
                bullet_list[i]->one.y += bullet_list[i]->dy;
                bullet_list[i]->two.x += bullet_list[i]->dx;
                bullet_list[i]->two.y += bullet_list[i]->dy;
            }

            bullet_list[i]->life--;
            if(bullet_list[i]->life < 0) {
                free(bullet_list[i]);
                bullet_list[i] = NULL;
            }
        }
    }

}

void user_ending_loop(void) {

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

    SDL_DestroyTexture(player_sprite->body);
    SDL_DestroyTexture(player_sprite->animation_sheet);
    free(player_sprite);
    
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        SDL_DestroyTexture(robot_list[i]->body);
        free(robot_list[i]);
    }
}

// END OF USER DEFINED FUNCTIONS //////////////////////////////////////////////




void damage_robot(struct Sprite* s, int hp_damage) {

    if(s->state == ACTIVE) {

        s->hp -= hp_damage;

        if(s->hp < 0) {
            s->state = DEACTIVATED;
        }
    }
}

void room_generation(void) {

    //fill array with spaces 
    for(int r = 0; r < MAP_ROWS; r++) {
        for(int c = 0; c < MAP_COLS; c++) {
            map[r][c] = SPACE;
        }
    }

    //install pillars
    for(int r = 0; r < MAP_ROWS; r+=2) {
        for(int c = 0; c < MAP_COLS; c+=2) {
            map[r][c] = PILLAR; 
        }
    }

    //put in some (horizontal) walls
    for(int r = 0; r < MAP_ROWS; r+=2) {
        for(int c = 1; c < MAP_COLS; c+=2) {
            if(map[r][c] == SPACE)
                map[r][c] = HORIZONTAL_WALL; 
        }
    }
    
    //put in some (vertical) walls
    for(int r = 1; r < MAP_ROWS; r+=2) {
        for(int c = 0; c < MAP_COLS; c+=2) {
            if(map[r][c] == SPACE)
                map[r][c] = VERTICAL_WALL; 
        }
    }
    
    //install exits (DOORWAYS)
    map[0]          [MAP_COLS/2]   = OPEN_DOOR;
    map[MAP_ROWS-1] [MAP_COLS/2]   = OPEN_DOOR;
    map[MAP_ROWS/2] [0]            = OPEN_DOOR;
    map[MAP_ROWS/2] [MAP_COLS-1]   = OPEN_DOOR;

    //knock down some walls to make some passages!
    for(int r = 1; r < MAP_ROWS-1; r+=1) {
        for(int c = 1; c < MAP_COLS-1; c+=1) {

            if(map[r][c] == VERTICAL_WALL || map[r][c] == HORIZONTAL_WALL) {
                if((rand() % 100) <= CHANCE_PASSAGE) {
                    map[r][c] = SPACE;
                }
            }
        }
    }

    //knock down isolated pillars 
    for(int r = 1; r < MAP_ROWS-1; r+=1) {
        for(int c = 1; c < MAP_COLS-1; c+=1) {

            int count = 0;

            if(map[r][c] == PILLAR) {

                if(map[r-1][c] == SPACE)
                    count++;
                if(map[r+1][c] == SPACE)
                    count++;
                if(map[r][c-1] == SPACE)
                    count++;
                if(map[r][c+1] == SPACE)
                    count++;

                if(count == 4)
                    map[r][c] = SPACE;
            }
        }
    }

    //print room
    printf("\n");
    for(int r = 0; r < MAP_ROWS; r++) {
        printf("   ");
        for(int c = 0; c < MAP_COLS; c++) {
            printf("%c", map[r][c]);
        }
        printf("\n");
    }
    printf("\n");
    fflush(stdout);
}

void project_map_to_textgrid(void) {

    //projects the "map", a 7 x 11 array, onto the textgrid_background array
   
    //project pillars
    int textgrid_row = MAZE_OFFSET_Y;
    int textgrid_col = MAZE_OFFSET_X;
    for(int r = 0; r < MAP_ROWS; r+=2) {
        
        for(int c = 0; c < MAP_COLS; c+=2) {

            if(map[r][c] == PILLAR) {
                textgrid_background[textgrid_row][textgrid_col] = 
                    WALL_COLOR;
            }

            textgrid_col += (WALL_LENGTH+1);
        }
            
        textgrid_row += (WALL_LENGTH+1);
        textgrid_col = MAZE_OFFSET_X;
    }
    
    //project horizontal walls 
    textgrid_row = MAZE_OFFSET_Y;
    textgrid_col = MAZE_OFFSET_X;
    for(int r = 0; r < MAP_ROWS; r+=2) {
       
        for(int c = 1; c < MAP_COLS-1; c+=2) {

            if(map[r][c] == HORIZONTAL_WALL) {

                for(int x = 0; x <= WALL_LENGTH; x++) {
                
                    textgrid_background[textgrid_row][textgrid_col+x] = 
                        WALL_COLOR;
                }
            }

            textgrid_col += (WALL_LENGTH+1);
        }
            
        textgrid_row += (WALL_LENGTH+1);
        textgrid_col = MAZE_OFFSET_X;
    }
    
    //project vertical walls 
    textgrid_row = MAZE_OFFSET_Y;
    textgrid_col = MAZE_OFFSET_X;
    for(int r = 1; r < MAP_ROWS-1; r+=2) {
       
        for(int c = 0; c < MAP_COLS; c+=2) {

            if(map[r][c] == VERTICAL_WALL) {

                for(int x = 0; x <= WALL_LENGTH; x++) {
                
                    textgrid_background[textgrid_row+x][textgrid_col] = 
                        WALL_COLOR;
                }
            }

            textgrid_col += (WALL_LENGTH+1);
        }
            
        textgrid_row += (WALL_LENGTH+1);
        textgrid_col = MAZE_OFFSET_X;
    }
}



void reset_robot_AI(struct Sprite* s) {

    // set up AI here
    //     robot waits until point 1 is reached
    //     robot decides on a direction to move and sets off
    //     robot walks until point 2 is reached
    //     robot stops

    int ai_seed = 400;
    int start_value = (rand() % ai_seed) + ai_seed;
    int half_value = (int)(start_value / 2) - 5;
    
    s->AI_RESET = start_value; // frames
    s->ai_counter = start_value;

    s->AI_POINT_1 = start_value - (rand() % half_value);
    s->AI_POINT_2 = s->AI_POINT_1 - (rand() % half_value);

    s->dx = 0;
    s->dy = 0;
}

bool overlap_test(struct Sprite* s) {

    // check this sprite against every other in the game, and if it overlaps
    // anything, return true
    bool result = false;

    // check every robot
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        if(robot_list[i] != NULL) {
            if(SDL_HasIntersection(&robot_list[i]->body_rect, &s->body_rect)) {
                if(s->id_number != robot_list[i]->id_number) {
                    result = true;
                }
            }
        }
    }
    
    // check player sprite 
    if(SDL_HasIntersection(&player_sprite->body_rect, &s->body_rect)) {
        if(s->id_number != player_sprite->id_number) {
            result = true;
        }
    }

    // check walls
    for(cell_row = 0; cell_row < TEXTGRID_HEIGHT; cell_row++) {
        for(cell_col = 0; cell_col < TEXTGRID_WIDTH; cell_col++) {
            if(textgrid_background[cell_row][cell_col] == WALL_COLOR) {
                if(SDL_HasIntersection(
                            &s->body_rect, 
                            &text_rect[cell_row][cell_col])) {
                    result = true;
                }
            }
        }
    }

    return result;
}

void damage_player(int d) {

    if(player_sprite->hp > 0) {
        player_sprite->hp--;
    }
}

void bounce_object(struct Sprite* s) {

    s->body_rect.x -= (s->dx*JOLT);
    s->body_rect.y -= (s->dy*JOLT);
    s->dx = -(s->dx);
    s->dy = -(s->dy);
}

void initialize_bullet_list(void) {

    for(int i = 0; i < BULLET_LIST_SIZE; i++) {

        if(bullet_list[i] != NULL)
            free(bullet_list[i]);
        
        bullet_list[i] = NULL;
    }
}

void add_bullet(int dir) {
   
    int new_bullet_index = 0;

    for(int i = 0; i < BULLET_LIST_SIZE; i++) {

        if(bullet_list[i] == NULL) {

            bullet_list[i] = (struct Bullet *)malloc(sizeof(struct Bullet));

            bullet_list[i]->color = YELLOW;
            bullet_list[i]->life = 200;
            bullet_list[i]->one.x = player_sprite->body_rect.x;
            bullet_list[i]->one.y = player_sprite->body_rect.y;
            bullet_list[i]->two.x = player_sprite->body_rect.x;
            bullet_list[i]->two.y = player_sprite->body_rect.y;
            bullet_list[i]->speed = 2;

            new_bullet_index = i;
            break;
        }
    }
    
    if(dir == SHOOT_E) {
        bullet_list[new_bullet_index]->one.x += 17;
        bullet_list[new_bullet_index]->one.y += 8;
        bullet_list[new_bullet_index]->two.x += 17;
        bullet_list[new_bullet_index]->two.y += 9;
        bullet_list[new_bullet_index]->dx = BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = 0;
    } else if(dir == SHOOT_NE) {
        bullet_list[new_bullet_index]->one.x += 16;
        bullet_list[new_bullet_index]->one.y += -3;
        bullet_list[new_bullet_index]->two.x += 16;
        bullet_list[new_bullet_index]->two.y += -4;
        bullet_list[new_bullet_index]->dx = BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = -BULLET_LENGTH;
    } else if(dir == SHOOT_N) {
        bullet_list[new_bullet_index]->one.x += 4;
        bullet_list[new_bullet_index]->one.y += -5;
        bullet_list[new_bullet_index]->two.x += 5;
        bullet_list[new_bullet_index]->two.y += -5;
        bullet_list[new_bullet_index]->dx = 0; 
        bullet_list[new_bullet_index]->dy = -BULLET_LENGTH;
    } else if(dir == SHOOT_NW) {
        bullet_list[new_bullet_index]->one.x += -5;
        bullet_list[new_bullet_index]->one.y += -3;
        bullet_list[new_bullet_index]->two.x += -5;
        bullet_list[new_bullet_index]->two.y += -4;
        bullet_list[new_bullet_index]->dx = -BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = -BULLET_LENGTH;
    } else if(dir == SHOOT_W) {
        bullet_list[new_bullet_index]->one.x += -6;
        bullet_list[new_bullet_index]->one.y += 8;
        bullet_list[new_bullet_index]->two.x += -6;
        bullet_list[new_bullet_index]->two.y += 9;
        bullet_list[new_bullet_index]->dx = -BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = 0;
    } else if(dir == SHOOT_SW) {
        bullet_list[new_bullet_index]->one.x += -3;
        bullet_list[new_bullet_index]->one.y += 19;
        bullet_list[new_bullet_index]->two.x += -4;
        bullet_list[new_bullet_index]->two.y += 19;
        bullet_list[new_bullet_index]->dx = -BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = BULLET_LENGTH;
    } else if(dir == SHOOT_S) {
        bullet_list[new_bullet_index]->one.x += 4;
        bullet_list[new_bullet_index]->one.y += 24;
        bullet_list[new_bullet_index]->two.x += 5;
        bullet_list[new_bullet_index]->two.y += 24;
        bullet_list[new_bullet_index]->dx = 0;
        bullet_list[new_bullet_index]->dy = BULLET_LENGTH;
    } else if(dir == SHOOT_SE) {
        bullet_list[new_bullet_index]->one.x += 14;
        bullet_list[new_bullet_index]->one.y += 19;
        bullet_list[new_bullet_index]->two.x += 15;
        bullet_list[new_bullet_index]->two.y += 19;
        bullet_list[new_bullet_index]->dx = BULLET_LENGTH;
        bullet_list[new_bullet_index]->dy = BULLET_LENGTH;
    }
}



int main(int argc, char* args[]) {

    // initialize robot list
    for(int i = 0; i < MAX_ROBOTS_IN_LIST; i++) {
        robot_list[i] = NULL;
    }

    // setup player spawn points
    player_spawn_point[DOORWAY_SOUTH][X_COORD] = 236;
    player_spawn_point[DOORWAY_SOUTH][Y_COORD] = 262;
    player_spawn_point[DOORWAY_NORTH][X_COORD] = 234;
    player_spawn_point[DOORWAY_NORTH][Y_COORD] = 48;
    player_spawn_point[DOORWAY_WEST ][X_COORD] = 28;
    player_spawn_point[DOORWAY_WEST ][Y_COORD] = 156;
    player_spawn_point[DOORWAY_EAST ][X_COORD] = 444;
    player_spawn_point[DOORWAY_EAST ][Y_COORD] = 156; 

    // setup threshold values to tell when an object is approaching
    // a doorway
    doorway_threshold[DOORWAY_SOUTH] = 279; 
    doorway_threshold[DOORWAY_NORTH] = 35; 
    doorway_threshold[DOORWAY_WEST ] = 20; 
    doorway_threshold[DOORWAY_EAST ] = 448; 

    main_game_loop();

    return 0;
}
