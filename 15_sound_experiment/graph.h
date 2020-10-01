//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
// Filename: graph.h
// Last Modified: Sun Nov 03, 2019  07:46AM
// LOC: 50
// Filesize: 1594 bytes
 
#ifndef GRAPH_H
#define GRAPH_H
#include "SDL.h"
#include "SDL_audio.h"
#include <stdio.h>
#include <cmath>
#include <string>
#include <stack>

/* Constants */
const int REFRESH_INTERVAL = 50;                // mseconds
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 255;
const std::string WINDOW_TITLE = "Wave Graph";

class Graph
{
private:
    SDL_Window *window;          // Declare a pointer

    // SDL audio stuff
    SDL_AudioSpec desiredDeviceSpec;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;

    int thread_exit = 0;
    bool pause_thread = false;

public:
    Graph();
    void init();
    void mainLoop();
    void drawGraph();

    void exit();
    SDL_AudioSpec* getSpec();

    struct Voice{
        int frequency;              // the frequency of the voice
        int amp;                    // the amplitude of the voice

        // number of samples to be played, eg: 1.2 seconds * 44100 samples 
        // per second
        int audioLength;            

        int audioPosition = 0;      // counter

        enum WaveForm{
            SINE = 0, SQUARE = 1, SAWTOOTH = 2, TRIANGLE = 3
        } waveForm;


        uint8_t getSample();
    } voice;
    int graphPointer = 0;
    uint8_t graphBuffer[1000];


};


#endif
