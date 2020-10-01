//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
//
// This program: 
// 
//     Starting with tutorial source code from Lazy Foo's online Tutorials for 
//     SDL 2.0 (September 2019), this program starts a minimal game engine.
//
//     The primary focus here is on a graphics layering system: 
//
//         1. background (1 of 27 colors)
//         2. graphics back level (bitmaps, sprites, primitives)
//         3. graphics mid level (bitmaps, sprites, primitives)
//         4. graphics front level (bitmaps, sprites, primitives)
//         5. textgrid background (60 x 45 color square grid, 0-26 each)
//         6. textgrid foreground (60 x 45 character grid, 0-255 each)
//         7. cursor level
//
//     Each frame (50 frames per second, so each loop = 20 ms), all these 
//     layers will have to be rendered to the screen in turn.
//     
//     This program only implements a subset of the above 7 layers.
//
//     A critical optimization that will have to be made for the textgrid 
//     layers: it will be much better to render these to a Texture target
//     so that the program doesn't have to iterate through the entire 2d
//     array each frame to render all that information to the screen.
//
//     There are many additional concepts also introduced here that were
//     not covered in previous tutorials:
//
//         a. Monitoring FPS by using SDL_Timer(s).
//         b. Implementing a Sprite structure to hold all data related
//            to each on-screen game object.
//         c. How to use color keying to blit Textures with transparent
//            pixels.
//         d. Full support for switching between windowed and full-
//            screen modes. This requires that all Textures be rebuilt
//            due to some low-end connection to the renderer, which 
//            must also be rebuilt each mode change.
//         e. Dropped support for Surfaces entirely.
//

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <ctime>    // to seed random number generator
#include <cstdlib>  // for rand() and srand() functions
#include <math.h>   // for round()

//Window dimensions (constants)
const int LOGICAL_SCREEN_WIDTH  = 480;    
const int LOGICAL_SCREEN_HEIGHT = 360;
const int TEXTGRID_WIDTH  = LOGICAL_SCREEN_WIDTH  / 8;  // 60 columns
const int TEXTGRID_HEIGHT = LOGICAL_SCREEN_HEIGHT / 8;  // 45 rows
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

//Global objects
SDL_Window*      window         = NULL;  
Uint32           window_pixel_format;
SDL_Rect         logical_window_rect;
int              current_graphics_mode;
SDL_Renderer*    windowRenderer = NULL;
SDL_RendererInfo renderer_info;
SDL_Event        input_event;
COLORS           letterbox_color; //shows in letterboxing and pillarboxing

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

    enum LAYERS {
        BACKGROUND_LAYER = 0,
        GRAPHICS_BOTTOM_LAYER,
        GRAPHICS_TOP_LAYER,
        TEXTGRID_BACKGROUND,
        LAYER_COUNT
    };

    //BACKGROUND LAYER
    COLORS background_layer_color;

    //GRAPHICS LAYERS
    const int MAX_SPRITES_PER_LAYER = 20;
    struct Sprite* graphics_bottom_layer_sprite[MAX_SPRITES_PER_LAYER];
    struct Sprite* graphics_top_layer_sprite[MAX_SPRITES_PER_LAYER];

    //TEXTGRID BACKGROUND (SOLID CELLS) 
    COLORS textgrid_background[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];

//Helper functions 
bool initializeEngine();
void shutdownEngine();
bool changeGraphicsMode(); 
SDL_Texture* createOptimizedTextureFromImageFile(const char* filename);
void initializeTextgridBackgroundArray();
bool processInput(void);                                        
void report_step(const char* s);
void rebuildSpriteArray(void);

//Helper variables
COLORS temp_color;
int temp_row, temp_col;
SDL_Rect temp_rect;

int main(int argc, char* args[]) {

    //Set everything that's independent of the window and renderer
    if(initializeEngine() == false) {
        printf(" Failed initializeEngine() call\n");
        fflush(stdout);
        return 1;
    }
    
    //initialize sprite arrays
    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        graphics_bottom_layer_sprite[i] = NULL;
        graphics_top_layer_sprite[i] = NULL;
    }

    //Initialize SDL system, create main window, link pointers to that main
    //window's Renderer and Surface.
    if(changeGraphicsMode() == false) {
        printf(" Failed changeGraphicsMode() call\n");
        fflush(stdout);
        return 1;
    }

    //Report what the native pixel format is of the Window
    window_pixel_format = SDL_GetWindowPixelFormat(window);
    const char* temp = SDL_GetPixelFormatName(window_pixel_format);
    printf(" SDL_Window created with pixel format %s\n", temp);
    fflush(stdout);
            
    //Display some information about the renderer        
    SDL_GetRenderDriverInfo(0, &renderer_info);
    printf(" Renderer name: %s\n", renderer_info.name);
    fflush(stdout);

    //set up initial graphics
    background_layer_color = BLUE;
    initializeTextgridBackgroundArray();
    
    //initialize textgrid background with some random squares
    for(int i = 0; i < 100; i++) {
        temp_col = rand() % TEXTGRID_WIDTH;
        temp_row = rand() % TEXTGRID_HEIGHT;
        temp_color = static_cast<COLORS>(rand() % COLOR_COUNT);
        textgrid_background[temp_row][temp_col] = temp_color;
    }

    //Loop until user quits.
    bool quit_program = false;
    while(quit_program == false) {
        
        // Start of logic section 
        logic_start_time = SDL_GetTicks();

        //Handle user input
        if(processInput() == false) {
            quit_program = true;
        }

        //Render BACKGROUND_LAYER
        SDL_SetRenderDrawColor(windowRenderer, 
                r_val[background_layer_color],
                g_val[background_layer_color],
                b_val[background_layer_color],
                0xFF);
        SDL_RenderFillRect(windowRenderer, &logical_window_rect);
        
        //Draw GRAPHICS_BOTTOM_LAYER 
        for(int i = 0; i < 1; i++) {
            if(graphics_bottom_layer_sprite[i]->visible == true) {
                SDL_RenderCopy(windowRenderer, 
                    graphics_bottom_layer_sprite[i]->texture, 
                    NULL,
                    &graphics_bottom_layer_sprite[i]->rect);
            }
        }
        
        //Draw GRAPHICS_TOP_LAYER 
        for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
            if(graphics_top_layer_sprite[i]->visible == true) {
                SDL_RenderCopy(windowRenderer, 
                    graphics_top_layer_sprite[i]->texture, 
                    NULL,
                    &graphics_top_layer_sprite[i]->rect);
            }
        }

        //Render TEXTGRID_BACKGROUND
        for(temp_row = 0; temp_row < TEXTGRID_HEIGHT; temp_row++) {
            for(temp_col = 0; temp_col < TEXTGRID_WIDTH; temp_col++) {
                if(textgrid_background[temp_row][temp_col] != EMPTY) {
                       
                    temp_rect = {temp_col*8, temp_row*8, 8, 8};
                    temp_color = textgrid_background[temp_row][temp_col];
                    SDL_SetRenderDrawColor(windowRenderer, 
                            r_val[temp_color],
                            g_val[temp_color],
                            b_val[temp_color],
                            0xFF);
                    SDL_RenderFillRect(windowRenderer, &temp_rect);
                }
            }
        }
        
        //Move sprites on top layer
        for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {

            graphics_top_layer_sprite[i]->rect.x +=
                 graphics_top_layer_sprite[i]->dx;
            
            graphics_top_layer_sprite[i]->rect.y +=
                 graphics_top_layer_sprite[i]->dy;

            if(graphics_top_layer_sprite[i]->rect.x +
               graphics_top_layer_sprite[i]->rect.w < 0) {
                   graphics_top_layer_sprite[i]->rect.x =
                       LOGICAL_SCREEN_WIDTH;
            } else if (graphics_top_layer_sprite[i]->rect.x >
                    LOGICAL_SCREEN_WIDTH) {
                   graphics_top_layer_sprite[i]->rect.x = 0 -
                       graphics_top_layer_sprite[i]->rect.w;
            }
            
            if(graphics_top_layer_sprite[i]->rect.y +
               graphics_top_layer_sprite[i]->rect.h < 0) {
                   graphics_top_layer_sprite[i]->rect.y =
                       LOGICAL_SCREEN_HEIGHT;
            } else if (graphics_top_layer_sprite[i]->rect.y >
                    LOGICAL_SCREEN_HEIGHT) {
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
                       LOGICAL_SCREEN_WIDTH;
            } else if (graphics_bottom_layer_sprite[i]->rect.x >
                    LOGICAL_SCREEN_WIDTH) {
                   graphics_bottom_layer_sprite[i]->rect.x = 0 -
                       graphics_bottom_layer_sprite[i]->rect.w;
            }
            
            if(graphics_bottom_layer_sprite[i]->rect.y +
               graphics_bottom_layer_sprite[i]->rect.h < 0) {
                   graphics_bottom_layer_sprite[i]->rect.y =
                       LOGICAL_SCREEN_HEIGHT;
            } else if (graphics_bottom_layer_sprite[i]->rect.y >
                    LOGICAL_SCREEN_HEIGHT) {
                   graphics_bottom_layer_sprite[i]->rect.y = 0 -
                       graphics_bottom_layer_sprite[i]->rect.h;
            }
        }

        //Calculate and record game-logic duration
        logic_end_time = SDL_GetTicks();
        logic_duration = (logic_end_time - logic_start_time);
        cumulative_logic_duration += logic_duration;

        //Render
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
    
    bool success = true; 
    
    // Standard C++ way of seeding random number generator
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
    report_step("RGB values calculated");

    //Build sprite arrays
    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        graphics_bottom_layer_sprite[i] = NULL;
        graphics_top_layer_sprite[i] = NULL;
    }

    //Set colors of letterbox background and logical screen background
    letterbox_color = GRAY;
    
    //Setup a Rect object with the window's logical pixel resolution.
    logical_window_rect.x = 0;
    logical_window_rect.y = 0;
    logical_window_rect.w = LOGICAL_SCREEN_WIDTH;
    logical_window_rect.h = LOGICAL_SCREEN_HEIGHT;

    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf(" SDL could not initialize (SDL_Error: %s)\n", 
                SDL_GetError());
        fflush(stdout);
        success = false;
    }
    report_step("SDL_Init() called");

    //Set graphics mode
    current_graphics_mode = FULLSCREEN_MODE;

    return success;
}

void shutdownEngine() {
    
    SDL_DestroyRenderer(windowRenderer);
    SDL_DestroyWindow(window);
    window = NULL;
    windowRenderer = NULL;

    IMG_Quit();
    SDL_Quit();
}

bool changeGraphicsMode() {
    
    bool success = true; 

    //Cleanup possible existing objects
    if(window != 0) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    if(windowRenderer != 0) {
        SDL_DestroyRenderer(windowRenderer);
        windowRenderer = NULL;
    }
   
    //Create main window
    if(current_graphics_mode == WINDOWED_MODE) {

        current_graphics_mode = FULLSCREEN_MODE;

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019", 
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 LOGICAL_SCREEN_WIDTH, LOGICAL_SCREEN_HEIGHT,
                 SDL_WINDOW_FULLSCREEN_DESKTOP);

    } else if(current_graphics_mode == FULLSCREEN_MODE) {

        current_graphics_mode = WINDOWED_MODE;

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019", 
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 LOGICAL_SCREEN_WIDTH, LOGICAL_SCREEN_HEIGHT,
                 SDL_WINDOW_SHOWN);
    }
        
    if(window == NULL) {

        printf(" Window could not be created (SDL Error: %s)\n", 
                SDL_GetError());
        fflush(stdout);
        success = false;
        
    } else {

        //Create renderer for window
        windowRenderer = SDL_CreateRenderer(window, -1, 
                SDL_RENDERER_PRESENTVSYNC |
                SDL_RENDERER_ACCELERATED  |
                SDL_RENDERER_TARGETTEXTURE);
	
        if(windowRenderer == NULL) {
                
            printf(" Renderer could not be created (SDL Error: %s)\n", 
                    SDL_GetError());
            fflush(stdout);
            success = false;

        } else {

            //Set logical rendering quality and dimensions 
            // 0 = "nearest"
            // 1 = "linear"
            // 2 = "best"
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            SDL_RenderSetLogicalSize(windowRenderer, 
                    LOGICAL_SCREEN_WIDTH, LOGICAL_SCREEN_HEIGHT);
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
    }
    
    //Hardware-based rendering to set up initial screen
    SDL_SetRenderDrawColor(windowRenderer, 
            r_val[letterbox_color], 
            g_val[letterbox_color], 
            b_val[letterbox_color], 0xFF);        
    SDL_RenderClear(windowRenderer); //effects entire fullscreen
    SDL_RenderPresent(windowRenderer);

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

    bool keep_program_running = true;

    //This will loop until no further input events are found in the 
    //event queue.
    while(SDL_PollEvent(&input_event) != 0) {
        
        if(input_event.type == SDL_QUIT) {
            keep_program_running = false;
        
        } else if(input_event.type == SDL_KEYDOWN) {        

            switch(input_event.key.keysym.sym) {

                case SDLK_ESCAPE:
                    keep_program_running = false;
                break;

                case SDLK_m:
                    changeGraphicsMode();
                break;

                case SDLK_1:
                    background_layer_color = 
                        static_cast<COLORS>(rand() % COLOR_COUNT);
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

                default:
                break;
            }
        }
    }

    return keep_program_running;
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
                (struct Sprite *)malloc(sizeof(struct Sprite));
            graphics_top_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile(
                        "../graphics/sprite_1.png");
            graphics_top_layer_sprite[i]->rect.x = 
                rand() % LOGICAL_SCREEN_WIDTH;
            graphics_top_layer_sprite[i]->rect.y = 
                rand() % LOGICAL_SCREEN_HEIGHT;
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
                        "../graphics/big_sprite.bmp");
        
        } else {

            graphics_bottom_layer_sprite[i] = 
                (struct Sprite *)malloc(sizeof(struct Sprite));
            graphics_bottom_layer_sprite[i]->texture =
                createOptimizedTextureFromImageFile(
                        "../graphics/big_sprite.bmp");
            graphics_bottom_layer_sprite[i]->rect.x = 
                rand() % LOGICAL_SCREEN_WIDTH;
            graphics_bottom_layer_sprite[i]->rect.y = 
                rand() % LOGICAL_SCREEN_HEIGHT;
            SDL_QueryTexture(graphics_bottom_layer_sprite[i]->texture,
                    NULL, NULL, 
                    &graphics_bottom_layer_sprite[i]->rect.w,
                    &graphics_bottom_layer_sprite[i]->rect.h);
            graphics_bottom_layer_sprite[i]->dx = (rand() % 3) - 1;
            graphics_bottom_layer_sprite[i]->dy = (rand() % 3) - 1;
            graphics_bottom_layer_sprite[i]->visible = true;
        }
    } 
}
