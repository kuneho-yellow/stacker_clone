/******************************************************************************
*  @file       	gamePhase.h
*  @brief      	Game phase handler
*  @author     	Lori
*  @created 	November 26, 2017
*  @modified   	November 27, 2017
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
static unsigned char updateList[UPDATE_LIST_SIZE];

// Constants

// Game visuals
#define INIT_BLOCK_SIZE 4			// Initial size of a block group
#define BLOCK_SIDE 16				// The dimension of one side a single block
#define SCREEN_WIDTH 256			// Should be the NES resolution
#define BASE_Y 208					// The y value at the base
#define CENTER_X (128 - BLOCK_SIDE)	// The x value of the center
#define SCREEN_MIN BLOCK_SIDE		// The effective left screen edge
#define SCREEN_MAX (SCREEN_WIDTH - BLOCK_SIDE)	// The effective right screen edge
#define TILE_EMPTY 0x00				// A tile in the chr set that is visually empty
									//  used for making blocks disappear

// Used in position computation
// Game uses 12:4 fixed point calculations
#define FP_BITS	4
#define TILE_SIZE_BIT 4
#define TILE_PLUS_FP_BITS (TILE_SIZE_BIT + FP_BITS)

// Variables
static unsigned int blockPosX;		// Block x-position
static unsigned char blockCoordX;	// Block coordinates
static unsigned char blockCoordY;
static unsigned char blockSpeed; 	// Current movmement speed
static unsigned char blockSize;		// No. of single-blocks in the block group
static unsigned char blockWidth; 	// The width of the current block group
static unsigned char stackHeight;	// Current stack height
static unsigned char isMoveRight;	// Flags rightward or leftward block movement
static unsigned char minStackCoordX;// The x coordinate value of the leftmost 
									//  area where you can place a block

void gamePhase(void)
{	
	// Set a random seed
	set_rand(frameCounter);

	// Clear sprites
	oam_clear();

	// Load the nametable
	vram_adr(NAMETABLE_A);
	vram_unrle(game_nam);
	
	// Load the palettes
	pal_bg(palette);
	pal_spr(palette);
	
	// Turn on the screen with normal brightness
	pal_bright(4);
	ppu_on_all();
	
	// Initialize the game variables
	gameResult = 0;
	blockSpeed = INIT_SPEED;
	blockSize = INIT_BLOCK_SIZE;
	blockWidth = BLOCK_SIDE * blockSize;
	blockPosX = CENTER_X << FP_BITS;
	blockCoordX = CENTER_X >> TILE_SIZE_BIT;
	blockCoordY = BASE_Y >> TILE_SIZE_BIT;
	stackHeight = 0;
	isMoveRight = 1;
	minStackCoordX = 0;

	// Set up update list
	memcpy(updateList, updateListData, sizeof(updateListData));
	set_vram_update(updateList);
	
	// Play the game bgm
	music_play(MUSIC_GAME);
	
	while (1)
	{
		// Display the moving blocks
		for (i = 0; i < blockSize; ++i)
		{
			oam_meta_spr((blockCoordX + i) << TILE_SIZE_BIT,
				blockCoordY << TILE_SIZE_BIT,
				i << 4,
				block_metasprite);
		}
		
		// Wait for the frame to finish
		ppu_wait_frame();
		++frameCounter;
		
		// Animate the BG via CHR bank switching
		bank_bg((frameCounter >> 4)&1);
		
		// Update block positions
		if (isMoveRight)
		{
			blockPosX += blockSpeed;
		}
		else
		{
			blockPosX -= blockSpeed;
		}

		// Remove insignificant bits from the x-position value
		//  to get the x-coordinate value
		blockCoordX = blockPosX >> TILE_PLUS_FP_BITS;
		// Check the discarded bits if rounding up should be done
		if ((((blockPosX & 0x00f0) >> FP_BITS)) >= 8)
		{	
			blockCoordX += 1;
		}
		
		// Check if block group has hit the game area edges
		if ((blockPosX >> FP_BITS) <= SCREEN_MIN ||
			(blockPosX >> FP_BITS) >= (SCREEN_MAX - blockWidth))
		{
			// Reverse the movement direction when hitting the screen edge
			isMoveRight ^= 1;
		}
		
		// Check for any player input
		if (pad_trigger(0))
		{
			// Initialize minStackCoordX for the very first block
			if (stackHeight < 1)
			{
				minStackCoordX = blockCoordX;
			}
			
			// Check for floating blocks:
			// happens when the leftmost block is not directly
			// on top of the leftmost previously placed block
			if (blockCoordX != minStackCoordX)
			{
				// Clear the OAM to get rid of unneeded sprites
				oam_clear();
				
				// j will store the number of floating blocks
				j = (blockCoordX < minStackCoordX) ?
					(minStackCoordX - blockCoordX): // Extra blocks to the left
					(blockCoordX - minStackCoordX); // Extra blocks to the right
				if (j > blockSize)
				{
					j = blockSize;
				}
				
				// If player is not about to lose,
				// floating blocks are converted to empty tiles
				if (blockSize != j)
				{
					// Make sure the number of visible block tiles in the updateList
					// matches the resulting number of remaining blocks
					for (i = 0; i < (j << 1); ++i)
					{
						updateList[2 + (blockSize << 1) - i] = TILE_EMPTY;
						updateList[13 + (blockSize << 1) - i] = TILE_EMPTY;
					}
				}
				
				// Update the stackable area edge
				if (blockCoordX > minStackCoordX ||
					blockSize == j) // Added to show how player loses
				{
					minStackCoordX = blockCoordX;
				}
				
				// Update the block count
				blockSize -= j;
				blockWidth = BLOCK_SIDE * blockSize;
				// TODO: Remove multiplication above
			}
			
			// Fix stacked blocks in their current position
			// by converting them into background tiles	
			// Update the addresses in the updateList to correctly show this
			var16Bit = NTADR_A(minStackCoordX << 1, (blockCoordY - 1) << 1);
			updateList[0] = MSB(var16Bit) | NT_UPD_HORZ;
			updateList[1] = LSB(var16Bit);
			var16Bit += 32;
			updateList[11] = MSB(var16Bit) | NT_UPD_HORZ;
			updateList[12] = LSB(var16Bit);
			
			// Check if gameover: no part of the block group landed correctly
			if (blockSize == 0)
			{
				gameResult = 0;
				break;
			}
			
			// Check if game has been won
			++stackHeight;
			if (stackHeight >= WIN_STACK_HEIGHT)
			{
				gameResult = 1;
				break;
			}
			
			// Compute the new position of the next block
			blockPosX = CENTER_X << FP_BITS;
			blockCoordX = CENTER_X >> TILE_SIZE_BIT;
			blockCoordY -= 1;
			// Randomize the next movement direction
			isMoveRight = (rand8() < 128) ? 0 : 1;
			
			// Increase block speed
			blockSpeed += INCREMENT_SPEED;
		}
	}
	
	// Allow one last vram update to show player's last action correctly
	delay(1);
	oam_clear();
	
	// Stop updating block visuals through updateList
	set_vram_update(NULL);
}
