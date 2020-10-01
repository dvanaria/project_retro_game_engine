//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../09_engine_charlie.cpp
// Last Modified: Fri Oct 04, 2019  11:53AM
// LOC: 668
// Filesize: 31686 bytes
//
// This program: 
//
//     Fullscreen needs to be fixed. Using the SDL feature "Desktop Fullscreen"
//     sounds great on paper, but it seems to be impossible to get pixel-
//     perfect scaling. You set a "hint" that does a pretty good job, but when
//     you compare it to whole number scaling, it looks terrible.
//
//     When using the SDL feature SDL_RenderSetScale(windowRenderer, 2, 2); the
//     scaled image is just as solid and flicker/blurry free as the original.
//     It is fluid and looks professional.
//
//     So here's the goal for fullscreen:
//
//         a. Logical window size will always be 480 x 360.
// 
//         b. Windowed mode will just use the logical window dimensions to
//            set the window size. Fullscreen mode will use the Desktop's
//            native resolution 1366 x 768.
//
//         c. The rendered fullscreen will be a large-as-possible scaling,
//            depending on the native desktop resolution. Say it is x2. That
//            would equal a RenderSetScale(2,2). That screen would then be
//            centered on the larger 1366 x 768, requiring some offset
//            rendering complexity, but it will be worth it.
//
//     Then, the overall goal for layering system:
//
//         0. letterbox (gray) 
//         1. background (1 of 27 colors)
//         2. graphics bottom level (bitmaps, sprites, primitives)
//         3. graphics top level (bitmaps, sprites, primitives)
//         4. textgrid background (60 x 45 color square grid, 0-26 each)
//         5. textgrid foreground (60 x 45 character grid, 0-255 each)
//         6. cursor level
//
//     Each frame (50 frames per second, so each loop = 20 ms), all these 
//     layers will have to be rendered to the screen in turn.
//     

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <ctime>    // to seed random number generator
#include <cstdlib>  // for rand() and srand() functions
#include <math.h>   // for round()

//Engine parameters
const int LOGICAL_SCREEN_WIDTH  = 480;    
const int LOGICAL_SCREEN_HEIGHT = 360;
int       NATIVE_SCREEN_WIDTH = 0; //set during initialization
int       NATIVE_SCREEN_HEIGHT = 0;
int       SCALING_FACTOR = 0;      //set during initialization
const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 8;
const int TEXTGRID_WIDTH = LOGICAL_SCREEN_WIDTH / FONT_WIDTH;    //60 columns
const int TEXTGRID_HEIGHT = LOGICAL_SCREEN_HEIGHT / FONT_HEIGHT; //45 rows
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
SDL_Window*      window          = NULL;  
SDL_Renderer*    windowRenderer  = NULL;
Uint32           window_pixel_format;
SDL_Rect         logical_window_rect;
SDL_RendererInfo renderer_info;
SDL_Event        input_event;
COLORS           letterbox_color; //shows in letterboxing and pillarboxing
int              current_graphics_mode;
SDL_Texture*     target_texture;  //used for fullscreen rendering
SDL_Rect         target_texture_rect;  //where the target texture lives

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
    SDL_Rect      glyph_rect[NUM_GLYPHS]; 
    SDL_Texture*  glyph_sheet = NULL;
    char          text[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];  //ASCII data per cell
    SDL_Rect      text_rect[TEXTGRID_HEIGHT][TEXTGRID_WIDTH];
    char          text_index; 
    char          temp_string[256];  //for writing formatted strings
    int           string_index;
    void initializeTextSystem(void); 
    void renderTextgrid(void);
    void clearTextgrid(void);
    void clearTextgridRow(int r);
    void printToTextgrid(char* string, int r, int c);

//Engine control functions 
bool         initializeEngine();
void         shutdownEngine();
bool         changeGraphicsMode(); 
SDL_Texture* createOptimizedTextureFromImageFile(const char* filename);
bool         processInput(void);                                        
void         rebuildSpriteArray(void);
void         report_step(const char* s);

//Helper variables
COLORS temp_color;
int temp_row, temp_col;
SDL_Rect temp_rect;
bool text_visible = false;

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
    printToTextgrid("ENGINE CHARLIE", 16, 24);

    //Loop until user quits.
    bool quit_program = false;
    while(quit_program == false) {
        
        // Start of logic section 
        logic_start_time = SDL_GetTicks();

        //Handle user input
        if(processInput() == false) {
            quit_program = true;
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
        SDL_RenderFillRect(windowRenderer, &logical_window_rect);

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
    report_step("RGB values calculated");
    
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

    //Build sprite arrays
    for(int i = 0; i < MAX_SPRITES_PER_LAYER; i++) {
        graphics_bottom_layer_sprite[i] = NULL;
        graphics_top_layer_sprite[i] = NULL;
    }

    //Set colors of letterbox background and logical screen background
    letterbox_color = GRAY;
    
    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf(" SDL could not initialize (SDL_Error: %s)\n", 
                SDL_GetError());
        fflush(stdout);
        success = false;
    }
    report_step("SDL_Init() called");

    //Set graphics mode
    current_graphics_mode = FULLSCREEN_MODE;  //flip-flops on entry
    
    //Capture native desktop resolution
    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        printf(" SDL_GetDesktopDisplayMode failed: %s", 
                SDL_GetError());
        fflush(stdout);
        return 1;
    }
    Uint32 f = dm.format;
    printf(" Native Resolution: bpp %i, %s, %i x %i\n", 
            SDL_BITSPERPIXEL(f), 
            SDL_GetPixelFormatName(f), 
            dm.w, dm.h);
    fflush(stdout);
    NATIVE_SCREEN_WIDTH = dm.w;   //setting global parameters here
    NATIVE_SCREEN_HEIGHT = dm.h;

    //Calculate and set scale factor to maximize screen size
    int scale_x;
    int scale_y;
    scale_x = (int)(NATIVE_SCREEN_WIDTH / LOGICAL_SCREEN_WIDTH);
    scale_y = (int)(NATIVE_SCREEN_HEIGHT / LOGICAL_SCREEN_HEIGHT);
    int final_scale = 0;
    if(scale_y > scale_x)
        final_scale = scale_x;
    else
        final_scale = scale_y;
    SCALING_FACTOR = final_scale;
    printf(" SCALING FACTOR set to %d\n", SCALING_FACTOR);
    fflush(stdout);

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

    //Not sure if this is needed, weird bug
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

    //Cleanup possible existing objects
    if(window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    if(windowRenderer != NULL) {
        SDL_DestroyRenderer(windowRenderer);
        windowRenderer = NULL;
    }
   
    //Create main window 
    if(current_graphics_mode == WINDOWED_MODE) {

        //Switch to Fullscreen mode:
        current_graphics_mode = FULLSCREEN_MODE;

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019", 
                 0,0,
                 NATIVE_SCREEN_WIDTH, NATIVE_SCREEN_HEIGHT,
                 SDL_WINDOW_FULLSCREEN_DESKTOP);

        //Use the native desktop resolution to calculate where the logical
        //screen should be located:
        target_texture_rect.x = 
            (int)((NATIVE_SCREEN_WIDTH - 
                       (LOGICAL_SCREEN_WIDTH * SCALING_FACTOR)) / 2) /
                        SCALING_FACTOR;
        target_texture_rect.y = 
            (int)((NATIVE_SCREEN_HEIGHT - 
                       (LOGICAL_SCREEN_HEIGHT * SCALING_FACTOR)) / 2) /
                        SCALING_FACTOR;
        target_texture_rect.w = LOGICAL_SCREEN_WIDTH;
        target_texture_rect.h = LOGICAL_SCREEN_HEIGHT;
    
    } else if(current_graphics_mode == FULLSCREEN_MODE) {

        //Switch to Windowed mode
        current_graphics_mode = WINDOWED_MODE;

        window = SDL_CreateWindow("SDL2/C++ Game Engine 2019", 
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                 LOGICAL_SCREEN_WIDTH * SCALING_FACTOR, 
                 LOGICAL_SCREEN_HEIGHT * SCALING_FACTOR,
                 SDL_WINDOW_SHOWN);
    }
        
    //Setup a Rect object with the window's logical pixel resolution.
    logical_window_rect.x = 0;
    logical_window_rect.y = 0;
    logical_window_rect.w = LOGICAL_SCREEN_WIDTH;
    logical_window_rect.h = LOGICAL_SCREEN_HEIGHT;

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
            //rendering 
            target_texture = SDL_CreateTexture(
                    windowRenderer, 
                    window_pixel_format,
		            SDL_TEXTUREACCESS_TARGET, 
                    LOGICAL_SCREEN_WIDTH, LOGICAL_SCREEN_HEIGHT);    

            //clear entire fullscreen desktop, only needs to be done once 
            SDL_SetRenderDrawColor(windowRenderer,
                    r_val[letterbox_color],
                    g_val[letterbox_color],
                    b_val[letterbox_color],
                    0xFF);
            SDL_RenderClear(windowRenderer);
            SDL_RenderPresent(windowRenderer);
           
        } else if(current_graphics_mode == WINDOWED_MODE) {

            //Create renderer for window
            windowRenderer = SDL_CreateRenderer(window, -1, 
                    SDL_RENDERER_PRESENTVSYNC |
                    SDL_RENDERER_ACCELERATED
                    );
           
            //Make certain to set main window as render target 
            SDL_SetRenderTarget(windowRenderer, NULL);
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

    //Set scaling here
    SDL_RenderSetScale(windowRenderer, SCALING_FACTOR, SCALING_FACTOR);

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
                (struct Sprite *)malloc(sizeof(struct Sprite));   //LEAK HERE!!!
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
       
            text_index = text[r][c]; // 0-255 

            if(text_index != ' ') {
                SDL_RenderCopy(windowRenderer, glyph_sheet, 
                        &glyph_rect[text_index], 
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
