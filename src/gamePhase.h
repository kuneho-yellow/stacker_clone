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
#define BLOCK_WIDTH_BIT	4		// Used for converting from blocks to bytes (bytes = blocks << BLOCK_WIDTH_BIT)
#define BASE_BLOCK_WIDTH 14		// Width of playfield base in blocks
// VRAM update list content indices
#define UPPER_START 3			// Upper row
#define LOWER_START 14			// Lower row

// Variables
static unsigned int blockX;			// Current block x-position
static unsigned char blockTileX;	// Block coordinates
static unsigned char blockTileY;
static unsigned char blockSpeed; 	// Current movmement speed
static unsigned char blockCount;	// Current block count
static unsigned char direction;		// Current movement direction
static unsigned char stackHeight;	// Current stack height
static unsigned char isMoving;
static unsigned char prevStartX;
static unsigned char prevBlockCount;
static unsigned char overlapStartX;
static unsigned char overlapBlockCount;

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
	direction = 1;
	stackHeight = 0;
	isMoving = 1;
	prevStartX = 0;		// Left edge of playfield
	prevBlockCount = BASE_BLOCK_WIDTH;
	
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
				
		// Check if block group has hit the edges
		if ((blockX >> FP_BITS) <= SCREEN_MIN ||
			(blockX >> FP_BITS) >= (SCREEN_MAX - (blockCount << BLOCK_WIDTH_BIT)))
		{
			// Reverse direction when hitting the screen edge
			direction ^= 1;
		}
		
		// Check for any player input
		if (pad_trigger(0))
		{
			// Get the start tile and amount of overlap
			overlapStartX = MAX(blockTileX, prevStartX);
			overlapBlockCount = MIN(prevStartX + prevBlockCount - 1, blockTileX + blockCount - 1) - MAX(blockTileX, prevStartX) + 1;
		
			// Check if gameover: no blocks landed correctly
			if (overlapBlockCount == 0 || overlapBlockCount > BASE_BLOCK_WIDTH)
			{
				break;
			}
			
			// Update variables for previous block
			prevStartX = overlapStartX;
			prevBlockCount = overlapBlockCount;
			
			// Fix stacked blocks in their current position
			// by converting them into background tiles
			// Set start addresses
			i16 = NTADR_A(overlapStartX << 1, (blockTileY - 1) << 1);
			updateList[0] = MSB(i16) | NT_UPD_HORZ;
			updateList[1] = LSB(i16);
			i16 += 32;
			updateList[11] = MSB(i16) | NT_UPD_HORZ;
			updateList[12] = LSB(i16);
			// Keep first overlapBlockCount*2 items as is, and set the rest to 0x00
			for (i = overlapBlockCount<<1; i < INIT_BLOCK_COUNT<<1; ++i)
			{
				updateList[UPPER_START + i] = 0x00;
				updateList[LOWER_START + i] = 0x00;
			}

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
			blockCount = overlapBlockCount;			
			oam_clear();

			// Increase block speed
			blockSpeed += INCREMENT_SPEED;
		}
	}
	
	// Fade out
	pal_fade_to(0);
}
