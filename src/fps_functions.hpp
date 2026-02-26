// fps_functions.hpp by InterChan

// In main loop add "fps_update();" which updates the variable fps10
// The int "fps10" represents the fps to one decimal place, except without the decimal point so 14.2 is just 142
// The "fps_formatted" string is the FPS to one decimal place, in the format "xx.x FPS"
// Use "fps_formatted_update();" to update the string to current fps10 before displaying it
// Use "fps_display();" to show the formatted FPS in the top left corner

#pragma once

#ifndef PC
#include <string.h>
#include <sdk/os/debug.h>
#else
#include <string.h>
#include <stdio.h> // For printf if debug used
#endif

// Extern declarations
#ifndef PC
extern uint8_t *R64CNT;
#endif
extern uint8_t prevtime;
extern int fps10;
extern char fps_formatted[9];

// Function prototypes
void fps_update();
void fps_formatted_update();

inline void fps_display() {
#ifndef PC
    Debug_Printf(0,0,false,0,fps_formatted);
#else
    printf("%s\n", fps_formatted);
#endif
}
