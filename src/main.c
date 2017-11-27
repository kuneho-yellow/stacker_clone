/******************************************************************************
*  @file       	main.c
*  @brief      	Main game code file
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 26, 2017
*      
*  @par [explanation]
*		> Used for global variable declarations, defines, and other
*		code for general use throughout game
******************************************************************************/
 
#include "lib/neslib.h"

// Game uses 12:4 fixed point calculations
#define FP_BITS	4

// Movement directions, mapped to gamepad button bits
#define DIR_NONE		0
#define DIR_LEFT		PAD_LEFT
#define DIR_RIGHT		PAD_RIGHT
#define DIR_UP			PAD_UP
#define DIR_DOWN		PAD_DOWN

// Put all the subsequent global vars into zeropage

#pragma bss-name (push,"ZEROPAGE")
#pragma data-name(push,"ZEROPAGE")

// Set of general purpose global vars that are used everywhere in the program
// This makes code faster and shorter, although not very convenient and readable
static unsigned char i;
static unsigned char j;
/*
static unsigned char frameCounter;
static unsigned char input;
static unsigned char wait;

static unsigned char px;
static unsigned char py;
static unsigned char ptr;
static unsigned char spr;
*/
static unsigned int i16;

// Used in fade functions (pal_fade_to, game loop fade)
static unsigned char bright;


#pragma data-name(pop)
#pragma bss-name (pop)

// Following variables will go to the default RAM location (BSS)

// Game Palette
const unsigned char palette[16]={ 0x0f,0x00,0x10,0x30,0x0f,0x11,0x21,0x31,0x0f,0x15,0x25,0x35,0x0f,0x16,0x27,0x37 };

// Include sound and music handler
#include "soundsAndMusic/soundsAndMusic.h"

// Smoothly fade current bright to the given value
// When to=0, stop music, turn display off, reset vram update and scroll
void pal_fade_to(unsigned to)
{
	if (!to) music_stop();

	while (bright != to)
	{
		delay(4);
		if (bright<to) 	++bright;
		else 			--bright;
		pal_bright(bright);
	}

	if (!bright)
	{
		ppu_off();
		set_vram_update(NULL);
		scroll(0,0);
	}
}

#include "gameConstants.h"
#include "titlePhase.h"
#include "gamePhase.h"

// Program entry-point
void main(void)
{
	while (1) // Infinite loop
	{
		titlePhase();
		
		pal_fade_to(0);
		
		gamePhase();
	}
}
