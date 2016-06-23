#ifndef _PPM_H
#define _PPM_H


// print joystick axis values on serial port
#define DEBUG_USB

#define PPM_LOW 800
#define PPM_MIDDLE 1500
#define PPM_HIGH 2200

// Dead-time between each channel in the PPM-stream.
#define DEAD_TIME 900

// Number of PPM channels out. 1 - 12 channels supported (both incl).
#define NUMBER_OF_CHANNELS 8

// Set frame-length depending on channels
#define FRAME_LENGTH 5003+NUMBER_OF_CHANNELS*5000

#endif
