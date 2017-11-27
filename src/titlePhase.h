/******************************************************************************
*  @file       	titlePhase.h
*  @brief      	Title phase handler
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 27, 2017
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
	vram_unrle(title_nam);
	
	// Load the palette
	pal_bg(palette);
	
	// Turn on the BG
	ppu_on_bg();
	
	while (1)
	{
		ppu_wait_frame();
		++frameCounter;
		
		// Toggle the colors of the start indicator
		pal_col(12, (frameCounter & 16) ? 0x0f : 0x0f);
		pal_col(13, (frameCounter & 16) ? 0x16 : 0x15);
		pal_col(14, (frameCounter & 16) ? 0x27 : 0x25);
		pal_col(15, (frameCounter & 16) ? 0x37 : 0x35);
		
		// Detect any button press to start the game
		if (pad_trigger(0))
		{
			break;
		}
	}
	
	// Fade out
	pal_fade_to(0);
}
