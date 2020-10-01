//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../engine_hotel.cpp
// Last Modified: Thu May 21, 2020  09:14AM
// LOC: 1046
// Filesize: 48334 bytes
//  
//
//
// Note on keyboard input: keys like RETURN, SHIFT, BACKSPACE, etc will 
// generate only SDL_KEYDOWN and SDL_KEYUP type events, while alphanumeric
// (aka "printable") keys will additionally generate SDL_TEXTINPUT type events.

#include <ctime>    // to seed random number generator
#include "engine_hotel.h"

//Engine parameters

//Global objects
SDL_Window*         window = NULL;  
Uint32              window_pixel_format;
int                 window_refresh_rate;
SDL_Renderer*       window_renderer = NULL;
SDL_RendererInfo    renderer_info;
SDL_Rect            game_screen_rect;
SDL_Texture*        target_texture;       //for fullscreen rendering
SDL_Rect            target_texture_rect;  //for fullscreen rendering
COLORS              letterbox_color;      //for fullscreen rendering
int                 current_graphics_mode;
int                 current_scale_factor;
bool                keep_main_loop_running = true;
bool                show_spin_cycle = false;

//Engine control functions 
bool build_window_and_renderer(); 

//Arrays to hold 0-255 values for each color
Uint8 r_val[NUM_COLORS];
Uint8 g_val[NUM_COLORS];
Uint8 b_val[NUM_COLORS];

//Array of strings with color names (27 colors)
const int COLOR_NAME_STRING_LENGTH = 30;
char COLOR_NAME[NUM_COLORS][COLOR_NAME_STRING_LENGTH] = {
    "BLACK",
    "DARK BLUE",
    "BLUE",
    "DARK GREEN",
    "GRAY BLUE GREEN",
    "SKY BLUE",
    "GREEN",
    "POND GREEN",
    "CYAN",
    "MAROON",
    "VIOLET",
    "POWDER BLUE",
    "MOSS",
    "GRAY",
    "GRAY BLUE",
    "KERMIT",
    "GRAY GREEN",
    "LIGHT CYAN",
    "RED",
    "PINK",
    "MAGENTA",
    "ORANGE",
    "GRAY RED",
    "LAVENDER",
    "YELLOW",
    "SAND",
    "WHITE" };

//Sprite objects
struct Sprite {
    
    // actual bounding box and location of sprite
    SDL_Texture* body; 
    SDL_Rect     body_rect;  

    // movement vectors (where to move body_rect.x, body_rect.y)
    int          dx;        
    int          dy;

    // orientation (degrees, north = 0, rotation = clockwise)
    double       angle;    // used with SDL_RenderCopyEX()
   
    // animation texture, each cell is slightly larger than bounding box 
    SDL_Texture* animation_sheet;
    int          ANIMATION_SHEET_NUM_COLUMNS; // dimensions of sheet
    int          ANIMATION_SHEET_NUM_ROWS;
    int          animation_sheet_index_col;   // current selection 
    int          animation_sheet_index_row; 
    SDL_Rect     animation_sheet_cell_rect;   
    int          animation_speed;             // larger value = slower animation

    // Where will we render the animation cell on the final screen?
    // This will be based on the body location and offset a bit
    // since the animation cell is often a larger than the body rect.
    SDL_Rect     render_target_rect;          
    
    bool         visible;    // is this a visible sprite on-screen?
    int          id_number;  // unique id_number for this sprite

    int          ai_counter; // decremented each frame
    int          AI_RESET;   // what value to reset counter to
    int          AI_POINT_1; // first trigger point for counter
    int          AI_POINT_2; // second trigger (point 2 < point 1)
    int          AI_POINT_3; // third trigger (point 3 < point 2)

    // More game specific now, all optionally used
    int          state;      // user-defined states (ACTIVE, DEAD, etc)
    int          hp;         // hit points

    // Generic weapon refire rates: establish WEAPON_RESET when you 
    // create the sprite. This is the number of frames to wait until the
    // weapon can be used again after "firing" it. 
    int          WEAPON_RESET;    
    int          weapon_cooldown;  // current cooldown value, approaching 0
                                  
};

// Sprite control functions
int sprite_id_counter; // track total number of sprites
SDL_Texture* create_optimized_texture(const char* filename);
void create_sprite_texture(struct Sprite* s, const char *filename1,
        const char *filename2);
void destroy_sprite_texture(struct Sprite* s);
    
void create_all_textures(void);
void destroy_all_textures(void);



void render_textgrid(void);

//Input system
SDL_Event             input_event;

// gamepad
SDL_GameController*   gamepad;
bool                  gamepad_enabled       = false;
bool                  gamepad_dpad_north    = false;
bool                  gamepad_dpad_west     = false;
bool                  gamepad_dpad_east     = false;
bool                  gamepad_dpad_south    = false;
bool                  gamepad_button_x      = false;
bool                  gamepad_button_y      = false;
bool                  gamepad_button_a      = false;
bool                  gamepad_button_b      = false;
void gamepad_dpad_handler(SDL_Event e);
void gamepad_button_handler(SDL_Event e);


//Sound system (digital sound generation)
SDL_AudioDeviceID   audio_device_id;
SDL_AudioFormat     audio_format = AUDIO_U8; // 0 to 255
Uint16              audio_buffer_size = 2048; // in number of samples
int                 audio_frequency = 44100;
void initialize_audio(void);

//USER DEFINED CALLS (functions that must be implemented in game code) 
void user_starting_loop(void); 
void user_create_all_textures(void);
void user_destroy_all_textures(void);
void user_gamepad_button_handler(SDL_Event e);
void user_update_sprites(void);
void user_collision_detection(void);
void user_render_graphics(void);
void user_ending_loop(void);
void user_shutdown(void); 

// Framerate information
Uint32 loop_start_time = 0;
Uint32 loop_end_time = 0;
Uint32 target_loop_duration = 1000 / DESIRED_FPS; 
Uint32 calculated_loop_duration = 0;
Uint32 cumulative_loop_duration = 0;
Uint32 cumulative_frame_count = 0;
Uint32 spin_cycle = 0;

// Sound effects
Mix_Chunk * sound_effect_list[NUM_SOUND_EFFECTS];
const int   MAX_MUSIC_IN_LIST = 10;
Mix_Music * music_list[MAX_MUSIC_IN_LIST];

// Graphics layers
COLORS background_layer_color;
COLORS textgrid_background[TEXTGRID_HEIGHT][TEXTGRID_WIDTH]; 
char textgrid_foreground[TEXTGRID_HEIGHT][TEXTGRID_WIDTH]; 

// Defined cell locations (rects)
SDL_Rect text_rect[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    






// FINAL (Public facing declarations) /////////////////////////////////////////

void user_keyboard_key_up_handler(SDL_Keycode kc);
void user_keyboard_key_down_handler(SDL_Keycode kc);
void user_keyboard_alpha_numeric_handler(SDL_Keycode kc);

bool keyboard_cursor_enabled = true;  
bool text_foreground_enabled = true;  
bool text_background_enabled = true;  

int keyboard_cursor_x = 0;           
int keyboard_cursor_y = 0;          

const SDL_Keycode KEY_TO_TOGGLE_SCREEN_MODE = SDLK_F1;
const SDL_Keycode KEY_TO_TOGGLE_WINDOW_SIZE = SDLK_F2;
const SDL_Keycode KEY_TO_EXIT_MAIN_LOOP = SDLK_ESCAPE;

bool keyboard_control_key_down[NUM_CONTROL_KEYS];  

int TEXT_WINDOW_LEFT_COLUMN = 0;
int TEXT_WINDOW_RIGHT_COLUMN = TEXTGRID_HEIGHT - 1;
int TEXT_WINDOW_TOP_ROW = 0;
int TEXT_WINDOW_BOTTOM_ROW = TEXTGRID_WIDTH - 1;


// FINAL (Private declarations) ///////////////////////////////////////////////

char*     ENGINE_NAME = "ENGINE HOTEL";
const int FULLSCREEN_MODE = 0;
const int WINDOWED_MODE   = 1;
int       DESKTOP_SCREEN_WIDTH  = 0; //set during initialization
int       DESKTOP_SCREEN_HEIGHT = 0;
int       MAX_SCALE_FACTOR      = 0; //to size game screen in full screen mode

void keyboard_key_down_handler(SDL_Event e);
void keyboard_key_up_handler(SDL_Event e);
void keyboard_alpha_numeric_handler(SDL_Event e);

const int CURSOR_BLINK_RESET = (int)(DESIRED_FPS / 1.25);
const int CURSOR_BLINK_HALF = (int)(CURSOR_BLINK_RESET / 2);
int cursor_blink = CURSOR_BLINK_RESET; 

COLORS cell_color;
int cell_row, cell_col;

SDL_Texture*  glyph_sheet = NULL;
const int     NUM_GLYPHS = 128; //number of cells on master font sheet
SDL_Rect      glyph_rect[NUM_GLYPHS]; 
char          array_index;  //for use during rendering of text array
char          temp_string[256];  //for writing formatted strings
int           string_index;     








//Main loop
int main_game_loop() {
    
    //Loop until user quits.
    bool quit_program = false;
    while(quit_program == false) {

        // Start of logic section 
        loop_start_time = SDL_GetTicks();

        user_starting_loop(); // USER DEFINED CALL

        //Handle user input:
        //This will loop until no further input events are found in the 
        //event queue.
        while(SDL_PollEvent(&input_event) != 0) {

            switch(input_event.type) {

                // KEYBOARD INPUT /////////////////////////
                case SDL_KEYDOWN:
                    keyboard_key_down_handler(input_event); 
                    break;
                
                case SDL_KEYUP:
                    keyboard_key_up_handler(input_event); 
                    break;
                
                case SDL_TEXTINPUT: 
                    keyboard_alpha_numeric_handler(input_event);
                    break;

                // JOYSTICK INPUT /////////////////////////
                case SDL_JOYAXISMOTION:
                    gamepad_dpad_handler(input_event);
                    break;

                case SDL_JOYBUTTONDOWN:
                    gamepad_button_handler(input_event);
                    break;

                case SDL_JOYBUTTONUP:
                    if(input_event.cbutton.button == 2)
                        gamepad_button_y = false;
                    if(input_event.cbutton.button == 0)
                        gamepad_button_b = false;
                    if(input_event.cbutton.button == 3)
                        gamepad_button_x = false;
                    if(input_event.cbutton.button == 1)
                        gamepad_button_a = false;
                    break;
               
                // MOUSE INPUT ///////////////////////////
                case SDL_WINDOWEVENT:
                    if(input_event.window.event == 
                        SDL_WINDOWEVENT_ENTER) {
                        printf(" >>> SDL_WINDOWEVENT (mouse has entered)\n");
                    } else if(input_event.window.event == 
                        SDL_WINDOWEVENT_LEAVE) {
                        printf(" >>> SDL_WINDOWEVENT (mouse has exited)\n");
                    }
                    break;

                case SDL_MOUSEMOTION:
                    //accurate x,y for windowed mode only
                    printf(" >>> SDL_MOUSEMOTION: %d, %d\n",
                           input_event.motion.x / current_scale_factor, 
                           input_event.motion.y / current_scale_factor);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    printf(" >>> SDL_MOUSEBUTTONDOWN: button %d\n",
                            input_event.button.button);
                    break;

                case SDL_MOUSEBUTTONUP:
                    printf(" >>> SDL_MOUSEBUTTONUP: button %d\n",
                            input_event.button.button);
                    break;

                // WINDOW INPUT ///////////////////////////
                case SDL_QUIT: //clicking 'x' button on window
                    keep_main_loop_running = false;
                    break;

                default:
                    printf(" >>> UNKNOWN INPUT %d <<<\n", input_event.type);
                    break;
            } 
            
            fflush(stdout);
        }

        //Check for user wanting to exit main loop 
        if(keep_main_loop_running == false) {
            quit_program = true;
            break;
        }

        //Move sprites, update state, handle AI, etc.
        user_update_sprites(); //USER DEFINED CALL

        //Collision detection
        user_collision_detection(); //USER DEFINED CALL

        //Change rendering targets here for fullscreen mode
        if(current_graphics_mode == FULLSCREEN_MODE)
            SDL_SetRenderTarget(window_renderer, target_texture);

        //Render SOLID BACKGROUND LAYER
        SDL_SetRenderDrawColor(window_renderer, 
                r_val[background_layer_color],
                g_val[background_layer_color],
                b_val[background_layer_color],
                0xFF);
        SDL_RenderFillRect(window_renderer, &game_screen_rect);
        
        //Render graphics
        user_render_graphics(); //USER DEFINED CALL
   
        //Render TEXTGRID BACKGROUND
        if(text_background_enabled == true) {
            for(cell_row = 0; cell_row < TEXTGRID_HEIGHT; cell_row++) {
                for(cell_col = 0; cell_col < TEXTGRID_WIDTH; cell_col++) {
                    if(textgrid_background[cell_row][cell_col] != EMPTY) {

                        cell_color = textgrid_background[cell_row][cell_col];

                        SDL_SetRenderDrawColor(window_renderer, 
                                r_val[cell_color],
                                g_val[cell_color],
                                b_val[cell_color],
                                0xFF);

                        SDL_RenderFillRect(window_renderer, 
                                &text_rect[cell_row][cell_col]);
                    }
                }
            }
        }

        //Render TEXTGRID FOREGROUND (actual text)
        if(text_foreground_enabled == true) {
            render_textgrid();
        }
        
        //Render cursors, if any are enabled
        if(keyboard_cursor_enabled) {

            cursor_blink--;

            if(cursor_blink > CURSOR_BLINK_HALF) {
                SDL_SetRenderDrawColor(window_renderer, 
                        r_val[YELLOW],
                        g_val[YELLOW],
                        b_val[YELLOW],
                        0xFF);

                SDL_RenderFillRect(window_renderer, 
                        &text_rect[keyboard_cursor_y][keyboard_cursor_x]);

            } else if(cursor_blink < 0) {
                cursor_blink = CURSOR_BLINK_RESET;
            }
        } 

        //Render setup
        if(current_graphics_mode == FULLSCREEN_MODE) {
            SDL_SetRenderTarget(window_renderer, NULL);
            SDL_RenderCopy(window_renderer, 
                    target_texture,
                    NULL,
                    &target_texture_rect);
        }

        // render here
        SDL_RenderPresent(window_renderer);
        
        // let user have a chance to do stuff at the end of the game loop
        user_ending_loop(); // USER DEFINED CALL
       
        //Calculate and record loop duration, pause here until
        //desired FPS is reached
        calculated_loop_duration = 0;
        spin_cycle = 0;
        while(calculated_loop_duration < target_loop_duration) {

            loop_end_time = SDL_GetTicks();
            calculated_loop_duration = (loop_end_time - loop_start_time);
            spin_cycle++; // count wasted cycles here

        }

        //Save cumulative info
        cumulative_loop_duration += calculated_loop_duration;
        cumulative_frame_count += 1;

        if(show_spin_cycle == true) {
            printf(" SPIN CYCLES: %d (FRAME: %d)\n", spin_cycle, 
                    cumulative_frame_count);
            fflush(stdout);
        }
    }

    //Cleanup
    shutdown_engine();

    //Report on framerate statistics
    double avg_loop = (double)cumulative_loop_duration / 
        (double)cumulative_frame_count;
    printf(" FRAMERATE: Average game-loop duration: %f ms (%d FPS)\n",
            avg_loop, (int)(1000.0 / 
                round(avg_loop)));
    fflush(stdout);
}

bool add_music_file(const char* filename) {

    bool success = false;

    //Find an open slot to store the music, if any
    for(int i = 0; i < MAX_MUSIC_IN_LIST; i++) {

        if(music_list[i] == NULL) {

            music_list[i] = Mix_LoadMUS(filename);
            
            if(music_list[i] != NULL) {

                success = true;

            } else {

                printf("Failed to load sound file %d: SDL_mixer Error: %s\n", 
                    i, Mix_GetError());
            }
        }
    }

    return success;
}

void move_sprite(struct Sprite* s) {

    //Move sprite by whatever dx,dy are set to
    if(cumulative_frame_count % s->animation_speed == 0) {
        s->body_rect.x += s->dx;
        s->body_rect.y += s->dy;
    }

    //Wrap sprite to other side of screen if off-screen
    if(s->body_rect.x + s->body_rect.w < 0) {
        s->body_rect.x = GAME_SCREEN_WIDTH;
    } else if (s->body_rect.x > GAME_SCREEN_WIDTH) {
        s->body_rect.x = 0 - s->body_rect.w;
    }
    
    if(s->body_rect.y + s->body_rect.h < 0) {
        s->body_rect.y = GAME_SCREEN_HEIGHT;
    } else if (s->body_rect.y > GAME_SCREEN_HEIGHT) {
        s->body_rect.y = 0 - s->body_rect.h;
    }
}

bool initialize_engine() {

    //Return value 
    bool success = true; 
    
    //Standard C++ way of seeding random number generator
    srand(time(NULL)); 
    printf(" INIT ENGINE: Random number generator seeded\n");
    fflush(stdout);
    
    //Calculate each RGB value in the color system.
    int color_index = 0;
    for(int r = 0; r < 0xFF; r += 127) {
        for(int g = 0; g < 0xFF; g += 127) {
            for(int b = 0; b < 0xFF; b += 127) {
                r_val[color_index] = r;
                g_val[color_index] = g;
                b_val[color_index] = b;
                color_index++;
            }
        }
    }
    printf(" INIT ENGINE: RGB values calculated for 27 colors\n");
    fflush(stdout);
    
    //The glyph sheet holds 128 cells arranged in 16 columns and 8 rows.
    for (int i = 0; i < NUM_GLYPHS; i++) {
        glyph_rect[i].x = (i % 16) * FONT_WIDTH;
        glyph_rect[i].y = (i / 16) * FONT_HEIGHT;
        glyph_rect[i].w = FONT_WIDTH;
        glyph_rect[i].h = FONT_HEIGHT;
    }
    printf(" INIT ENGINE: Rects for glyph sheet Texture calculated\n");
    fflush(stdout);
    
    //Pre-calculate all textgrid Rects 
    SDL_Rect text_target = {0, 0, FONT_WIDTH, FONT_HEIGHT};
    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {

            text_target.x = c * FONT_WIDTH;
            text_target.y = r * FONT_HEIGHT;

            text_rect[r][c].x = text_target.x;
            text_rect[r][c].y = text_target.y;
            text_rect[r][c].w = text_target.w;
            text_rect[r][c].h = text_target.h;
        }
    }
    printf(" INIT ENGINE: Rects for textgrid locations calculated\n");
    fflush(stdout);
    
    //Set colors of letterbox/pillarbox 
    letterbox_color = GRAY;
    printf(" INIT ENGINE: Letterbox color set for fullscreen: %s\n",
            COLOR_NAME[GRAY]);
    fflush(stdout);
    
    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO |
                SDL_INIT_AUDIO |
                SDL_INIT_GAMECONTROLLER |
                SDL_INIT_TIMER |
                SDL_INIT_EVENTS) < 0) {
        printf(" SDL could not initialize (SDL_Error: %s)\n", 
                SDL_GetError());
        fflush(stdout);
        success = false;
    }
    printf(" INIT ENGINE: SDL_Init() called\n");
    fflush(stdout);

    //Enable Text Input to handle upper/lowercase keyboard input
    SDL_StartTextInput();
    printf(" INIT ENGINE: SDL_StartTextInput() called\n");
    fflush(stdout);

    //Clear all keyboard control keys in array
    for(int i = 0; i < NUM_CONTROL_KEYS; i++) {
        keyboard_control_key_down[i] = false;
    }
    printf(" INIT ENGINE: cleared array that tracks keyboard control keys\n");
    fflush(stdout);

    //Set graphics mode
    current_graphics_mode = WINDOWED_MODE;  
    printf(" INIT ENGINE: Graphics mode set to WINDOWED\n");
    fflush(stdout);
    
    //Capture native desktop resolution, must be called after SDL_Init()
    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        printf(" SDL_GetDesktopDisplayMode failed: %s", 
                SDL_GetError());
        fflush(stdout);
        return 1;
    }
    Uint32 f = dm.format;
    printf(" INIT ENGINE: Desktop Resolution: %i x %i @ %i Hz\n", 
            dm.w, dm.h, dm.refresh_rate);
    DESKTOP_SCREEN_WIDTH = dm.w;   //setting global parameters here
    DESKTOP_SCREEN_HEIGHT = dm.h;  //setting global parameters here
    window_refresh_rate = dm.refresh_rate; //critical value captured here
    printf(" INIT ENGINE: Desktop Pixel Format: %s, %i (bpp)\n", 
            SDL_GetPixelFormatName(f), 
            SDL_BITSPERPIXEL(f) );
    fflush(stdout);

    //Calculate and set scale factor to maximize screen size
    int scale_x;
    int scale_y;
    scale_x = (int)(DESKTOP_SCREEN_WIDTH / GAME_SCREEN_WIDTH);
    scale_y = (int)(DESKTOP_SCREEN_HEIGHT / GAME_SCREEN_HEIGHT);
    int final_scale = 0;
    if(scale_y > scale_x)
        final_scale = scale_x;
    else
        final_scale = scale_y;
    MAX_SCALE_FACTOR = final_scale;
    printf(" INIT ENGINE: Maximum SCALING FACTOR found to be x%d\n", 
            MAX_SCALE_FACTOR);
    fflush(stdout);
    current_scale_factor = 1; //Make initial window non-scaled 
    printf(" INIT ENGINE: Current scale factor: x%d\n", 
           current_scale_factor); 
    fflush(stdout);
    
    //Setup a Rect object with the window's logical pixel resolution.
    game_screen_rect.x = 0;
    game_screen_rect.y = 0;
    game_screen_rect.w = GAME_SCREEN_WIDTH;
    game_screen_rect.h = GAME_SCREEN_HEIGHT;
    printf(" INIT ENGINE: Game screen logical resolution: %d x %d\n", 
           game_screen_rect.w, game_screen_rect.h); 
    fflush(stdout);
        
    //Use the native desktop resolution to calculate where the game 
    //screen should be located in fullscreen mode. The maximum scaling
    //factor will always be used for fullscreen mode, regardless of how
    //windowed mode is being scaled.
    target_texture_rect.x = 
        ((DESKTOP_SCREEN_WIDTH - 
                   (GAME_SCREEN_WIDTH * MAX_SCALE_FACTOR)) / 2.0) /
                    MAX_SCALE_FACTOR;
    target_texture_rect.y = 
        ((DESKTOP_SCREEN_HEIGHT - 
                   (GAME_SCREEN_HEIGHT * MAX_SCALE_FACTOR)) / 2.0) /
                    MAX_SCALE_FACTOR;
    target_texture_rect.w = GAME_SCREEN_WIDTH;
    target_texture_rect.h = GAME_SCREEN_HEIGHT;
    printf(" INIT ENGINE: Target texture Rect calculated\n");
    fflush(stdout);
    
    //Load support for PNG image formats
    int flags = IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if((initted & flags) != flags) {
        printf(" Failed to init png support (IMG_Init: %s)\n", 
                IMG_GetError());
        fflush(stdout);
        success = false;
    }
    printf(" INIT ENGINE: PNG image format enabled\n");
    fflush(stdout);
    
    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0) {
        printf(" SDL_mixer could not initialize! SDL_mixer Error: %s\n", 
                Mix_GetError());
        success = false;
    }
    printf(" INIT ENGINE: SDL_mixer initialized\n");
    fflush(stdout);

    //Setup gamepad
    gamepad = NULL;
    for(int i = 0; i < SDL_NumJoysticks(); ++i) {
        if(SDL_IsGameController(i)) {
            gamepad = SDL_GameControllerOpen(i);
            if(gamepad) {
                printf(" INIT ENGINE: Gamepad set up and initialized\n");
                gamepad_enabled = true;
                fflush(stdout);
                break;
            } else {
                printf(" INIT ENGINE: Could not find gamepad %i: %s\n", 
                        i, SDL_GetError());
                fflush(stdout);
            }
        }
    }

    //Setup sound (digitial sound synthesis)
    initialize_audio(); 
    printf(" INIT ENGINE: Audio initialized\n");
    fflush(stdout);

    // initialize sound and music lists 
    for(int i = 0; i < NUM_SOUND_EFFECTS; i++) {
        sound_effect_list[i] = NULL;
    }
    for(int i = 0; i < MAX_MUSIC_IN_LIST; i++) {
        music_list[i] = NULL;
    }
    printf(" INIT ENGINE: cleared arrays that hold sound effects and music\n");
    fflush(stdout);
    
    //Create main window, renderer, target textures, etc.
    if(build_window_and_renderer() == false) {
        printf(" Failed build_window_and_renderer() call\n");
        fflush(stdout);
        return 1;
    }
    printf(" INIT ENGINE: Main window and renderer created!\n");
    fflush(stdout);
    
    //Initialize text/font system
    glyph_sheet = create_optimized_texture(
            "../graphics/c64_font.bmp");
    printf(" INIT ENGINE: loaded glyph_sheet optimized texture\n");
    fflush(stdout);
    initialize_textgrid_background_array();
    printf(" INIT ENGINE: cleared textgrid_background array\n");
    fflush(stdout);
    
    //Report what the native pixel format is (main Window)
    window_pixel_format = SDL_GetWindowPixelFormat(window);
    const char* temp = SDL_GetPixelFormatName(window_pixel_format);
    printf(" INIT ENGINE: Window pixel format: %s\n", temp);
    fflush(stdout);

    //Display some information about the renderer (once)
    SDL_GetRenderDriverInfo(0, &renderer_info);
    printf(" INIT ENGINE: Renderer name: %s\n", renderer_info.name);
    fflush(stdout);

    //Set text window
    TEXT_WINDOW_TOP_ROW = 0;
    TEXT_WINDOW_BOTTOM_ROW = TEXTGRID_HEIGHT - 1;
    TEXT_WINDOW_LEFT_COLUMN = 0;
    TEXT_WINDOW_RIGHT_COLUMN = TEXTGRID_WIDTH - 1;
    keyboard_cursor_x = TEXT_WINDOW_LEFT_COLUMN;
    keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
    printf(" INIT ENGINE: Text window set to full-screen\n");
    fflush(stdout);

    //Set up system sounds
    load_sound_effect_wav_file("../sound/cursor_insert.wav", 
            SFX_CURSOR_INSERT);
    load_sound_effect_wav_file("../sound/cursor_control.wav", 
            SFX_CURSOR_CONTROL_KEY);
    load_sound_effect_wav_file("../sound/computer_on.wav", 
            SFX_COMPUTER_ON);
    play_sound_effect(SFX_COMPUTER_ON); 
    printf(" INIT ENGINE: System sounds loaded and initialized\n");
    fflush(stdout);

    printf(" INIT ENGINE: function complete!\n");
    fflush(stdout);

    return success;
}

void initialize_audio(void) {

    int nad;
    const char* audio_device_name;

    nad = SDL_GetNumAudioDevices(0);
    printf(" SOUND: number of audio devices found: %d\n", nad);
    fflush(stdout);

    for(int i = 0; i < nad; i++) {
        audio_device_name = SDL_GetAudioDeviceName(i, 0);
        printf(" SOUND: audio device name: %s\n", audio_device_name);
        fflush(stdout);
    }

    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want)); 
    want.freq = audio_frequency;
    want.format = audio_format;  
    want.channels = 2;
    want.samples = audio_buffer_size; 
    want.callback = NULL; 
    
    audio_device_id = SDL_OpenAudioDevice(
            audio_device_name,
            0, //want a playback device, not a recording device
            &want, 
            &have, 
            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    printf(" SOUND: audio device id: %d\n", audio_device_id);
    fflush(stdout);

    printf(" SOUND: found frequency: %d\n", have.freq);
    printf(" SOUND: found format: %d\n", have.format);
    printf(" SOUND: found channels: %d\n", have.channels);
    printf(" SOUND: found samples: %d\n", have.samples);
    fflush(stdout);
}

void shutdown_engine() {

    user_shutdown(); //USER DEFINED CALL
    
    for(int i = 0; i < NUM_SOUND_EFFECTS; i++) {
        Mix_FreeChunk(sound_effect_list[i]);
        sound_effect_list[i] = NULL;
    }

    SDL_GameControllerClose(gamepad); 
    gamepad = NULL; 

    SDL_StopTextInput(); // paired: SDL_StartTextInput() in initialize_engine

    SDL_DestroyRenderer(window_renderer);
    window_renderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}
 
bool build_window_and_renderer() {
    
    bool success = true; 

    //Cleanup possible existing graphics objects
    if(window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    if(window_renderer != NULL) {
        SDL_DestroyRenderer(window_renderer);
        window_renderer = NULL;
    }
   
    //Create main window 
    if(current_graphics_mode == FULLSCREEN_MODE) {

        window = SDL_CreateWindow(NULL,
                 0,0,0,0, // paramaters ignored when using fullscreen desktop
                 SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    } else if(current_graphics_mode == WINDOWED_MODE) {

        window = SDL_CreateWindow(
                //"SDL2/C++ Game Engine 2019 (HOTEL)",
                NULL,
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 GAME_SCREEN_WIDTH * current_scale_factor,
                 GAME_SCREEN_HEIGHT * current_scale_factor,
                 NULL);
    }
        
    //Create renderer 
    if(window == NULL) {

        printf(" Window could not be created (SDL Error: %s)\n", 
                SDL_GetError());
        fflush(stdout);
        success = false;
        
    } else {
    
        if(current_graphics_mode == FULLSCREEN_MODE) {

            //Create renderer for window
            window_renderer = SDL_CreateRenderer(window, -1, 
                    SDL_RENDERER_PRESENTVSYNC |
                    SDL_RENDERER_ACCELERATED |
                    SDL_RENDERER_TARGETTEXTURE
                    );
    
            //Create target texture that will be used for all fullscreen 
            //rendering. Remember that textures will always be the size of
            //game coordinates (480x360). SDL handles all stretching/scaling
            //internally.
            if(target_texture != NULL)
                SDL_DestroyTexture(target_texture);
            target_texture = SDL_CreateTexture(
                    window_renderer, 
                    window_pixel_format,
		            SDL_TEXTUREACCESS_TARGET, 
                    GAME_SCREEN_WIDTH, 
                    GAME_SCREEN_HEIGHT);    
        
            //Set scaling here
            //This is an important function call here that effects all later 
            //rendering calls
            if(SDL_RenderSetScale(window_renderer, 
                        MAX_SCALE_FACTOR, MAX_SCALE_FACTOR) == 0) {
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }

            //clear entire fullscreen desktop
            SDL_SetRenderDrawColor(window_renderer,
                    r_val[letterbox_color],
                    g_val[letterbox_color],
                    b_val[letterbox_color],
                    0xFF);
            SDL_RenderClear(window_renderer);
            SDL_RenderPresent(window_renderer);
           
        } else if(current_graphics_mode == WINDOWED_MODE) {

            //Create renderer for window
            window_renderer = SDL_CreateRenderer(window, -1, 
                    SDL_RENDERER_PRESENTVSYNC |
                    SDL_RENDERER_ACCELERATED 
                    );
            
            //Set scaling here
            //This is a big function call here that effects all later 
            //rendering calls
            if(SDL_RenderSetScale(window_renderer, 
                        current_scale_factor, current_scale_factor) != 0) {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }
           
            //Make certain to set main window as render target 
            SDL_SetRenderTarget(window_renderer, NULL);
        }
	
        if(window_renderer == NULL) {
                
            printf(" Renderer could not be created (SDL Error: %s)\n", 
                    SDL_GetError());
            fflush(stdout);
            success = false;
        } 
    }

    return success;
}

void destroy_all_textures(void) {
    
    //Textures must be destroyed BEFORE destroying the renderer they are
    //attached to. Otherwise the call to SDL_DestroyTexture() will result
    //in an "invalid texture" error message.
    
    SDL_DestroyTexture(glyph_sheet);

    user_destroy_all_textures(); //USER DEFINED CALL
}

void destroy_sprite_texture(struct Sprite* s) {

    if(s != NULL) {
        SDL_DestroyTexture(s->body);
        SDL_DestroyTexture(s->animation_sheet);
    }
}

void create_all_textures(void) {

    // load font sheet texture 
    glyph_sheet = create_optimized_texture(
            "../graphics/c64_font.bmp");

    user_create_all_textures(); //USER DEFINED CALL
}

void create_sprite_texture(struct Sprite* s, const char *filename1,
       const char *filename2) {

    s->body = create_optimized_texture(filename1);
    s->animation_sheet = create_optimized_texture(filename2);
}

struct Sprite* create_sprite(const char *filename1, 
        const char* filename2) {

    struct Sprite* s = (struct Sprite *)malloc(sizeof(struct Sprite));

    s->body = create_optimized_texture(filename1);
    
    s->body_rect.x = rand() % GAME_SCREEN_WIDTH;  //random location
    s->body_rect.y = rand() % GAME_SCREEN_HEIGHT;

    SDL_QueryTexture(s->body,
            NULL, NULL, 
            &s->body_rect.w,
            &s->body_rect.h); // bounding box used for collision testing

    s->dx = (rand() % 8) - 4;  //random velocity
    s->dy = (rand() % 8) - 4;

    s->visible = true;

    s->hp = 10;

    s->id_number = sprite_id_counter;
    sprite_id_counter++;
    
    s->animation_sheet = create_optimized_texture(filename2);

    return s;
}

SDL_Texture* create_optimized_texture(const char* filename) {

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(filename);

    if(loadedSurface == NULL) {
            
        printf(" Unable to load image %s! SDL_image Error: %s\n", 
               filename, IMG_GetError() );
        fflush(stdout);
        
    } else {
    
        //Set what color to be "transparent" 
        SDL_SetColorKey(loadedSurface, 
                SDL_TRUE, 
                SDL_MapRGB(loadedSurface->format, 10, 10, 10));
            
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(
                window_renderer, loadedSurface);
            
        if(newTexture == NULL) {
                
            printf(" Unable to create texture from %s! SDL Error: %s\n", 
                   filename, SDL_GetError());
            fflush(stdout);
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void render_textgrid(void) {

    //Iterates through entire text[][] array and if the character is not a
    //' ' character, uses the glyph Texture to render onto the main screen. 

    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
       
            array_index = textgrid_foreground[r][c]; // 0-127 

            if(array_index != ' ') {
                SDL_RenderCopy(window_renderer, glyph_sheet, 
                        &glyph_rect[array_index], 
                        &text_rect[r][c]); 
            }
        }
    }
}


void play_sound_effect(int i) {

    if(i >= 0 && i < NUM_SOUND_EFFECTS) {
        Mix_PlayChannel(-1, sound_effect_list[i], 0);
    }
}

void load_sound_effect_wav_file(const char *filename, int i) {

    if(i >= 0 && i < NUM_SOUND_EFFECTS) {

        if(sound_effect_list[i] != NULL) {
            Mix_FreeChunk(sound_effect_list[i]);
            sound_effect_list[i] = NULL;
        }

        sound_effect_list[i] = Mix_LoadWAV(filename);
            
        if(sound_effect_list[i] == NULL) {
            printf("Failed to load sound file %d: SDL_mixer Error: %s\n", 
                    i, Mix_GetError());
            fflush(stdout);
        }
    }
}

void move_sprite(struct Sprite* s, int dx, int dy) {

    // Move sprite by supplied dx and dy, not by the sprite's internal dx,dy
    s->body_rect.x += dx;
    s->body_rect.y += dy;

    if(s->body_rect.x + s->body_rect.w < 0) {
        s->body_rect.x = GAME_SCREEN_WIDTH;
    } else if (s->body_rect.x > GAME_SCREEN_WIDTH) {
        s->body_rect.x = 0 - s->body_rect.w;
    }
    
    if(s->body_rect.y + s->body_rect.h < 0) {
        s->body_rect.y = GAME_SCREEN_HEIGHT;
    } else if (s->body_rect.y > GAME_SCREEN_HEIGHT) {
        s->body_rect.y = 0 - s->body_rect.h;
    }
}

void gamepad_button_handler(SDL_Event e) {

    user_gamepad_button_handler(e); //USER DEFINED CALL
}

void gamepad_dpad_handler(SDL_Event e) {
                   
    //left-right is axis 0 (value = + to the right)
    //up-down is axis 1 (value = + is downward)
        
    if(e.caxis.value == 0) { //dpad released
        
        if(e.caxis.axis == 0) {  // x-axis dpad
            gamepad_dpad_west = false;
            gamepad_dpad_east = false;
        } else if(e.caxis.axis == 1) { //y-axis dpad
            gamepad_dpad_north = false;
            gamepad_dpad_south = false;
        }

    } else {

        if(e.caxis.axis == 0) {  // x-axis dpad

            if(e.caxis.value < 0) {
                gamepad_dpad_west = true;
            } else if(e.caxis.value > 0) {
                gamepad_dpad_east = true;
            }

        } else if(e.caxis.axis == 1) { //y-axis dpad

            if(e.caxis.value < 0) {
                gamepad_dpad_north = true;
            } else if(e.caxis.value > 0) {
                gamepad_dpad_south = true;
            }
        }
    }
}

char *get_color_name(int i) { 
    return COLOR_NAME[i]; 
};

void draw_text_window_border(COLORS color) {

    int x1 = TEXT_WINDOW_LEFT_COLUMN;
    int y1 = TEXT_WINDOW_TOP_ROW;
    int x2 = TEXT_WINDOW_RIGHT_COLUMN;
    int y2 = TEXT_WINDOW_BOTTOM_ROW;

    if(x1 <= 0)
        x1 = 1;
    if(y1 <= 0)
        y1 = 1;
    if(x2 >= TEXTGRID_WIDTH-1)
        x2 = TEXTGRID_WIDTH - 2;
    if(y2 >= TEXTGRID_HEIGHT-1)
        y2 = TEXTGRID_HEIGHT - 2;

    for(int r = y1-1; r <= y2+1; r++) {
        textgrid_background[r][x2+1] = color;
        textgrid_background[r][x1-1] = color;
    }
    
    for(int c = x1-1; c <= x2+1; c++) {
        textgrid_background[y1-1][c] = color;
        textgrid_background[y2+1][c] = color;
    }
}

// FINAL (PUBLIC) ////////////////////////////////////////////////////////////

void print_to_textgrid(char* string, int r, int c) {

    // like printf(), but prints to the textgrid instead of stdout.

    if(c < TEXT_WINDOW_LEFT_COLUMN)
        c = TEXT_WINDOW_LEFT_COLUMN;
    if(c > TEXT_WINDOW_RIGHT_COLUMN)
        c = TEXT_WINDOW_RIGHT_COLUMN;
    if(r < TEXT_WINDOW_TOP_ROW)
        r = TEXT_WINDOW_TOP_ROW;
    if(r > TEXT_WINDOW_BOTTOM_ROW)
        r = TEXT_WINDOW_BOTTOM_ROW;

    string_index = 0;

    while(string[string_index] != 0) {
    
        textgrid_foreground[r][c] = string[string_index];
        c++;
        string_index++;

        // wrap text
        if(c > TEXT_WINDOW_RIGHT_COLUMN) {

            c = TEXT_WINDOW_LEFT_COLUMN;
            r++;
            if(r > TEXT_WINDOW_BOTTOM_ROW) {
                r = TEXT_WINDOW_TOP_ROW;
            }

        }
    }
}

void clear_entire_textgrid(void) {
    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
            textgrid_foreground[r][c] = ' ';
        }
    }
}

void clear_textgrid_row(int r) {
    for (int c = 0; c < TEXTGRID_WIDTH; c++) {
        textgrid_foreground[r][c] = ' ';
    }
}

void initialize_textgrid_background_array() {
    for(int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for(int c = 0; c < TEXTGRID_WIDTH; c++) {
            textgrid_background[r][c] = EMPTY;
        }
    }
}

void define_text_window(int x1, int y1, int x2, int y2) {

    if(x1 < 0)
        x1 = 0;

    if(y1 < 0)
        y1 = 0;

    if(x2 > TEXTGRID_WIDTH)
        x2 = TEXTGRID_WIDTH;

    if(y2 > TEXTGRID_HEIGHT)
        y2 = TEXTGRID_HEIGHT;

    TEXT_WINDOW_LEFT_COLUMN = x1;
    TEXT_WINDOW_RIGHT_COLUMN = x2;
    TEXT_WINDOW_TOP_ROW = y1;
    TEXT_WINDOW_BOTTOM_ROW = y2;
    keyboard_cursor_x = TEXT_WINDOW_LEFT_COLUMN;
    keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
}


// FINAL (PRIVATE) ///////////////////////////////////////////////////////////

void keyboard_key_up_handler(SDL_Event e) {

    SDL_Keycode kc = e.key.keysym.sym; 

    switch(kc) {

        case SDLK_ESCAPE:
            keyboard_control_key_down[ESCAPE] = false;
        break;
       
        case SDLK_BACKSPACE:
            keyboard_control_key_down[BACKSPACE] = false;
        break;

        case SDLK_UP:
            keyboard_control_key_down[UP_ARROW] = false;
        break;

        case SDLK_DOWN:
            keyboard_control_key_down[DOWN_ARROW] = false;
        break;
        
        case SDLK_LEFT:
            keyboard_control_key_down[LEFT_ARROW] = false;
        break;
        
        case SDLK_RIGHT:
            keyboard_control_key_down[RIGHT_ARROW] = false;
        break;
        
        case SDLK_RETURN:
            keyboard_control_key_down[RETURN] = false;
        
        default:
        break;
    }
   
    // always let user have a crack at it    
    user_keyboard_key_up_handler(kc); //USER DEFINED CALL
}

void keyboard_key_down_handler(SDL_Event e) {

    SDL_Keycode kc =  e.key.keysym.sym; 

    switch(kc) {

        case KEY_TO_EXIT_MAIN_LOOP:

            keyboard_control_key_down[KEY_TO_EXIT_MAIN_LOOP] = true;
            keep_main_loop_running = false;

        break;
        
        case SDLK_BACKSPACE:

            keyboard_control_key_down[BACKSPACE] = true;

            if(keyboard_cursor_enabled) {
                keyboard_cursor_x--;
                if(keyboard_cursor_x < TEXT_WINDOW_LEFT_COLUMN) {
                    keyboard_cursor_x = TEXT_WINDOW_RIGHT_COLUMN;
                    keyboard_cursor_y--;
                }
                if(keyboard_cursor_y < TEXT_WINDOW_TOP_ROW) {
                    keyboard_cursor_y = 
                        TEXT_WINDOW_BOTTOM_ROW;
                }
                print_to_textgrid(" ", keyboard_cursor_y, keyboard_cursor_x);
                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;
                    
        case SDLK_UP:

            keyboard_control_key_down[UP_ARROW] = true;

            if(keyboard_cursor_enabled) {
                keyboard_cursor_y--;
                if(keyboard_cursor_y < TEXT_WINDOW_TOP_ROW) {
                    keyboard_cursor_y = TEXT_WINDOW_BOTTOM_ROW;
                }
                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;

        case SDLK_DOWN:

            keyboard_control_key_down[DOWN_ARROW] = true;

            if(keyboard_cursor_enabled) {
                keyboard_cursor_y++;
                if(keyboard_cursor_y > TEXT_WINDOW_BOTTOM_ROW) {
                    keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
                }
                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;
        
        case SDLK_LEFT:

            keyboard_control_key_down[LEFT_ARROW] = true;

            if(keyboard_cursor_enabled) {
                keyboard_cursor_x--;
                if(keyboard_cursor_x < TEXT_WINDOW_LEFT_COLUMN) {
                    keyboard_cursor_x = TEXT_WINDOW_RIGHT_COLUMN;
                    keyboard_cursor_y--;
                }
                if(keyboard_cursor_y < TEXT_WINDOW_TOP_ROW) {
                    keyboard_cursor_y = TEXT_WINDOW_BOTTOM_ROW;
                }
                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;
        
        case SDLK_RIGHT:

            keyboard_control_key_down[RIGHT_ARROW] = true;

            if(keyboard_cursor_enabled) {

                keyboard_cursor_x++;
                if(keyboard_cursor_x > TEXT_WINDOW_RIGHT_COLUMN) {
                    keyboard_cursor_x = TEXT_WINDOW_LEFT_COLUMN;
                    keyboard_cursor_y++;
                }
                if(keyboard_cursor_y > TEXT_WINDOW_BOTTOM_ROW) {
                    keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
                }
                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;
        
        case SDLK_RETURN:

            keyboard_control_key_down[RETURN] = true;

            if(keyboard_cursor_enabled) {

                keyboard_cursor_x = TEXT_WINDOW_LEFT_COLUMN;
                keyboard_cursor_y++;
                if(keyboard_cursor_y > TEXT_WINDOW_BOTTOM_ROW) {
                    keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
                }

                play_sound_effect(SFX_CURSOR_CONTROL_KEY);
                cursor_blink = CURSOR_BLINK_RESET;
            }

        break;
        
        case KEY_TO_TOGGLE_SCREEN_MODE:

            if(current_graphics_mode == FULLSCREEN_MODE) {
                current_graphics_mode = WINDOWED_MODE;
            } else {
                current_graphics_mode = FULLSCREEN_MODE;
            }

            destroy_all_textures();
            build_window_and_renderer();
            create_all_textures();

        break;

        case KEY_TO_TOGGLE_WINDOW_SIZE:

            if(current_graphics_mode == WINDOWED_MODE) {

                current_scale_factor++;
                if(current_scale_factor > MAX_SCALE_FACTOR)
                    current_scale_factor = 1;

                destroy_all_textures();
                build_window_and_renderer();
                create_all_textures();
            }

        break;

        default:
        break;
    }
    
    // always let user have a crack at it    
    user_keyboard_key_down_handler(kc); //USER DEFINED CALL
}

void keyboard_alpha_numeric_handler(SDL_Event e) {
    
    SDL_Keycode kc =  e.key.keysym.sym; 

    if(keyboard_cursor_enabled) {

        print_to_textgrid(e.text.text, keyboard_cursor_y, keyboard_cursor_x);
        play_sound_effect(SFX_CURSOR_INSERT);

        keyboard_cursor_x++;
        if(keyboard_cursor_x > TEXT_WINDOW_RIGHT_COLUMN) {
            keyboard_cursor_x = TEXT_WINDOW_LEFT_COLUMN;
            keyboard_cursor_y++;
            if(keyboard_cursor_y > TEXT_WINDOW_BOTTOM_ROW) {
                keyboard_cursor_y = TEXT_WINDOW_TOP_ROW;
            }
        }

        cursor_blink = CURSOR_BLINK_RESET;
    }
    
    // always let user have a crack at it    
    user_keyboard_alpha_numeric_handler(kc); //USER DEFINED CALL
}
