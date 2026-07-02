// main loop
#include "chip8.h"
#include <SDL2/SDL.h>

void precompute_keymap(uint8_t keymap[], int size){
    memset(keymap, 16, size);
    keymap['1'] = 1;
    keymap['2'] = 2;
    keymap['3'] = 3;
    keymap['4'] = 0xC;
    keymap['q'] = 4;
    keymap['w'] = 5;
    keymap['e'] = 6;
    keymap['r'] = 0xD;
    keymap['a'] = 7;
    keymap['s'] = 8;
    keymap['d'] = 9;
    keymap['f'] = 0xE;
    keymap['z'] = 0xA;
    keymap['x'] = 0;
    keymap['c'] = 0xB;
    keymap['v'] = 0xF;
}

void audio_callback(void* userdata, Uint8* stream, int len){
    Chip8* chip8 = (Chip8*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / sizeof(int16_t);
    int amplitude = 3000;

    static int s = 0;

    for(int i = 0; i < samples; i++){
        if(chip8->sound_timer > 0){
            int samples_per_cycle = 44100 / 440;
            int position_in_cycle = s % samples_per_cycle;
            buffer[i] = (position_in_cycle < samples_per_cycle / 2) ? amplitude : -amplitude;
        } else {
            buffer[i] = 0;
        }
        s++;
    }
}

int main(int argc, char* argv[]){

    if(argc == 1){
        printf("Please specify the path, ex : ./main /path/to/game");
        return 1;
    }

    // initialize the processor
    Chip8 mainChip;
    init_chip8(&mainChip);

    load_chip8(&mainChip, argv[1]);

    // initialize the game window

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);

    // initialize the renderer

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // initialize texture

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING, 64, 32);

    // initialize sound

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;
    want.userdata = &mainChip;

    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    // precompute the key map
    uint8_t keymap[256];

    precompute_keymap(keymap, sizeof(keymap));

    // the main loop
    uint32_t cur_time, l_time = 0, delta_time;
    float cpu_timer = 0;
    float timer_acc = 0;
    bool running = true;
    while(running){

        // poll events
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){
                running = false;
            }
            if(e.type == SDL_KEYDOWN){
                if(e.key.keysym.sym >= 0 && e.key.keysym.sym <= 255
                    && keymap[e.key.keysym.sym] != 16)
                    mainChip.keypad[keymap[e.key.keysym.sym]] = 1;
            }
            if(e.type == SDL_KEYUP){
                if(e.key.keysym.sym >= 0 && e.key.keysym.sym <= 255
                    && keymap[e.key.keysym.sym] != 16)
                    mainChip.keypad[keymap[e.key.keysym.sym]] = 0;
            }
        }

        cur_time = SDL_GetTicks();
        delta_time = cur_time - l_time;
        l_time = cur_time;

        if(delta_time > 0) {
            cpu_timer += delta_time;
            timer_acc += delta_time;
        }

        if(cpu_timer > 1.4) {
            cpu_timer -= 1.4;
            fetch_chip8(&mainChip);
            decode_execute_chip8(&mainChip);
        }

        if(timer_acc > 16.6){
            timer_acc -= 16.6;
            if(mainChip.delay_timer > 0)
                mainChip.delay_timer--;
            if(mainChip.sound_timer > 0)
                mainChip.sound_timer--;

            // draw the texture
            SDL_UpdateTexture(texture, NULL, mainChip.video, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            // handle the sound

            SDL_PauseAudioDevice(audioDevice, mainChip.sound_timer > 0 ? 0 : 1);

        }
    }

    // cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}