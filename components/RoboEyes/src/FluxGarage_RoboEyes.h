/*
 * FluxGarage RoboEyes for OLED Displays V 1.1.1
 * Header file for FluxGarage_RoboEyes.c
 * Provides declarations for RoboEyes functions and constants.
 *
 * Copyright (C) 2024-2025 Dennis Hoelscher
 * www.fluxgarage.com
 * www.youtube.com/@FluxGarage
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _FLUXGARAGE_ROBOEYES_H
#define _FLUXGARAGE_ROBOEYES_H

#include <stdint.h>
#include <stdbool.h>

// Constants for mood types
#define DEFAULT 0
#define TIRED 1
#define ANGRY 2
#define HAPPY 3

// Constants for on/off states
#define ON 1
#define OFF 0

// Constants for predefined positions
#define N 1  // North, top center
#define NE 2 // North-east, top right
#define E 3  // East, middle right
#define SE 4 // South-east, bottom right
#define S 5  // South, bottom center
#define SW 6 // South-west, bottom left
#define W 7  // West, middle left
#define NW 8 // North-west, top left

typedef void (*DrawRoundedRectangleFunc)(int x, int y, int width, int height, int borderRadius, uint8_t color);
typedef void (*ClearDisplayFunc)();
typedef void (*UpdateDisplayFunc)();
typedef void (*DrawTriangleFunc)(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color);
typedef uint32_t (*MillisFunc)();
typedef uint32_t (*RandomFunc)(uint32_t limit);

// Function declarations
void RoboEyes_init(DrawRoundedRectangleFunc DrawRoundedRectangle,
    DrawTriangleFunc DrawTriangle,
    ClearDisplayFunc ClearDisplay,
    UpdateDisplayFunc UpdateDisplay,
    MillisFunc Millis,
    RandomFunc Random
);
void RoboEyes_begin(int width, int height, uint8_t frameRate);
void RoboEyes_update();
void RoboEyes_setFramerate(uint8_t fps);
void RoboEyes_setDisplayColors(uint8_t background, uint8_t main);
void RoboEyes_setWidth(uint8_t leftEye, uint8_t rightEye);
void RoboEyes_setHeight(uint8_t leftEye, uint8_t rightEye);
void RoboEyes_setBorderradius(uint8_t leftEye, uint8_t rightEye);
void RoboEyes_setSpacebetween(int space);
void RoboEyes_setMood(uint8_t mood);
void RoboEyes_setPosition(uint8_t position);

int RoboEyes_getScreenConstraint_X();
int RoboEyes_getScreenConstraint_Y();
void RoboEyes_setAutoblinker2(bool active, int interval, int variation);
void RoboEyes_setAutoblinker(bool active);
void RoboEyes_setIdleMode2(bool active, int interval, int variation);
void RoboEyes_setIdleMode(bool active);
void RoboEyes_setCuriosity(bool curiousBit);
void RoboEyes_setCyclops(bool cyclopsBit);
void RoboEyes_setHFlicker2(bool flickerBit, uint8_t Amplitude);
void RoboEyes_setHFlicker(bool flickerBit);
void RoboEyes_setVFlicker2(bool flickerBit, uint8_t Amplitude);
void RoboEyes_setVFlicker(bool flickerBit);
void RoboEyes_setSweat(bool sweatBit);
void RoboEyes_close();
void RoboEyes_open();
void RoboEyes_blink();
void RoboEyes_close2(bool left, bool right);
void RoboEyes_open2(bool left, bool right);
void RoboEyes_blink2(bool left, bool right);
void RoboEyes_anim_confused();
void RoboEyes_anim_laugh();

#endif // _FLUXGARAGE_ROBOEYES_H
