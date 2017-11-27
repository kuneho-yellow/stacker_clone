/******************************************************************************
*  @file       	main.c
*  @brief      	Main game code file
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 27, 2017
*      
*  @par [explanation]
*		> Used for global variable declarations, defines, and other
*		code for general use throughout game
******************************************************************************/
 
#include "lib/neslib.h"

// Put all the subsequent global vars into zeropage

#pragma bss-name (push,"ZEROPAGE")
#pragma data-name(push,"ZEROPAGE")

// Set of general purpose global vars that are used everywhere in the program
// This makes code faster and shorter, although not very convenient and readable
static unsigned char i;				// Looping variable
static unsigned char j;				// Looping variable
static unsigned char frameCounter;	// Tracks elapsed frames
static unsigned char gameResult;	// Tracks game result
static unsigned int var16Bit;		// General variable for 16-bit computations
static unsigned char bright;		// Used in fade functions (pal_fade_to

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
#include "resultPhase.h"

// Program entry-point
void main(void)
{
	 // Game loop
	while (1)
	{
		titlePhase();
		gamePhase();		
		resultPhase();
	}
}
