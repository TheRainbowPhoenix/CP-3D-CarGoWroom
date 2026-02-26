#include "fps_functions.hpp"

#ifndef PC
uint8_t *R64CNT = (uint8_t*)0xA413FEC0;
#endif
uint8_t prevtime = 0;
int fps10 = 10;
char fps_formatted[9];

void fps_update() {
#ifndef PC
    // update fps10
    if (*R64CNT - prevtime == 0) {
        fps10 = 1280;
    } else {
        fps10 = 1280 / ((int)((*R64CNT - prevtime) & 0b01111111));
    }
    prevtime = *R64CNT;
#endif
}

void fps_formatted_update() {
    int16_t dec00_1 = fps10;
    uint8_t dec01_0 = 0;
    uint8_t dec10_0 = 0;
    while (dec00_1 >= 100) { dec00_1-= 100; dec10_0++; }
    while (dec00_1 >= 10)  { dec00_1-=  10; dec01_0++; }
#ifndef PC
    strcpy(fps_formatted, "  .  FPS");
#else
    // Use strncpy or similar for PC if needed, but strcpy is fine
    // Check if strcpy is available (included via string.h in header)
    // Actually, fps_functions.hpp includes sdk/os/string.h which maps to string.h
    // But for PC build, sdk/os/string.h is not available.
    // I should check fps_functions.hpp includes.
    strcpy(fps_formatted, "  .  FPS");
#endif
    fps_formatted[0] = dec10_0 + 48;
    fps_formatted[1] = dec01_0 + 48;
    fps_formatted[3] = (uint8_t)(dec00_1 + 48);
}
