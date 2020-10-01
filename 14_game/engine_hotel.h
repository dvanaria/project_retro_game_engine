//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../engine_hotel.h
// Last Modified: Mon Nov 04, 2019  02:41AM
// LOC: 884
// Filesize: 38998 bytes
  
#ifndef ENGINE_HOTEL
#define ENGINE_HOTEL

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <ctime>    // to seed random number generator
#include <cstdlib>  // for rand() and srand() functions
#include <math.h>   // for round()

//Engine parameters
const int GAME_SCREEN_WIDTH  = 480;    
const int GAME_SCREEN_HEIGHT = 360;
int       DESKTOP_SCREEN_WIDTH  = 0; //set during initialization
int       DESKTOP_SCREEN_HEIGHT = 0;
int       MAX_SCALE_FACTOR      = 0; //always a whole-number value
const int FONT_WIDTH  = 8;
const int FONT_HEIGHT = 8;
const int TEXTGRID_WIDTH  = GAME_SCREEN_WIDTH / FONT_WIDTH;   //60 columns
const int TEXTGRID_HEIGHT = GAME_SCREEN_HEIGHT / FONT_HEIGHT; //45 rows
const int FULLSCREEN_MODE = 0;
const int WINDOWED_MODE   = 1;
char*     ENGINE_NAME = "ENGINE ECHO";

//Color system (27 colors)
enum COLORS {
    BLACK = 0,
    DARK_BLUE,
    BLUE,
    DARK_GREEN,
    GRAY_BLUE_GREEN,
    SKY_BLUE,
    GREEN,
    POND_GREEN,
    CYAN,
    MAROON,
    VIOLET,
    POWDER_BLUE,
    MOSS,
    GRAY,
    GRAY_BLUE,
    KERMIT,
    GRAY_GREEN,
    LIGHT_CYAN,
    RED,
    PINK,
    MAGENTA,
    ORANGE,
    GRAY_RED,
    LAVENDER,
    YELLOW,
    SAND,
    WHITE,
    COLOR_COUNT,
    EMPTY 
};
Uint8 r_val[COLOR_COUNT];
Uint8 g_val[COLOR_COUNT];
Uint8 b_val[COLOR_COUNT];

//Array of strings with color names (27 colors)
char COLOR_NAME[27][20] = {
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
bool                keep_program_running = true;
int                 sprite_id_counter = 0; // track total number of sprites

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

// Framerate information
Uint32 render_start_time;
Uint32 render_end_time;
Uint32 render_duration;
Uint32 cumulative_render_duration;
Uint32 logic_start_time;
Uint32 logic_end_time;   
Uint32 logic_duration;
Uint32 cumulative_logic_duration;
Uint32 frame_count = 0;

// Graphics layers

    //SOLID BACKGROUND LAYER
    COLORS background_layer_color;

    //SPRITES
    SDL_Texture* createOptimizedTextureFromImageFile(const char* filename);
    struct Sprite* createSprite(const char *filename1, const char* filename2);
    void createAllTextures(void);
    void createSpriteTexture(struct Sprite* s, const char *filename1,
            const char *filename2);
    void destroyAllTextures(void);
    void destroySpriteTexture(struct Sprite* s);
    void moveSprite(struct Sprite* s); //auto-move by s->dx
    void moveSprite(struct Sprite* s, int dx, int dy); //manual-move

    //TEXTGRID BACKGROUND (SOLID COLOR BLOCK) 
    COLORS        textgrid_background[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    COLORS        cell_color;
    int           cell_row, cell_col;
    void initializeTextgridBackgroundArray();

    //TEXTGRID FOREGROUND (WHITE TEXT)
    char          text[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];  //ASCII data per cell
    SDL_Rect      text_rect[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    SDL_Texture*  glyph_sheet = NULL;
    const int     NUM_GLYPHS = 128; //number of cells on master font sheet
    SDL_Rect      glyph_rect[NUM_GLYPHS]; 
    void renderTextgrid(void);
    void clearTextgrid(void);
    void clearTextgridRow(int r);
    void printToTextgrid(char* string, int r, int c);
    char          array_index;  //for use during rendering of text array
    char          temp_string[256];  //for writing formatted strings
    int           string_index;     
    bool          text_visible = false;

//Engine control functions 
bool initializeEngine();
void shutdownEngine();
bool buildWindowAndRenderer(); 

//Input system
SDL_Event             input_event;
bool processInput(void);                                        

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

    // keyboard
    bool                  keyboard_arrow_north  = false;
    bool                  keyboard_arrow_south  = false;
    bool                  keyboard_arrow_west   = false;
    bool                  keyboard_arrow_east   = false;
    void keyboard_key_down_handler(SDL_Keycode kc);
    void keyboard_key_up_handler(SDL_Keycode kc);

//Sound system (digital sound generation)
SDL_AudioDeviceID   audio_device_id;
SDL_AudioFormat     audio_format = AUDIO_U8; // 0 to 255
Uint16              audio_buffer_size = 2048; // in number of samples
int                 audio_frequency = 44100;
void initialize_audio(void);

//Sound system (SDL_mixer system)
bool loadMusicAndSoundFiles();

//USER DEFINED CALLS (functions that must be implemented in game code) 
bool user_music_and_sound_files(void);
void user_set_up_graphics(void);
    void user_starting_loop(void); 
    void user_keyboard_handler_key_up(SDL_Keycode kc);
    void user_keyboard_handler_key_down(SDL_Keycode kc);
        void user_create_all_textures(void);
        void user_destroy_all_textures(void);
    void user_gamepad_button_handler(SDL_Event e);
    void user_update_sprites(void);
    void user_collision_detection(void);
    void user_render_graphics(void);
    void user_ending_loop(void);
void user_shutdown(void); 




//Main loop
int main_game_loop() {

    //Set everything that's independent of rendering: calls SDL_Init(),
    //figures out native desktop screen resolution, calculates scaling 
    //factor, initializes controllers and sound.
    if(initializeEngine() == false) {
        printf(" Failed initializeEngine() call\n");
        fflush(stdout);
        return 1;
    }

    //Create main window, renderer, target textures, etc.
    if(buildWindowAndRenderer() == false) {
        printf(" Failed buildWindowAndRenderer() call\n");
        fflush(stdout);
        return 1;
    }

    //Report what the native pixel format is (main Window)
    window_pixel_format = SDL_GetWindowPixelFormat(window);
    const char* temp = SDL_GetPixelFormatName(window_pixel_format);
    printf(" GRAPHICS: SDL_Window created with pixel format %s\n", temp);
    fflush(stdout);

    //Display some information about the renderer (once)
    SDL_GetRenderDriverInfo(0, &renderer_info);
    printf(" GRAPHICS: Renderer name: %s\n", renderer_info.name);
    fflush(stdout);

    //Initialize text/font system
    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");
    initializeTextgridBackgroundArray();
    
    //Set up initial graphics here
    user_set_up_graphics(); //USER DEFINED CALL

    //Loop until user quits.
    bool quit_program = false;
    while(quit_program == false) {

        // Start of logic section 
        logic_start_time = SDL_GetTicks();

        user_starting_loop(); // USER DEFINED CALL

        //Handle user input
        processInput();

        //Check for user wanting to end program
        if(keep_program_running == false) {
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

        //Render graphics
        user_render_graphics(); //USER DEFINED CALL

        //Render TEXTGRID BACKGROUND
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

        //Render TEXTGRID FOREGROUND (actual text)
        if(text_visible == true)
            renderTextgrid();

        //Calculate and record game-logic duration
        logic_end_time = SDL_GetTicks();
        logic_duration = (logic_end_time - logic_start_time);
        cumulative_logic_duration += logic_duration;

        //Render
        if(current_graphics_mode == FULLSCREEN_MODE) {
            SDL_SetRenderTarget(window_renderer, NULL);
            SDL_RenderCopy(window_renderer, 
                    target_texture,
                    NULL,
                    &target_texture_rect);
        }

        // render here
        render_start_time = SDL_GetTicks();
        SDL_RenderPresent(window_renderer);
        render_end_time = SDL_GetTicks();
        render_duration = (render_end_time - render_start_time);

            /*
            // pause here on faster monitors?
            while(render_duration + logic_duration < 19.95) {  
                render_end_time = SDL_GetTicks();
                render_duration = (render_end_time - render_start_time);
            }
            */
        
        user_ending_loop(); // USER DEFINED CALL
 
        render_end_time = SDL_GetTicks();
        render_duration = (render_end_time - render_start_time);
        cumulative_render_duration += render_duration;

        //Frame complete
        frame_count++;
    }

    //Cleanup
    shutdownEngine();

    //Report on framerate statistics
    double avg_logic = (double)cumulative_logic_duration / 
        (double)frame_count;
    printf(" FRAMERATE: Average logic duration was: %f ms\n", 
            avg_logic); 
    double avg_render = (double)cumulative_render_duration / 
        (double)frame_count;
    printf(" FRAMERATE: Average render duration was: %f ms\n", 
            avg_render); 
    printf(" FRAMERATE: Total average frame duration: %f ms (%d FPS)\n",
            avg_logic + avg_render, (int)(1000.0 / 
                round(avg_logic + avg_render)));
    fflush(stdout);
}

void moveSprite(struct Sprite* s) {

    //Move sprite by whatever dx,dy are set to
    if(frame_count % s->animation_speed == 0) {
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

bool initializeEngine() {

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
    
    //The glyph sheet holds 128 cells arranged in 16 columns and 16 rows.
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
    
    //Set colors of letterbox background and logical screen background
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
    window_refresh_rate = dm.refresh_rate; //critical value captured here
    printf(" INIT ENGINE: Desktop Pixel Format: %s, %i (bpp)\n", 
            SDL_GetPixelFormatName(f), 
            SDL_BITSPERPIXEL(f) );
    fflush(stdout);
    DESKTOP_SCREEN_WIDTH = dm.w;   //setting global parameters here
    DESKTOP_SCREEN_HEIGHT = dm.h;  //setting global parameters here

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
    success = loadMusicAndSoundFiles();
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
    //initialize_audio(); 
    //printf(" INIT ENGINE: Audio initialized\n");
    //fflush(stdout);

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

void shutdownEngine() {

    user_shutdown(); //USER DEFINED CALL
    SDL_GameControllerClose(gamepad); 
    gamepad = NULL; 

    SDL_DestroyRenderer(window_renderer);
    window_renderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}
 
bool buildWindowAndRenderer() {
    
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
        printf(" GRAPHICS: Graphics Mode set to FULLSCREEN\n");
        fflush(stdout);
    
    } else if(current_graphics_mode == WINDOWED_MODE) {

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019 (DELTA)",
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 GAME_SCREEN_WIDTH * current_scale_factor,
                 GAME_SCREEN_HEIGHT * current_scale_factor,
                 SDL_WINDOW_SHOWN);
        printf(" GRAPHICS: Graphics Mode set to WINDOWED\n");
        fflush(stdout);
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
                printf(" GRAPHICS: SDL_RenderSetScale() returned success\n");
                fflush(stdout);
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
            
            printf(" GRAPHICS: FULLSCREEN renderer created\n");
            fflush(stdout);
           
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
                        current_scale_factor, current_scale_factor) == 0) {
                printf(" GRAPHICS: SDL_RenderSetScale() returned success\n");
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }
           
            //Make certain to set main window as render target 
            SDL_SetRenderTarget(window_renderer, NULL);
            
            printf(" GRAPHICS: WINDOWED renderer created\n");
            fflush(stdout);
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

void destroyAllTextures(void) {
    
    //Textures must be destroyed BEFORE destroying the renderer they are
    //attached to. Otherwise the call to SDL_DestroyTexture() will result
    //in an "invalid texture" error message.

    user_destroy_all_textures(); //USER DEFINED CALL
}

void destroySpriteTexture(struct Sprite* s) {

    if(s != NULL) {
        SDL_DestroyTexture(s->body);
        SDL_DestroyTexture(s->animation_sheet);
    }
}

void createAllTextures(void) {

    user_create_all_textures(); //USER DEFINED CALL
}

void createSpriteTexture(struct Sprite* s, const char *filename1,
       const char *filename2) {

    s->body = createOptimizedTextureFromImageFile(filename1);
    s->animation_sheet = createOptimizedTextureFromImageFile(filename2);
}

struct Sprite* createSprite(const char *filename1, 
        const char* filename2) {

    struct Sprite* s = (struct Sprite *)malloc(sizeof(struct Sprite));

    s->body = createOptimizedTextureFromImageFile(filename1);
    
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
    
    s->animation_sheet = createOptimizedTextureFromImageFile(filename2);

    return s;
}

SDL_Texture* createOptimizedTextureFromImageFile(const char* filename) {

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

void initializeTextgridBackgroundArray() {
    for(int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for(int c = 0; c < TEXTGRID_WIDTH; c++) {
            textgrid_background[r][c] = EMPTY;
        }
    }
}

void clearTextgrid(void) {
    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
            text[r][c] = ' ';
        }
    }
}

void clearTextgridRow(int r) {
    for (int c = 0; c < TEXTGRID_WIDTH; c++) {
        text[r][c] = ' ';
    }
}

void printToTextgrid(char* string, int r, int c) {

    // like printf(), but prints to the textgrid instead of stdout.

    string_index = 0;

    if (r < TEXTGRID_HEIGHT) {

        while(c < TEXTGRID_WIDTH && string[string_index] != 0) {
        
            text[r][c] = string[string_index];
            c++;
            string_index++;
        }
    }
}

void renderTextgrid(void) {

    //Iterates through entire text[][] array and if the character is not a
    //' ' character, uses the glyph Texture to render onto the main screen. 

    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
       
            array_index = text[r][c]; // 0-127 

            if(array_index != ' ') {
                SDL_RenderCopy(window_renderer, glyph_sheet, 
                        &glyph_rect[array_index], 
                        &text_rect[r][c]); 
            }
        }
    }
}

bool processInput(void) {

    //This will loop until no further input events are found in the 
    //event queue.
    while(SDL_PollEvent(&input_event) != 0) {

        switch(input_event.type) {

            case SDL_WINDOWEVENT:
                /*
                if(input_event.window.event == SDL_WINDOWEVENT_ENTER) {
                    printf(" >>> SDL_WINDOWEVENT (mouse has entered)\n");
                } else if(input_event.window.event == SDL_WINDOWEVENT_LEAVE) {
                    printf(" >>> SDL_WINDOWEVENT (mouse has exited)\n");
                }
                */
                break;

            case SDL_KEYDOWN:
                keyboard_key_down_handler(input_event.key.keysym.sym); 
                break;

            case SDL_KEYUP:
                keyboard_key_up_handler(input_event.key.keysym.sym); 
                break;

            case SDL_JOYAXISMOTION:
                gamepad_dpad_handler(input_event);
                break;

            case SDL_JOYBUTTONDOWN:
                gamepad_button_handler(input_event);
                break;
            case SDL_JOYBUTTONUP:
                /*
                printf(" >>> SDL_JOYBUTTONUP: button %d\n",
                        input_event.cbutton.button);
                        */
                if(input_event.cbutton.button == 2)
                    gamepad_button_y = false;
                if(input_event.cbutton.button == 0)
                    gamepad_button_b = false;
                if(input_event.cbutton.button == 3)
                    gamepad_button_x = false;
                if(input_event.cbutton.button == 1)
                    gamepad_button_a = false;
                break;
            case SDL_MOUSEMOTION:
                //accurate x,y for windowed mode only
                /*
                printf(" >>> SDL_MOUSEMOTION: %d, %d\n",
                       input_event.motion.x / current_scale_factor, 
                       input_event.motion.y / current_scale_factor);
                       */
                break;
            case SDL_MOUSEBUTTONDOWN:
                /*
                printf(" >>> SDL_MOUSEBUTTONDOWN: button %d\n",
                        input_event.button.button);
                        */
                break;
            case SDL_MOUSEBUTTONUP:
                /*
                printf(" >>> SDL_MOUSEBUTTONUP: button %d\n",
                        input_event.button.button);
                        */
                break;
            case SDL_QUIT: //clicking 'x' button on window
                keep_program_running = false;
                break;
            default:
                //printf(" >>> UNKNOWN INPUT %d <<<\n",
                //    input_event.type);
                break;
        } 
        
        fflush(stdout);
    }
}

void keyboard_key_up_handler(SDL_Keycode kc) {

    switch(kc) {

        case SDLK_UP:
            keyboard_arrow_north = false;
        break;

        case SDLK_DOWN:
            keyboard_arrow_south = false;
        break;
        
        case SDLK_LEFT:
            keyboard_arrow_west = false;
        break;
        
        case SDLK_RIGHT:
            keyboard_arrow_east = false;
        break;
        
        user_keyboard_handler_key_up(kc); //USER DEFINED CALL
        
        default:
        break;
    }
}

void keyboard_key_down_handler(SDL_Keycode kc) {

    int cc;

    switch(kc) {

        case SDLK_ESCAPE:
            keep_program_running = false;
        break;
        
        case SDLK_UP:
            keyboard_arrow_north = true;
        break;

        case SDLK_DOWN:
            keyboard_arrow_south = true;
        break;
        
        case SDLK_LEFT:
            keyboard_arrow_west = true;
        break;
        
        case SDLK_RIGHT:
            keyboard_arrow_east = true;
        break;
        
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

        default:
            user_keyboard_handler_key_down(kc); //USER DEFINED CALL
        break;
    }
}

void moveSprite(struct Sprite* s, int dx, int dy) {

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

bool loadMusicAndSoundFiles() {
    
    //Loading success flag
    bool success = true;

    success = user_music_and_sound_files(); //USER DEFINED CALL

    return success;
}
#endif
