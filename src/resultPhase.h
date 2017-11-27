/******************************************************************************
*  @file       	resultPhase.h
*  @brief      	Title phase handler
*  @author     	Lori
*  @created 	November 27, 2017
*  @modified   	November 27, 2017
*      
*  @par [explanation]
*		> Holds code displaying game results
******************************************************************************/

// Title screen nametable
#include "nametables/hud.h"

// Constants
#define COLOR_SWAP_FRAME_BIT 8

void resultPhase(void)
{
	// Game fail screen
	if (!gameResult)
	{
		// Turn off rendering
		ppu_off();
		
		// Display the failure message
		vram_adr(0x2189);
		vram_write((unsigned char*)fail_nam1, 14);
		vram_adr(0x21a9);
		vram_write((unsigned char*)fail_nam2, 14);
		vram_adr(0x21c9);
		vram_write((unsigned char*)fail_nam3, 14);
		
		// Change the colors of the blocks
		pal_col(4, 0x0f);
		pal_col(5, 0x16);
		pal_col(6, 0x27);
		pal_col(7, 0x37);
		
		// Turn on the ppu when ready
		ppu_wait_frame();
		ppu_on_all();
		
		// Play the lose bgm
		music_play(MUSIC_LOSE);
		
		// Wait for any input to go back to the title screen
		while(1)
		{
			// Wait for the frame to finish
			ppu_wait_frame();
			++frameCounter;
			
			// Animate the BG via CHR bank switching
			bank_bg((frameCounter >> 4)&1);
		
			if (pad_trigger(0))
			{
				break;
			}
		}
	}
	// Game win screen
	else
	{
		// Play the lose bgm
		music_play(MUSIC_WELL_DONE);
		
		while(1)
		{
			// Wait for the frame to finish
			ppu_wait_frame();
			++frameCounter;
			
			// Toggle the colors of the goal indicator
			pal_col(8, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x0f : 0x0f);
			pal_col(9, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x15 : 0x16);
			pal_col(10, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x25 : 0x27);
			pal_col(11, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x35 : 0x37);
			
			// Toggle the colors of the blocks
			pal_col(4, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x0f : 0x0f);
			pal_col(5, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x11 : 0x16);
			pal_col(6, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x21 : 0x27);
			pal_col(7, (frameCounter & COLOR_SWAP_FRAME_BIT) ? 0x31 : 0x37);
			
			// Animate the BG via CHR bank switching
			bank_bg((frameCounter >> 2)&1);
			
			// Wait for any input to go back to the title screen
			if (pad_trigger(0))
			{
				break;
			}
		}
	}
	
	// Fade out
	pal_fade_to(0);
}
