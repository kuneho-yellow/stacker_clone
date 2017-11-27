/******************************************************************************
*  @file       	titlePhase.h
*  @brief      	Title phase handler
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 26, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the title phase
******************************************************************************/

// Title screen nametable
#include "nametables/title.h"

void titlePhase(void)
{
	// Load the nametable
	vram_adr(NAMETABLE_A);
	vram_unrle(title);
	
	// Load the palette
	pal_bg(palette);
	
	// Turn on the BG
	ppu_on_bg();
	
	while (1)
	{
		// Detect any button press to start the game
		if (pad_trigger(0))
		{
			break;
		}
	}
	
	// Fade out
	pal_fade_to(0);
}
