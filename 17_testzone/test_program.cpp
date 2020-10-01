//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: ../test_program.cpp
// Last Modified: Mon Jun 22, 2020  10:33PM
// LOC: 165
// Filesize: 7971 bytes

/*                                                                            / 
 * We want to play a sine wave of the form                                    /
 *                                                                            /
 *      y=sin(2πft)                                                           /
 *                                                                            /
 * Usually f would be cycles per second, and t would be seconds, but it is    /
 * easier for us to express these as cycles per sample and samples. Here are  /
 * our includes and some global variables for keeping track of things we'll   /
 * need.                                                                      / 
 *                                                                            /
 *  
 */                                                                            

 
#include "engine_hotel.h"

// setup for sine wave sound
#define FREQ 440 // the frequency we want 
unsigned int audio_pos; /* which sample we are up to: audio_pos acts as 
                           our t, keeping track of the sample we are up to. 
                           We store the position outside the callback so we 
                           can keep the sine wave produced at the end of 
                           one callback and the start of another 
                           continuous. */
int audio_len; /* how many samples left to play, stops when <= 0 */
float audio_freq; /* audio frequency in cycles per sample */
float audio_volume; /* audio volume, 0 - ~32000 */
SDL_AudioDeviceID dev;

// dummy callback 
void MyAudioCallback(void *data, Uint8* stream, int len) {
 
    // Play a sine wave ////////////////////////////////////////////////////   
    len /= 2; // 16 bit
    
    int i;
    
    Sint16* buf = (Sint16*)stream;
    
    for(i = 0; i < len; i++) { 
    
        buf[i] = audio_volume * sin(2 * M_PI * 
                audio_pos * audio_freq);
        audio_pos++;
    }
    
    audio_len -= len;
    // Play a sine wave (end) //////////////////////////////////////////////   
  
    return;
}


void play_audio(void) {

    SDL_PauseAudioDevice(dev, 0);
    while(audio_len > 0) {
        SDL_Delay(500);
    }
}


int main(int argc, char* args[]) {

    keyboard_cursor_enabled = true; 
    show_spin_cycle = true;

    initialize_engine();

    int i;


    // Sound work begins here...
    int numAudioDrivers = SDL_GetNumAudioDrivers();

    // print the list of audio backends 
    printf("[SDL] %d audio backends compiled into SDL:", numAudioDrivers);
    for(i=0;i<numAudioDrivers;i++) {
        printf(" \'%s\'", SDL_GetAudioDriver(i));
    }
    printf("\n");


/*
    // attempt to init audio 
    for(i=0;i<numAudioDrivers;i++) {
        if(!SDL_AudioInit(SDL_GetAudioDriver(i))) {
                break;
        }
    }
    if(i == numAudioDrivers) {
        printf("[SDL] Failed to init audio: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
   
    // print the audio driver we are using 
    printf("[SDL] Audio driver: %s\n", SDL_GetCurrentAudioDriver());

    // pass it 0 for playback 
    int numAudioDevices = SDL_GetNumAudioDevices(0);

    // print the audio devices that we can see 
    printf("[SDL] %d audio devices:", numAudioDevices);
    for(i = 0; i < numAudioDevices; i++) {
        printf(" \'%s\'", SDL_GetAudioDeviceName(i, 0)); // again, 0 for playback 
    }
    printf("\n");

    SDL_AudioSpec want, have;
    SDL_zero(want);

    // a general specification 
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1; // 1, 2, 4, or 6 
    want.samples = 4096; // power of 2, or 0 and env SDL_AUDIO_SAMPLES is used 
    want.callback = MyAudioCallback; // can not be NULL 

    printf(
    "[SDL] Desired - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d\n",         want.freq, SDL_AUDIO_ISFLOAT(want.format), SDL_AUDIO_ISSIGNED(want.format), 
        SDL_AUDIO_ISBIGENDIAN(want.format), SDL_AUDIO_BITSIZE(want.format), 
        want.channels, want.samples);

    // open audio device, allowing any changes to the specification 
    // Notice we are passing NULL as the first parameter, the audio device
    // name - this will allow SDL to try all available devices.
    // The second parameter is 0 (no recording, just playback)
    dev = 
      SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if(!dev) {
        printf("[SDL] Failed to open audio device: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf(
  "[SDL] Obtained - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d\n", 
          have.freq, SDL_AUDIO_ISFLOAT(have.format), SDL_AUDIO_ISSIGNED(have.format), 
          SDL_AUDIO_ISBIGENDIAN(have.format), SDL_AUDIO_BITSIZE(have.format), 
          have.channels, have.samples);

    textgrid_foreground[0][0] = 'A';    
    textgrid_foreground[44][59] = 'Z';    

    print_to_textgrid("Hello World, it is I, the Admiral!", 5, 20);

    textgrid_background[1][1] = RED;
    textgrid_background[43][58] = BLUE;
    textgrid_background[5][20] = CYAN;
    textgrid_background[5][21] = CYAN;
    textgrid_background[5][22] = CYAN;
    textgrid_background[0][0] = EMPTY;

    audio_len = 10;
    audio_pos = 0;
    audio_freq = 1.0 * FREQ / have.freq;
    audio_volume = 500;
    play_audio();

    audio_len = 10;
    audio_pos = 0;
    audio_freq = 1.0 * 880 / have.freq;
    audio_volume = 500;
    play_audio();
 

 */   
    SDL_CloseAudioDevice(dev);

    main_game_loop();
  
    SDL_Quit();
    
    shutdown_engine();

    return 0;
}

/*
void user_starting_loop(void) {}
void user_keyboard_key_up_handler(SDL_Keycode kc) {}
void user_keyboard_key_down_handler(SDL_Keycode kc) {}
void user_keyboard_alpha_numeric_handler(SDL_Keycode kc) {}
void user_create_all_textures(void) {}
void user_destroy_all_textures(void) {}
void user_gamepad_button_handler(SDL_Event e) {}
void user_update_sprites(void) {}
void user_collision_detection(void) {}
void user_render_graphics(void) {}
void user_ending_loop(void) {}
void user_shutdown(void) {}
*/

void user_starting_loop(void) {}

void user_keyboard_key_up_handler(SDL_Keycode kc) {
    // called on any key press (alpha-numeric or control key)
    printf("user keyboard key up called\n");
    fflush(stdout);
}
void user_keyboard_key_down_handler(SDL_Keycode kc) {
    // called on any key press (alpha-numeric or control key)
    printf("user keyboard key down called\n");
    fflush(stdout);
}

void user_keyboard_alpha_numeric_handler(SDL_Keycode kc) {

    // called when engine game loop receives an SDL_TEXTINPUT event
    printf("user keyboard key alpha/numeric called\n");
    fflush(stdout);

    if(kc == SDLK_a) {

        for(int i = 0; i < 10; i++) {
            print_to_textgrid("A was pressed!",5+i,10);
        }

    } else {

        int rx = rand() % 40;
        int ry = rand() % 40;

        print_to_textgrid("RANDOM", rx, ry);
    }
}

void user_create_all_textures(void) {}
void user_destroy_all_textures(void) {}
void user_gamepad_button_handler(SDL_Event e) {}
void user_update_sprites(void) {}
void user_collision_detection(void) {}
void user_render_graphics(void) {}

void user_ending_loop(void) {
    if(keyboard_control_key_down[LEFT_ARROW] == true) {
        printf("LEFT ARROW\n");
        fflush(stdout);
    }
}

void user_shutdown(void) {}
