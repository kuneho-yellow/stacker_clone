/******************************************************************************
*  @file       	gamePhase.h
*  @brief      	Game phase handler
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 26, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the game phase
******************************************************************************/

// Game screen nametable
#include "nametables/game.h"

// Game metasprites
const unsigned char block_metasprite[] = {
	  0,-17,0x40,1,
	  8,-17,0x41,1,
	  0,- 9,0x42,1,
	  8,- 9,0x43,1,
	128
};

// Pre-initialized update list used during gameplay
#define UPDATE_LIST_SIZE 23
const unsigned char updateListData[UPDATE_LIST_SIZE] =
{
	MSB(NAMETABLE_C) | NT_UPD_HORZ, LSB(NAMETABLE_C),
	8,0x40,0x41,0x40,0x41,0x40,0x41,0x40,0x41,

	MSB(NAMETABLE_C) | NT_UPD_HORZ, LSB(NAMETABLE_C),
	8,0x42,0x43,0x42,0x43,0x42,0x43,0x42,0x43,
	
	// End of file marker for VRAM update list
	NT_UPD_EOF
};

// Update list
static unsigned char updateList[UPDATE_LIST_SIZE];

// Constants
#define CENTER_X 128			// The x coordinate value of the center
#define BASE_Y 208				// The y coordinate value at the base
#define	INIT_SPEED 16			// Movement in bits/frame
#define INCREMENT_SPEED 0//8		// The increase in speed every stack
#define BLOCK_SIZE 16			// Size of one side of each block
#define SCREEN_WIDTH 256		// Should be the NES resolution
#define SCREEN_MIN BLOCK_SIZE
#define SCREEN_MAX SCREEN_WIDTH - BLOCK_SIZE
#define INIT_BLOCK_COUNT 4		// Initial no. of blocks
#define WIN_STACK_HEIGHT 10		// Height of the stack needed to win
#define TILE_SIZE_BIT 4			//
#define TILE_PLUS_FP_BITS (TILE_SIZE_BIT + FP_BITS)

// Variables
static unsigned int blockX;			// Current block x-position
static unsigned char blockTileX;	// Block coordinates
static unsigned char blockTileY;
static unsigned char blockSpeed; 	// Current movmement speed
static unsigned char blockCount;	// Current block count
static unsigned char blockGroupWidth; // Current block group width
static unsigned char direction;		// Current movement direction
static unsigned char stackHeight;	// Current stack height
static unsigned char isMoving;
static unsigned char minStackableX;	// The minimum x coord that allows stacking


void gamePhase(void)
{	
	// Clear sprites
	oam_clear();

	// Load the nametable
	vram_adr(NAMETABLE_A);
	vram_unrle(game);
	
	// Load the palettes
	pal_bg(palette);
	pal_spr(palette);
	
	// Turn on the screen with normal brightness
	pal_bright(4);
	ppu_on_all();
	
	// Initialize the game variables
	blockX = CENTER_X << FP_BITS;
	blockTileX = CENTER_X >> TILE_SIZE_BIT;
	blockTileY = BASE_Y >> TILE_SIZE_BIT;
	blockSpeed = INIT_SPEED;
	blockCount = INIT_BLOCK_COUNT;
	blockGroupWidth = BLOCK_SIZE * (blockCount - 1);
	direction = 1;
	stackHeight = 0;
	minStackableX = 0;
	isMoving = 1;
	
	// Set up update list
	memcpy(updateList, updateListData, sizeof(updateListData));
	set_vram_update(updateList);
	
	while (1)
	{
		// Display the moving blocks
		for (i = 0; i < blockCount; ++i)
		{
			oam_meta_spr((blockTileX + i) << TILE_SIZE_BIT,
				blockTileY << TILE_SIZE_BIT,
				i << 4,
				block_metasprite);
		}
		
		// Wait for the frame to finish
		ppu_wait_frame();
//		++frameCounter;
		
		if (isMoving)
		{
			// Compute new block positions
			if (direction)
			{
				blockX += blockSpeed;
			}
			else
			{
				blockX -= blockSpeed;
			}
		}

		blockTileX = blockX >> TILE_PLUS_FP_BITS;
		if ((((blockX & 0x00f0) >> FP_BITS)) >= 8)
		{	
			blockTileX += 1;
		}
				
		// Check if block has hit the edges
		if ((blockX >> FP_BITS) <= SCREEN_MIN ||
			(blockX >> FP_BITS) + BLOCK_SIZE >= (SCREEN_MAX - blockGroupWidth))
		{
			// Hit an edge
			direction ^= 1;
		}
		
		// Check for any player input
		if (pad_trigger(0))
		{
			//isMoving = 0;
			
			if (stackHeight < 1)
			{
				minStackableX = blockTileX;
			}
			
			// Remove floating blocks
			if (blockTileX < minStackableX)
			{
				// No. of extra blocks to the left
				j = minStackableX - blockTileX;
				if (j > blockCount)
				{
					j = blockCount;
				}
				
				// Replace all tiles with the empty tile
				/*for (i = 0; i < j; ++i)
				{
					updateList[3 + (i<<1)] = 0x00;
					updateList[4 + (i<<1)] = 0x00;
					updateList[14 + (i<<1)] = 0x00;
					updateList[15 + (i<<1)] = 0x00;
				}*/
				/*
				for (i = 0; i < (j << 1); ++i)
				{
					updateList[3 + i] = 0x00;
					updateList[14 + i] = 0x00;
				}
				*/
								
				// Update the block count
				blockCount -= j;
			}/*
			else if (blockTileX > minStackableX)
			{
				// No. of extra blocks to the right
				j = blockTileX - minStackableX;
				if (j > blockCount)
				{
					j = blockCount;
				}
		
				// Replace all tiles with the empty tile
				j <<= 1;
				for (i = 0; i < j; ++i)
				{
					updateList[10 - i] = 0x00;
					updateList[21 - i] = 0x00;
				}
				j >>= 1;
				// Update the block count
				minStackableX += j;
				blockCount -= j;
			}*/
			
			// Check if gameover: no blocks landed correctly
			if (blockCount == 0)
			{
				break;
			}
			
			// Fix the block in its current position
			// by converting it into a background tile	
			/*for (i = 0; i < 11; ++i)
			{
				i16 = NTADR_A(blockTileX << 1, blockTileY << 1);
				updateList[i] = MSB(i16);
				updateList[++i] = LSB(i16);
			}*/
			//i16 = NTADR_A(blockTileX << 1, (blockTileY - 1) << 1);
			i16 = NTADR_A(blockTileX << 1, (blockTileY - 1) << 1);
			updateList[0] = MSB(i16) | NT_UPD_HORZ;
			updateList[1] = LSB(i16);
			i16 += 32;
			updateList[11] = MSB(i16) | NT_UPD_HORZ;
			updateList[12] = LSB(i16);
			
			// Check if game has been won
			++stackHeight;
			if (stackHeight >= WIN_STACK_HEIGHT)
			{
				break;
			}
			
			// Prepare new blocks
			// Compute the new positions of next blocks
			// TODO: Randomize starting pos
			blockX = CENTER_X << FP_BITS;
			blockTileX = CENTER_X >> TILE_SIZE_BIT;
			blockTileY -= 1;
			
			// Increase block speed
			blockSpeed += INCREMENT_SPEED;
		}
	}
	
	// Fade out
	pal_fade_to(0);
}
