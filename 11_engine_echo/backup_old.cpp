//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../11_engine_echo.cpp
// Last Modified: Mon Oct 07, 2019  12:09AM
// LOC: 819
// Filesize: 37569 bytes
//  
// Notes:
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
//                                ^                      button 2
//                                |
//         (axis 0)   -32768 <----0----> 32767    button 3      button 0
//                                |
//                                v                      button 1
//                              32767
//
//

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <ctime>    // to seed random number generator
#include <cstdlib>  // for rand() and srand() functions
#include <math.h>   // for round()

//Engine parameters
const int GAME_SCREEN_WIDTH  = 480;    
const int GAME_SCREEN_HEIGHT = 360;
int       DESKTOP_SCREEN_WIDTH = 0; //set during initialization
int       DESKTOP_SCREEN_HEIGHT = 0;
int       MAX_SCALE_FACTOR = 0; //always a whole number value
const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 8;
const int TEXTGRID_WIDTH = GAME_SCREEN_WIDTH / FONT_WIDTH;    //60 columns
const int TEXTGRID_HEIGHT = GAME_SCREEN_HEIGHT / FONT_HEIGHT; //45 rows
const int FULLSCREEN_MODE = 0;
const int WINDOWED_MODE   = 1;

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
int                 current_graphics_mode;
SDL_Rect            game_screen_rect;
SDL_Texture*        target_texture;       //for fullscreen rendering
COLORS              letterbox_color;      //for fullscreen rendering
SDL_Rect            target_texture_rect;  //for fullscreen rendering
int                 current_scale_factor;
bool                keep_program_running = true;

//Sprite objects
struct Sprite {
    SDL_Texture* texture;
    SDL_Rect     rect;
    int          dx;
    int          dy;
    bool         visible;
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

    //GRAPHICS LAYERS
    const int      MAX_SPRITES_PER_LAYER = 20;
    struct Sprite* graphics_bottom_layer_sprite[MAX_SPRITES_PER_LAYER];
    struct Sprite* graphics_top_layer_sprite[MAX_SPRITES_PER_LAYER];

    //TEXTGRID BACKGROUND (SOLID CELLS) 
    COLORS textgrid_background[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    void initializeTextgridBackgroundArray();

    //TEXTGRID FOREGROUND (TEXT)
    const int     NUM_GLYPHS = 128; //number of cells on master font sheet
    SDL_Texture*  glyph_sheet = NULL;
    SDL_Rect      glyph_rect[NUM_GLYPHS]; 
    char          text[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];  //ASCII data per cell
    SDL_Rect      text_rect[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    void initializeTextSystem(void); 
    void renderTextgrid(void);
    void clearTextgrid(void);
    void clearTextgridRow(int r);
    void printToTextgrid(char* string, int r, int c);
    char          array_index; 
    char          temp_string[256];  //for writing formatted strings
    int           string_index;

//Engine control functions 
bool         initializeEngine();
void         shutdownEngine();
bool         changeGraphicsMode(); 
SDL_Texture* createOptimizedTextureFromImageFile(const char* filename);
void         rebuildSpriteArray(void);
void         report_step(const char* s);

//Input system
SDL_Event           input_event;
SDL_GameController* gamepad;
bool processInput(void);                                        
void keyboard_handler(SDL_Keycode kc);

//Helper variables
COLORS temp_color;
int temp_row, temp_col;
SDL_Rect temp_rect;
bool text_visible = false;
int cc; //for keyboard input handler

//Main loop
int main(int argc, char* args[]) {

    //Set everything that's independent of graphics 
    if(initializeEngine() == false) {
        printf(" Failed initializeEngine() call\n");
        fflush(stdout);
        return 1;
    }
    
    //Initialize SDL system, create main window, link pointers to that main
    //window's Renderer and Surface.
    if(changeGraphicsMode() == false) {
        printf(" Failed changeGraphicsMode() call\n");
        fflush(stdout);
        return 1;
    }

    //Report what the native pixel format is of the Window (once)
    window_pixel_format = SDL_GetWindowPixelFormat(window);
    const char* temp = SDL_GetPixelFormatName(window_pixel_format);
    printf(" SDL_Window created with pixel format %s\n", temp);
    fflush(stdout);
            
    //Display some information about the renderer (once)
    SDL_GetRenderDriverInfo(0, &renderer_info);
    printf(" Renderer name: %s\n", renderer_info.name);
    fflush(stdout);

    //set up initial graphics
    background_layer_color = BLUE;
    initializeTextgridBackgroundArray();
    
    //Initialize text/font system
    initializeTextSystem();

    //Fill up text[][] with random garbage
    for (int r = 0; r < TEXTGRID_HEIGHT; r++) {
        for (int c = 0; c < TEXTGRID_WIDTH; c++) {
            text[r][c] = rand() % NUM_GLYPHS;
        }
    }
    for(int i = 14; i < 19; i++) {
        clearTextgridRow(i);
    }
    printToTextgrid("ENGINE ECHO", 16, 24);

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

        //Draw GRAPHICS BOTTOM LAYER 
        for(int i = 0; i < 1; i++) {
            if(graphics_bottom_layer_sprite[i]->visible == true) {
                SDL_RenderCopy(windowRenderer, 
                    graphics_bottom_layer_sprite[i]->texture, 
                    NULL,
                    &graphics_bottom_layer_sprite[i]->rect);
            }
        }
        
        //Draw GRAPHICS TOP LAYER 
        for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
            if(graphics_top_layer_sprite[i]->visible == true) {
                SDL_RenderCopy(windowRenderer, 
                    graphics_top_layer_sprite[i]->texture, 
                    NULL,
                    &graphics_top_layer_sprite[i]->rect);
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
        
        //Move sprites on top layer
        for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {

            graphics_top_layer_sprite[i]->rect.x +=
                 graphics_top_layer_sprite[i]->dx;
            
            graphics_top_layer_sprite[i]->rect.y +=
                 graphics_top_layer_sprite[i]->dy;

            if(graphics_top_layer_sprite[i]->rect.x +
               graphics_top_layer_sprite[i]->rect.w < 0) {
                   graphics_top_layer_sprite[i]->rect.x =
                       GAME_SCREEN_WIDTH;
            } else if (graphics_top_layer_sprite[i]->rect.x >
                    GAME_SCREEN_WIDTH) {
                   graphics_top_layer_sprite[i]->rect.x = 0 -
                       graphics_top_layer_sprite[i]->rect.w;
            }
            
            if(graphics_top_layer_sprite[i]->rect.y +
               graphics_top_layer_sprite[i]->rect.h < 0) {
                   graphics_top_layer_sprite[i]->rect.y =
                       GAME_SCREEN_HEIGHT;
            } else if (graphics_top_layer_sprite[i]->rect.y >
                    GAME_SCREEN_HEIGHT) {
                   graphics_top_layer_sprite[i]->rect.y = 0 -
                       graphics_top_layer_sprite[i]->rect.h;
            }
        }
        
        //Move sprites on bottom layer
        for(int i = 0; i < 1; i++) {

            graphics_bottom_layer_sprite[i]->rect.x +=
                 graphics_bottom_layer_sprite[i]->dx;
            
            graphics_bottom_layer_sprite[i]->rect.y +=
                 graphics_bottom_layer_sprite[i]->dy;

            if(graphics_bottom_layer_sprite[i]->rect.x +
               graphics_bottom_layer_sprite[i]->rect.w < 0) {
                   graphics_bottom_layer_sprite[i]->rect.x =
                       GAME_SCREEN_WIDTH;
            } else if (graphics_bottom_layer_sprite[i]->rect.x >
                    GAME_SCREEN_WIDTH) {
                   graphics_bottom_layer_sprite[i]->rect.x = 0 -
                       graphics_bottom_layer_sprite[i]->rect.w;
            }
            
            if(graphics_bottom_layer_sprite[i]->rect.y +
               graphics_bottom_layer_sprite[i]->rect.h < 0) {
                   graphics_bottom_layer_sprite[i]->rect.y =
                       GAME_SCREEN_HEIGHT;
            } else if (graphics_bottom_layer_sprite[i]->rect.y >
                    GAME_SCREEN_HEIGHT) {
                   graphics_bottom_layer_sprite[i]->rect.y = 0 -
                       graphics_bottom_layer_sprite[i]->rect.h;
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
 
        //Frame complete
        frame_count++;
    }

    //Cleanup
    shutdownEngine();

    //Report on framerate statistics
    double avg_logic = (double)cumulative_logic_duration / (double)frame_count;
    printf(" FRAMERATE: Average logic duration was: %f ms\n", 
            avg_logic); 
    double avg_render = (double)cumulative_render_duration / (double)frame_count;
    printf(" FRAMERATE: Average render duration was: %f ms\n", 
            avg_render); 
    printf(" FRAMERATE: Total average frame duration: %f ms (%d FPS)\n",
            avg_logic + avg_render, (int)(1000.0 / round(avg_logic + avg_render)));
    fflush(stdout);

	return 0;
}

bool initializeEngine() {

    //Return value 
    bool success = true; 
    report_step("initializeEngine() function begins here");
    
    //Standard C++ way of seeding random number generator
    srand(time(NULL)); 
    report_step("random number generator seeded");
    
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
    report_step("RGB values calculated for 27 colors");
    
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
    report_step("All cells in textgrid pre-calculated (Rects)");

    //Build sprite arrays
    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        graphics_bottom_layer_sprite[i] = NULL;
        graphics_top_layer_sprite[i] = NULL;
    }
    report_step("Sprite lists initialized to NULL");

    //Set colors of letterbox background and logical screen background
    letterbox_color = GRAY;
    
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
    report_step("SDL_Init() called");

    //Set graphics mode
    current_graphics_mode = WINDOWED_MODE;  
    
    //Capture native desktop resolution, must be called after SDL_Init()
    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        printf(" SDL_GetDesktopDisplayMode failed: %s", 
                SDL_GetError());
        fflush(stdout);
        return 1;
    }
    Uint32 f = dm.format;
    printf(" Desktop Resolution: %i x %i @ %i Hz\n", 
            dm.w, dm.h, dm.refresh_rate);
    window_refresh_rate = dm.refresh_rate; //critical value captured here
    printf(" Desktop Pixel Format: %s, %i (bpp)\n", 
            SDL_GetPixelFormatName(f), 
            SDL_BITSPERPIXEL(f) );
    fflush(stdout);
    DESKTOP_SCREEN_WIDTH = dm.w;   //setting global parameters here
    DESKTOP_SCREEN_HEIGHT = dm.h;  //setting global parameters here
    report_step("Captured desktop resolution");

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
    printf(" Maximum SCALING FACTOR found to be x%d\n", MAX_SCALE_FACTOR);
    fflush(stdout);
    report_step("Found scaling factor for Fullscreen Mode");

    current_scale_factor = 1; //Make initial window non-scaled 

    //Setup gamepad
    gamepad = NULL;
    for(int i = 0; i < SDL_NumJoysticks(); ++i) {
        if(SDL_IsGameController(i)) {
            gamepad = SDL_GameControllerOpen(i);
            if(gamepad) {
                report_step("Gamepad set up");
                break;
            } else {
                printf(" Could not open gamecontroller %i: %s\n", 
                        i, SDL_GetError());
            }
        }
    }

    report_step("initializeEngine() function ends here");

    return success;
}

void shutdownEngine() {
    
    SDL_GameControllerClose(gamepad); 
    gamepad = NULL; 

    SDL_DestroyRenderer(windowRenderer);
    windowRenderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    IMG_Quit();

    SDL_Quit();
}

bool changeGraphicsMode() {
    
    bool success = true; 

    //Cleanup any existing Textures held in Sprite objects
    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        if(graphics_top_layer_sprite[i] != NULL) {
            SDL_DestroyTexture(graphics_top_layer_sprite[i]->texture);
            graphics_top_layer_sprite[i]->texture = NULL;
        }
    }
    for(int i = 0; i < 1; i++) {
        if(graphics_bottom_layer_sprite[i] != NULL) {
            SDL_DestroyTexture(graphics_bottom_layer_sprite[i]->texture);
            graphics_bottom_layer_sprite[i]->texture = NULL;
        }
    }

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
    
        report_step("Graphics Mode set to FULLSCREEN");
    
    } else if(current_graphics_mode == WINDOWED_MODE) {

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019 (DELTA)",
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 GAME_SCREEN_WIDTH * current_scale_factor,
                 GAME_SCREEN_HEIGHT * current_scale_factor,
                 SDL_WINDOW_SHOWN);
        
        report_step("Graphics Mode set to WINDOWED");
    }
        
    //Setup a Rect object with the window's logical pixel resolution.
    game_screen_rect.x = 0;
    game_screen_rect.y = 0;
    game_screen_rect.w = GAME_SCREEN_WIDTH;
    game_screen_rect.h = GAME_SCREEN_HEIGHT;

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
            target_texture = SDL_CreateTexture(
                    windowRenderer, 
                    window_pixel_format,
		            SDL_TEXTUREACCESS_TARGET, 
                    GAME_SCREEN_WIDTH, 
                    GAME_SCREEN_HEIGHT);    
        
            //Set scaling here
            //This is a big function call here that effects all later 
            //rendering calls
            if(SDL_RenderSetScale(windowRenderer, 
                        MAX_SCALE_FACTOR, MAX_SCALE_FACTOR) == 0) {
                report_step("SDL_RenderSetScale() returned success");
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }

            //clear entire fullscreen desktop, only needs to be done once 
            SDL_SetRenderDrawColor(windowRenderer,
                    r_val[letterbox_color],
                    g_val[letterbox_color],
                    b_val[letterbox_color],
                    0xFF);
            SDL_RenderClear(windowRenderer);
            SDL_RenderPresent(windowRenderer);

            report_step("FULLSCREEN renderer created");
           
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
                report_step("SDL_RenderSetScale() returned success");
            } else {
                printf(" SDL_RenderSetScale() returned error: %s\n",
                        SDL_GetError());
                fflush(stdout);
            }
           
            //Make certain to set main window as render target 
            SDL_SetRenderTarget(windowRenderer, NULL);
            
            report_step("WINDOWED renderer created");
        }
	
        if(windowRenderer == NULL) {
                
            printf(" Renderer could not be created (SDL Error: %s)\n", 
                    SDL_GetError());
            fflush(stdout);
            success = false;
        } 
    }
                
    //Load support for PNG image formats
    int flags = IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if((initted & flags) != flags) {
        printf(" Failed to init png support (IMG_Init: %s)\n", 
                IMG_GetError());
        fflush(stdout);
        success = false;
    }

    //Rebuild Texture arrays
    rebuildSpriteArray();
    
    return success;
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
        newTexture = SDL_CreateTextureFromSurface(windowRenderer, loadedSurface);
            
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
                if(input_event.key.repeat == 0) {   //ignore repeats
                    printf(" >>> SDL_KEYDOWN: %s\n",
                        SDL_GetKeyName(input_event.key.keysym.sym));
                    keyboard_handler(input_event.key.keysym.sym); 
                }
                break;
            case SDL_KEYUP:
                printf(" >>> SDL_KEYUP: %s\n",
                    SDL_GetKeyName(input_event.key.keysym.sym));
                break;
            case SDL_JOYAXISMOTION:
                //left-right is axis 0 (+ to the right)
                //up-down is axis 1 (+ is downward)
                if(input_event.caxis.value != 0) {
                    printf(" >>> SDL_JOYAXISMOTION: %d (value), %d (axis)   ",
                        input_event.caxis.value,
                        input_event.caxis.axis);
                    if(input_event.caxis.axis == 0) {
                        if(input_event.caxis.value < 0)
                            printf("LEFT");
                        else if(input_event.caxis.value > 0)
                            printf("RIGHT");
                    } else {
                        if(input_event.caxis.value < 0)
                            printf("UP");
                        else if(input_event.caxis.value > 0)
                            printf("DOWN");
                    }
                    printf("\n");
                }
                break;
            case SDL_JOYBUTTONDOWN:
                printf(" >>> SDL_JOYBUTTONDOWN: button %d\n",
                        input_event.cbutton.button);
                break;
            case SDL_JOYBUTTONUP:
                printf(" >>> SDL_JOYBUTTONUP: button %d\n",
                        input_event.cbutton.button);
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

void report_step(const char* s) {

    printf(" STEP COMPLETE: %s\n", s);
    fflush(stdout);
}

void rebuildSpriteArray(void) {

    //rebuild sprites in top graphics layer, if sprite exists,
    //otherwise create a new sprite in that array slot.

    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        
        if(graphics_top_layer_sprite[i] != NULL) {

            //First, free up the memory taken by the old Texture
            SDL_DestroyTexture(graphics_top_layer_sprite[i]->texture);
            graphics_top_layer_sprite[i]->texture = NULL;

            //Second, allocate memory for the new Texture
            graphics_top_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile( 
                        "../graphics/sprite_1.png");
        
        } else {
            
            graphics_top_layer_sprite[i] = 
                (struct Sprite *)malloc(sizeof(struct Sprite));   //LEAK HERE!!!
            graphics_top_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile(
                        "../graphics/sprite_1.png");
            graphics_top_layer_sprite[i]->rect.x = 
                rand() % GAME_SCREEN_WIDTH;
            graphics_top_layer_sprite[i]->rect.y = 
                rand() % GAME_SCREEN_HEIGHT;
            SDL_QueryTexture(graphics_top_layer_sprite[i]->texture,
                    NULL, NULL, 
                    &graphics_top_layer_sprite[i]->rect.w,
                    &graphics_top_layer_sprite[i]->rect.h);
            graphics_top_layer_sprite[i]->dx = (rand() % 8) - 4;
            graphics_top_layer_sprite[i]->dy = (rand() % 8) - 4;
            graphics_top_layer_sprite[i]->visible = true;

        }
    } 
    
    //rebuild sprites in bottom graphics layer, if sprite exists,
    //otherwise create a new sprite in that array slot.

    for(int i = 0; i < 1; i++) {
        
        if(graphics_bottom_layer_sprite[i] != NULL) {

            //First, free up the memory taken by the old Texture
            SDL_DestroyTexture(graphics_bottom_layer_sprite[i]->texture);
            graphics_bottom_layer_sprite[i]->texture = NULL;

            //Second, allocate memory for the new Texture
            graphics_bottom_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile( 
                        "../graphics/big_sprite.png");
        
        } else {

            graphics_bottom_layer_sprite[i] = 
                (struct Sprite *)malloc(sizeof(struct Sprite));
            graphics_bottom_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile(
                        "../graphics/big_sprite.png");
            graphics_bottom_layer_sprite[i]->rect.x = 
                rand() % GAME_SCREEN_WIDTH;
            graphics_bottom_layer_sprite[i]->rect.y = 
                rand() % GAME_SCREEN_HEIGHT;
            SDL_QueryTexture(graphics_bottom_layer_sprite[i]->texture,
                    NULL, NULL, 
                    &graphics_bottom_layer_sprite[i]->rect.w,
                    &graphics_bottom_layer_sprite[i]->rect.h);
            graphics_bottom_layer_sprite[i]->dx = (rand() % 3) - 1;
            graphics_bottom_layer_sprite[i]->dy = (rand() % 3) - 1;
            while(graphics_bottom_layer_sprite[i]->dx == 0 || 
                  graphics_bottom_layer_sprite[i]->dy == 0) {
                    graphics_bottom_layer_sprite[i]->dx = 
                        (rand() % 3) - 1;
                    graphics_bottom_layer_sprite[i]->dy = 
                        (rand() % 3) - 1;
            }
            graphics_bottom_layer_sprite[i]->visible = true;
        }
    } 

    initializeTextSystem(); 
}

void initializeTextSystem(void) {

    //This function loads the big Texture full of 128 character glyphs that
    //will be used to render all text, and it calculates the Rects (x,y
    //locations) of each glyph on the sheet (also used during rendering).

    glyph_sheet = createOptimizedTextureFromImageFile(
            "../graphics/c64_font.bmp");

    //The glyph sheet holds 128 cells arranged in 16 columns and 16 rows.
    for (int i = 0; i < NUM_GLYPHS; i++) {
        glyph_rect[i].x = (i % 16) * FONT_WIDTH;
        glyph_rect[i].y = (i / 16) * FONT_HEIGHT;
        glyph_rect[i].w = FONT_WIDTH;
        glyph_rect[i].h = FONT_HEIGHT;
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

void keyboard_handler(SDL_Keycode kc) {

    switch(kc) {

        case SDLK_ESCAPE:
            keep_program_running = false;
        break;

        case SDLK_m:
            if(current_graphics_mode == FULLSCREEN_MODE)
                current_graphics_mode = WINDOWED_MODE;
            else
                current_graphics_mode = FULLSCREEN_MODE;
            changeGraphicsMode();
        break;

        case SDLK_s:
            if(current_graphics_mode == WINDOWED_MODE) {
                current_scale_factor++;
                if(current_scale_factor > MAX_SCALE_FACTOR)
                    current_scale_factor = 1;
                changeGraphicsMode();
            }
        break;

        case SDLK_1:
            cc = rand() % COLOR_COUNT;
            background_layer_color = static_cast<COLORS>(cc);
            printf(" Background color changed to %s\n", 
                    COLOR_NAME[cc]);
            fflush(stdout);
        break;
        
        case SDLK_2:
            for(int i = 0; i < 1; i++) {
                if(graphics_bottom_layer_sprite[i]->visible == false) {
                    graphics_bottom_layer_sprite[i]->visible = true;
                } else {
                    graphics_bottom_layer_sprite[i]->visible = false;
                }
            }
        break;

        case SDLK_3:
            for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
                if(graphics_top_layer_sprite[i]->visible == false) {
                    graphics_top_layer_sprite[i]->visible = true;
                } else {
                    graphics_top_layer_sprite[i]->visible = false;
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
        break;

        case SDLK_5:
            if(text_visible == true) 
                text_visible = false;
            else
                text_visible = true;
        break;

        default:
        break;
    }
}
