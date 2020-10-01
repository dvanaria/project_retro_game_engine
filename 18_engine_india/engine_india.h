#ifndef ENGINE_HOTEL
#define ENGINE_HOTEL

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>  // for printf() and fflush(stdout)
#include <cstdlib>  // for rand() and srand() functions
#include <math.h>   // for round()

// primary display specs
const int GAME_SCREEN_WIDTH = 320;   
const int GAME_SCREEN_HEIGHT = 200; 
const int FONT_WIDTH = 8;          
const int FONT_HEIGHT = 8;        
const int TEXTGRID_WIDTH  = 40;
const int TEXTGRID_HEIGHT = 25;
const int DESIRED_FPS = 25;

// color system (27 colors)
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
    NUM_COLORS,
    EMPTY 
};

// primary control functions
bool initialize_engine();
int  main_game_loop();
void shutdown_engine();

// user controls (of how engine functions)
extern bool show_spin_cycle;
extern bool keyboard_cursor_enabled;
extern bool text_foreground_enabled;  
extern bool text_background_enabled;  

// timer globals (for access within user programs)
extern Uint32 loop_start_time;
extern Uint32 loop_end_time;
extern Uint32 target_loop_duration; 
extern Uint32 calculated_loop_duration;
extern Uint32 cumulative_loop_duration;
extern Uint32 cumulative_frame_count;
extern Uint32 spin_cycle;

// text window for keyboard cursor and text insertion via keyboard
extern int TEXT_WINDOW_LEFT_COLUMN;
extern int TEXT_WINDOW_RIGHT_COLUMN;
extern int TEXT_WINDOW_TOP_ROW;
extern int TEXT_WINDOW_BOTTOM_ROW;
void draw_text_window_border(COLORS color);
void define_text_window(int x1, int y1, int x2, int y2); 

// Keyboard cursor. When enabled, the keyboard will control the on-screen
// cursor. This includes inserting text, deleting text, and cursor
// control keys such as arrow keys, RETURN, HOME, END, etc.
extern int keyboard_cursor_x;
extern int keyboard_cursor_y;

//TEXTGRID FOREGROUND (WHITE TEXT)
extern char textgrid_foreground[][TEXTGRID_WIDTH];  // ASCII value for cell
void clear_entire_textgrid(void);
void clear_textgrid_row(int r);
void print_to_textgrid(char* string, int r, int c);
    
//TEXTGRID BACKGROUND (SOLID COLOR BLOCK) 
extern COLORS textgrid_background[][TEXTGRID_WIDTH];  // COLOR for cell
void initialize_textgrid_background_array();  // empty out array

// Textgrid pixel locations for each cell 
extern SDL_Rect text_rect[][TEXTGRID_WIDTH];  // pre-defined rects

// key bindings (user changeable)
extern const SDL_Keycode KEY_TO_TOGGLE_SCREEN_MODE;
extern const SDL_Keycode KEY_TO_TOGGLE_WINDOW_SIZE;
extern const SDL_Keycode KEY_TO_EXIT_MAIN_LOOP;
    
// specific keys that will always be tracked (state: down or up)
enum KEYBOARD_CONTROL_KEYS {
    BACKSPACE = 0,    // SDLK_BACKSPACE
    DELETE,           // SDLK_DELETE
    DOWN_ARROW,       // SDLK_DOWN
    END,              // SDLK_END
    ESCAPE,           // SDLK_ESCAPE
    HOME,             // SDLK_HOME
    LEFT_ALT,         // SDLK_LALT
    LEFT_CTRL,        // SDLK_LCTRL
    LEFT_ARROW,       // SDLK_LEFT
    LEFT_SHIFT,       // SDLK_LSHIFT
    RIGHT_ALT,        // SDLK_RALT
    RIGHT_CTRL,       // SDLK_RCTRL
    RETURN,           // SDLK_RETURN
    RIGHT_ARROW,      // SDLK_RIGHT
    RIGHT_SHIFT,      // SDLK_RSHIFT
    SPACEBAR,         // SDLK_SPACE
    UP_ARROW,         // SDLK_UP
    NUM_CONTROL_KEYS
};

// array for instant access to control-key state (up/down)
extern bool keyboard_control_key_down[];  

// function to quickly get a string version of a color name, like "GRAY RED"
char *get_color_name(int i); // if string form is needed

// sound effects (for .wav files)
enum SOUND_EFFECTS {
    SFX_CURSOR_INSERT = 0,
    SFX_CURSOR_CONTROL_KEY,
    SFX_COMPUTER_ON,
    NUM_SOUND_EFFECTS = 95
};

void load_sound_effect_wav_file(const char *filename, int i);
void play_sound_effect(int i);

//SOLID BACKGROUND LAYER
extern COLORS background_layer_color;

//SPRITES
struct Sprite* create_sprite(const char *filename1, const char* filename2);
void move_sprite(struct Sprite* s); //auto-move by s->dx
void move_sprite(struct Sprite* s, int dx, int dy); //manual-move
bool overlap_test(struct Sprite* s);


#endif
