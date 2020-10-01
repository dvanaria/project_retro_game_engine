//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../12_engine_foxtrot.cpp
// Last Modified: Tue Oct 15, 2019  11:47PM
// LOC: 1048
// Filesize: 46382 bytes
//  
//     Graphics: 
//
//     Window sizes (whether for Fullscreen mode or Windowed mode) will be
//     multiplied by the SCALING FACTOR (found by dividing the native desktop
//     pixel resolution by the desired game resolution).
//
//     The logical game resolution (480 x 360) is always used for in-game
//     coordinates (SCALING is never considered).
//
//     The function SDL_RenderSetScale() is used during setup (and can be
//     modified during game) to setup SDL to do all the scaling calculations for
//     us.
// 
//     Gamepad:
//                            (axis 1)
//
//                             -32768
//                                ^                      button 2(y)
//                                |
//         (axis 0)   -32768 <----0----> 32767    button 3(x)    button 0(b)
//                                |
//                                v                      button 1(a)
//                              32767
//
//
//     Digital Sound Generation:
//
//     Old-school arcade machines often produced sound using discrete logic
//     circuits. Space Invaders, for example, used 8 discrete circuits to
//     produce all of its sound effects (many circuits had multiple inputs and
//     so could be used to generate several sound effects).
//
//     Discrete circuits (hardware) is often built up from smaller, simpler
//     modules, just like complex software is built.
//
//     In order to emulate this behavior, the analog waveforms these circuits
//     generate (described by mathematical formulas which are often well
//     documented) must be digitally "sampled" (captured levels at specific
//     points in time). 
//
//     It turns out that for even very simple waveforms, such as a square wave,
//     it takes a LOT of samples to approximate the analog wave. A typical
//     sampling rate is 44100 samples PER SECOND!
//
//     The SDL audio library provides a function, SDL_QueueAudio() which is
//     fed digital samples (really just volume levels), buffering them
//     internally until a threshold is reached and then plays the sound.
//
//     Since the game loop is locked into sampling rate (which should be
//     accurately polled from the system), you can figure out how many samples
//     should be sent to the SDL function per frame, in order to give the
//     desired 44100 sample per second frequency.
//
//     Collision Detection:
//
//     SDL has some great collision detection routines built in:
//
//         1. SDL_HasIntersection(r1, r2)          //ship hits another ship?
//         2. SDL_IntersectRectAndLine(r1, line)   //laser hits ship?
//         3. SDL_PointInRect(point, r)            //bullet hits robot?
//
//    Simplified Sound Effects and Music:
//
//    You can use SDL_mixer (a 3rd party library like SDL_image) to play .wav
//    files (for both music and sound effects).
//
//    Data types for music and sound effects are Mix_Music and Mix_Chunk.

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
int       MAX_SCALE_FACTOR      = 0; //always a whole number value
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
SDL_Window*         window          = NULL;  
Uint32              window_pixel_format;
int                 window_refresh_rate;
SDL_Renderer*       windowRenderer  = NULL;
SDL_RendererInfo    renderer_info;
SDL_Rect            game_screen_rect;
SDL_Texture*        target_texture;       //for fullscreen rendering
SDL_Rect            target_texture_rect;  //for fullscreen rendering
COLORS              letterbox_color;      //for fullscreen rendering
int                 current_graphics_mode;
int                 current_scale_factor;
bool                keep_program_running = true;

//Sprite objects
struct Sprite {
    SDL_Texture* texture;
    SDL_Rect     rect;
    int          dx;
    int          dy;
    bool         visible;
    double       angle;    // degrees, north = 0, rotation = clockwise
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

//Layering system (graphics)

//SOLID BACKGROUND LAYER
COLORS background_layer_color;

//SPRITES
const int         MAX_SPRITES_IN_LIST = 20;
struct Sprite*    sprite_list[MAX_SPRITES_IN_LIST];
struct Sprite*    background_sprite = NULL;
struct Sprite*    gamepad_sprite = NULL;
struct Sprite*    keyboard_sprite = NULL;
SDL_Texture* createOptimizedTextureFromImageFile(const char* filename);
struct Sprite* createSprite(const char *filename);
void createAllTextures(void);
void createSpriteTexture(struct Sprite* s, const char *filename);
void destroyAllTextures(void);
void destroySpriteTexture(struct Sprite* s);
void moveSprite(struct Sprite* s); //auto-move by s->dx
void moveSprite(struct Sprite* s, int dx, int dy); //manual-move

//TEXTGRID BACKGROUND (SOLID CELLS) 
COLORS        textgrid_background[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
void initializeTextgridBackgroundArray();

//TEXTGRID FOREGROUND (TEXT)
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

//Engine control functions 
bool initializeEngine();
void shutdownEngine();
bool buildWindowAndRenderer(); 

//Input system
SDL_Event             input_event;
bool processInput(void);                                        
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
bool                  keyboard_arrow_north  = false;
bool                  keyboard_arrow_south  = false;
bool                  keyboard_arrow_west   = false;
bool                  keyboard_arrow_east   = false;
void keyboard_key_down_handler(SDL_Keycode kc);
void keyboard_key_up_handler(SDL_Keycode kc);

//Sound system (digital synth)
SDL_AudioDeviceID   audio_device_id;
SDL_AudioFormat     audio_format = AUDIO_S16; //+32767 to -32768
Uint16              audio_buffer_size = 4096;
int                 audio_frequency = 44100;
void initialize_audio(void);

//Sound system (SDL_mixer system)
const int         MAX_SOUND_EFFECTS_IN_LIST = 4;
Mix_Chunk *       sound_effect_list[MAX_SOUND_EFFECTS_IN_LIST];
const int         MAX_MUSIC_IN_LIST = 1;
Mix_Music *       music_list[MAX_MUSIC_IN_LIST];
bool loadMusicAndSoundFiles();

//Helper variables
COLORS temp_color;
int temp_row, temp_col;
SDL_Rect temp_rect;
bool text_visible = false;

//Main loop
int main(int argc, char* args[]) {

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

    //Initialize text/font system
    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");
    initializeTextgridBackgroundArray();

    //Fill up text[][] with random garbage
    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
            text[r][c] = rand() % NUM_GLYPHS;
        }
    }
    for(int i = 14; i < 19; i++) {
        clearTextgridRow(i);
    }
    printToTextgrid(ENGINE_NAME, 16, 24);

    //Loop until user quits.
    bool quit_program = false;
    while(quit_program == false) {

        // Start of logic section 
        logic_start_time = SDL_GetTicks();

        //Handle user input
        processInput();

        //Check for user wanting to end program
        if(keep_program_running == false) {
            quit_program = true;
            break;
        }

        //Change rendering targets here for fullscreen mode
        if(current_graphics_mode == FULLSCREEN_MODE)
            SDL_SetRenderTarget(windowRenderer, target_texture);

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

        //Render TEXTGRID BACKGROUND
        for(temp_row = 0; temp_row < TEXTGRID_HEIGHT; temp_row++) {
            for(temp_col = 0; temp_col < TEXTGRID_WIDTH; temp_col++) {
                if(textgrid_background[temp_row][temp_col] != EMPTY) {

                    temp_color = textgrid_background[temp_row][temp_col];

                    SDL_SetRenderDrawColor(windowRenderer, 
                            r_val[temp_color],
                            g_val[temp_color],
                            b_val[temp_color],
                            0xFF);

                    SDL_RenderFillRect(windowRenderer, 
                            &text_rect[temp_row][temp_col]);
                }
            }
        }

        //Render TEXTGRID FOREGROUND (actual text)
        if(text_visible == true)
            renderTextgrid();

        //Move sprites

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

        //Collision detection
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
        
        //Calculate and record game-logic duration
        logic_end_time = SDL_GetTicks();
        logic_duration = (logic_end_time - logic_start_time);
        cumulative_logic_duration += logic_duration;

        //Render
        if(current_graphics_mode == FULLSCREEN_MODE) {
            SDL_SetRenderTarget(windowRenderer, NULL);
            SDL_RenderCopy(windowRenderer, 
                    target_texture,
                    NULL,
                    &target_texture_rect);
        }
        render_start_time = SDL_GetTicks();
        SDL_RenderPresent(windowRenderer);
        render_end_time = SDL_GetTicks();
        render_duration = (render_end_time - render_start_time);
        cumulative_render_duration += render_duration;

        /*
        // pause here on faster monitors
        while(render_duration + logic_duration < 19.91) {  
            render_end_time = SDL_GetTicks();
            render_duration = (render_end_time - render_start_time);
        }
        */
 
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

	return 0;
}

void moveSprite(struct Sprite* s) {

    s->rect.x += s->dx;
    s->rect.y += s->dy;

    if(s->rect.x + s->rect.w < 0) {
        s->rect.x = GAME_SCREEN_WIDTH;
    } else if (s->rect.x > GAME_SCREEN_WIDTH) {
        s->rect.x = 0 - s->rect.w;
    }
    
    if(s->rect.y + s->rect.h < 0) {
        s->rect.y = GAME_SCREEN_HEIGHT;
    } else if (s->rect.y > GAME_SCREEN_HEIGHT) {
        s->rect.y = 0 - s->rect.h;
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
    loadMusicAndSoundFiles();
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
    
    SDL_GameControllerClose(gamepad); 
    gamepad = NULL; 

    SDL_DestroyRenderer(windowRenderer);
    windowRenderer = NULL;

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
    if(windowRenderer != NULL) {
        SDL_DestroyRenderer(windowRenderer);
        windowRenderer = NULL;
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
            windowRenderer = SDL_CreateRenderer(window, -1, 
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
                    windowRenderer, 
                    window_pixel_format,
		            SDL_TEXTUREACCESS_TARGET, 
                    GAME_SCREEN_WIDTH, 
                    GAME_SCREEN_HEIGHT);    
        
            //Set scaling here
            //This is an important function call here that effects all later 
            //rendering calls
            if(SDL_RenderSetScale(windowRenderer, 
                        MAX_SCALE_FACTOR, MAX_SCALE_FACTOR) == 0) {
                printf(" GRAPHICS: SDL_RenderSetScale() returned success\n");
                fflush(stdout);
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }

            //clear entire fullscreen desktop
            SDL_SetRenderDrawColor(windowRenderer,
                    r_val[letterbox_color],
                    g_val[letterbox_color],
                    b_val[letterbox_color],
                    0xFF);
            SDL_RenderClear(windowRenderer);
            SDL_RenderPresent(windowRenderer);
            
            printf(" GRAPHICS: FULLSCREEN renderer created\n");
            fflush(stdout);
           
        } else if(current_graphics_mode == WINDOWED_MODE) {

            //Create renderer for window
            windowRenderer = SDL_CreateRenderer(window, -1, 
                    SDL_RENDERER_PRESENTVSYNC |
                    SDL_RENDERER_ACCELERATED
                    );
            
            //Set scaling here
            //This is a big function call here that effects all later 
            //rendering calls
            if(SDL_RenderSetScale(windowRenderer, 
                        current_scale_factor, current_scale_factor) == 0) {
                printf(" GRAPHICS: SDL_RenderSetScale() returned success\n");
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }
           
            //Make certain to set main window as render target 
            SDL_SetRenderTarget(windowRenderer, NULL);
            
            printf(" GRAPHICS: WINDOWED renderer created\n");
            fflush(stdout);
        }
	
        if(windowRenderer == NULL) {
                
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

    destroySpriteTexture(background_sprite);
    destroySpriteTexture(gamepad_sprite);
    destroySpriteTexture(keyboard_sprite);

    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        destroySpriteTexture(sprite_list[i]);

    SDL_DestroyTexture(glyph_sheet);
}

void destroySpriteTexture(struct Sprite* s) {

    if(s != NULL)
        SDL_DestroyTexture(s->texture);
}


void createAllTextures(void) {
    
    //Rebuild sprite Textures
    createSpriteTexture(background_sprite, 
            "../graphics/background_sprite.bmp");
    createSpriteTexture(gamepad_sprite,
           "../graphics/gamepad_sprite.png");
    createSpriteTexture(keyboard_sprite,
            "../graphics/keyboard_sprite.png");
    for(int i = 0; i < MAX_SPRITES_IN_LIST; i++)
        createSpriteTexture(sprite_list[i],
                "../graphics/game_object_sprite.png");
    
    //Rebuild glyph_sheet Texture
    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");
}

void createSpriteTexture(struct Sprite* s, const char *filename) {

    s->texture = createOptimizedTextureFromImageFile(filename);

}

struct Sprite* createSprite(const char *filename) {

    struct Sprite* s = (struct Sprite *)malloc(sizeof(struct Sprite));

    s->texture = createOptimizedTextureFromImageFile(filename);
    
    s->rect.x = rand() % GAME_SCREEN_WIDTH;  //random location
    s->rect.y = rand() % GAME_SCREEN_HEIGHT;

    SDL_QueryTexture(s->texture,
            NULL, NULL, 
            &s->rect.w,
            &s->rect.h);

    s->dx = (rand() % 8) - 4;  //random velocity
    s->dy = (rand() % 8) - 4;

    s->visible = true;

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
                windowRenderer, loadedSurface);
            
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
                SDL_RenderCopy(windowRenderer, glyph_sheet, 
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
                if(input_event.window.event == SDL_WINDOWEVENT_ENTER) {
                    printf(" >>> SDL_WINDOWEVENT (mouse has entered)\n");
                } else if(input_event.window.event == SDL_WINDOWEVENT_LEAVE) {
                    printf(" >>> SDL_WINDOWEVENT (mouse has exited)\n");
                }
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
                printf(" >>> SDL_JOYBUTTONDOWN: button %d\n",
                        input_event.cbutton.button);
                if(input_event.cbutton.button == 2)
                    gamepad_button_y = true;
                if(input_event.cbutton.button == 0) 
                    gamepad_button_b = true;
                if(input_event.cbutton.button == 3) {
                    if(Mix_PlayingMusic() == 0) {
                        //Play the music
                        Mix_PlayMusic(music_list[0], -1);
                    }
                    gamepad_button_x = true;
                }
                if(input_event.cbutton.button == 1) {
                    gamepad_button_a = true;
                    Mix_HaltMusic();
                }
                break;
            case SDL_JOYBUTTONUP:
                printf(" >>> SDL_JOYBUTTONUP: button %d\n",
                        input_event.cbutton.button);
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

    //if(input_event.key.repeat == 0) {   //ignore repeats

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
        
        default:
        break;
    }
}

void keyboard_key_down_handler(SDL_Keycode kc) {

    //if(input_event.key.repeat == 0) {   //ignore repeats
     
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

void moveSprite(struct Sprite* s, int dx, int dy) {

    s->rect.x += dx;
    s->rect.y += dy;

    if(s->rect.x + s->rect.w < 0) {
        s->rect.x = GAME_SCREEN_WIDTH;
    } else if (s->rect.x > GAME_SCREEN_WIDTH) {
        s->rect.x = 0 - s->rect.w;
    }
    
    if(s->rect.y + s->rect.h < 0) {
        s->rect.y = GAME_SCREEN_HEIGHT;
    } else if (s->rect.y > GAME_SCREEN_HEIGHT) {
        s->rect.y = 0 - s->rect.h;
    }
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
