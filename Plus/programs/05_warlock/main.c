#include "nanoprintf.h"
#include "memory_mapped_registers.h"
#include <stdint.h>
#include "yrv/ee_printf.h"
#include "cordic/cordic.h"

#define LED_0 0xe
#define LED_1 0xd
#define LED_2 0xb
#define LED_3 0x7

#define HEX_H 0b11001000
#define HEX_E 0b10110000
#define HEX_L 0b11110001
#define HEX_O 0b10000001

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0

// Compile nanoprintf in this translation unit.
#define NANOPRINTF_IMPLEMENTATION 1


// D E F I N E S /////////////////////////////////////////////////////////////

// #define DEBUG 1

#define OVERBOARD          48 // the absolute closest a player can get to a wall

#define INTERSECTION_FOUND 1


#define _GBORDER 1
#define _GFILLINTERIOR 0

// constants used to represent angles

#define ANGLE_0     0
#define ANGLE_1     5
#define ANGLE_2     10
#define ANGLE_4     20
#define ANGLE_5     25
#define ANGLE_6     30
#define ANGLE_15    80
#define ANGLE_30    160
#define ANGLE_45    240
#define ANGLE_60    320
#define ANGLE_90    480
#define ANGLE_135   720
#define ANGLE_180   960
#define ANGLE_225   1200
#define ANGLE_270   1440
#define ANGLE_315   1680
#define ANGLE_360   1920

#define WORLD_ROWS    16        // number of rows in the game world
#define WORLD_COLUMNS 16        // number of columns in the game world
#define CELL_X_SIZE   64        // size of a cell in the gamw world
#define CELL_Y_SIZE   64

// size of overall game world

#define WORLD_X_SIZE  (WORLD_COLUMNS * CELL_X_SIZE)
#define WORLD_Y_SIZE  (WORLD_ROWS    * CELL_Y_SIZE)

// G L O B A L S /////////////////////////////////////////////////////////////

//unsigned int * clock = (unsigned int *)0x0000046C; // pointer to internal
// 18.2 clicks/sec


// world map of nxn cells, each cell is 64x64 pixels

char world[WORLD_ROWS][WORLD_COLUMNS] = { 
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1},
	{1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,1,1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1},
	{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,1,1,1,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,1,1,1,1,1,1,1,1,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};       
// pointer to matrix of cells that make up world

float tan_table[1921] = {1.0f};
float inv_tan_table[1921]= {1.0f};

float y_step[1921]= {1.0f};
float x_step[1921]= {1.0f}; 

float cos_table[1921]= {1.0f}; 

float inv_cos_table[1921]= {1.0f}; 
float inv_sin_table[1921]= {1.0f};
float cos_local[361]= {1.0f};
float sin_local[361]= {1.0f};


/*
0x00010000 port0 = {8'bxxxxxxxx, RDP, RA, RB, RC, RD, RE, RF, RG}
0x00010002 port1 = {4'bxxxx, C46, C45, C43, C42, 4'bxxxx, AN4, AN3, AN2, AN1}
0x00010004 port2 = L[16:1]
0x00010006 port3 = {CLR_EI, 1'bx, INIT, ECALL, NMI, LINT, INT, EXCEPT, L[24:17]}
0x00010008 port4 = DIP[16:1]
0x0001000a port5 = {C9, C8, C6, S5, S4, S3, S2, S1, DIP[24:17]}
0x0001000c port6 = {DIV_RATE, S_RESET, 3'bxxx}
0x0001000e port7 = {4'bxxxx, EMPTY, DONE, FULL, OVR, SER_DATA}
*/

uint32_t clock;
void cordic(int theta, int *s, int *c, int n);
void cordic_grad(int grad, int *sin, int *cos);
float fabs(float f);
void sleep();

void long_sleep();

void very_long_sleep();

void clean();

char getchar() {
    return (char) port4;
}


void H();

void E();

void L();

void O();

void HELO(int state);

int next(int prev, int step);

void itoa(int, char *);

void beep();

void __attribute__((optimize("O0"))) say_serial_a();

void __attribute__((optimize("O0"))) say_lpt_a();

void plot(int x, int y, char color);
void line_slow(int x1, int y1, int x2, int y2, char color);
float sin(int grad);
short time();
void wfi();
void cls();
void rainbow();


int color = 0;
int global_x = 0, global_y=0;
int text_x, text_y;
int isMap = 0;

//Screen dimension constants
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

void _setcolor(int new_color);
void _moveto(int x1, int y1);
int _setpixel(int x, int y);
int _lineto(int x2, int y2);
int _linetoh(int x2, int y2);
int _rectangle(int order, int x1, int y1, int x2, int y2);

int clofk = 0;
char *VGA = (char *)0xA0000000L;


/////////////////////////////////////////////////////////////////////////////

void sline(long x1, long y1, long x2, long y2, int color)
{

	// used a a diagnostic function to draw a scaled line

	x1 = x1 / 4;
	y1 = 256 - (y1 / 4);

	x2 = x2 / 4;
	y2 = 256 - (y2 / 4);

	_setcolor(color);
	_moveto((int)x1, (int)y1);
	_lineto((int)x2, (int)y2);

} 

/////////////////////////////////////////////////////////////////////////////

void splot(long x, long y, int color)
{
	// used as a diagnostic function to draw a scaled point

	x = x / 4;
	y = 256 - (y / 4);

	_setcolor(color);

	_setpixel((int)x, (int)y);
	_setpixel((int)x + 1, (int)y);
	_setpixel((int)x, (int)y + 1);
	_setpixel((int)x + 1, (int)y + 1);

} // end splot


void Build_Tables(void)
{

	int ang;
	float rad_angle;



	// create tables, sit back for a sec!

	for (ang = ANGLE_0; ang <= ANGLE_360; ang++)
	{

		rad_angle = (float)(3.272e-4f) + (float)ang * 2.0f * 3.141592654f / ANGLE_360;

		tan_table[ang] = (float)tanc_rad(rad_angle);
		inv_tan_table[ang] = (float)1.0f / tan_table[ang];


		// tangent has the incorrect signs in all quadrants except 1, so
		// manually fix the signs of each quadrant since the tangent is
		// equivalent to the slope of a line and if the tangent is wrong
		// then the ray that is case will be wrong

		if (ang >= ANGLE_0 && ang < ANGLE_180)
		{
			y_step[ang] = fabs((tan_table[ang] * CELL_Y_SIZE));
		}
		else
			y_step[ang] = -fabs(tan_table[ang] * (float)CELL_Y_SIZE);

		if (ang >= ANGLE_90 && ang < ANGLE_270)
		{
			x_step[ang] = -fabs(inv_tan_table[ang] * (float)CELL_X_SIZE);
		}
		else
		{
			x_step[ang] = fabs(inv_tan_table[ang] * (float)CELL_X_SIZE);
		}

		// create the sin and cosine tables to copute distances

		inv_cos_table[ang] = 1.0f / cosc_rad(rad_angle);

		inv_sin_table[ang] = 1.0f / sinc_rad(rad_angle);

	} // end for ang

// create view filter table.  There is a cosine wave modulated on top of
// the view distance as a side effect of casting from a fixed point.
// to cancell this effect out, we multiple by the inverse of the cosine
// and the result is the proper scale.  Without this we would see a
// fishbowl effect, which might be desired in some cases?

	for (ang = -ANGLE_30; ang <= ANGLE_30; ang++)
	{

		rad_angle = (float)(3.272e-4f) + (float)ang * 2.0f * 3.141592654f / ANGLE_360;

		cos_table[ang + ANGLE_30] = 1.0f / cosc_rad(rad_angle);

	} // end for

} // end Build_Tables


void Draw_2D_Map(void)
{
	// draw 2-D map of world

	int row, column, block, t, done = 0;

	for (row = 0; row < WORLD_ROWS; row++)
	{
		for (column = 0; column < WORLD_COLUMNS; column++)
		{

			block = world[row][column];

			// test if there is a solid block there

			if (block == 0)
			{

				_setcolor(253);
				_rectangle(_GBORDER, column * CELL_X_SIZE / 4, row * CELL_Y_SIZE / 4,
					column * CELL_X_SIZE / 4 + CELL_X_SIZE / 4 - 1, row * CELL_Y_SIZE / 4 + CELL_Y_SIZE / 4 - 1);

			}
			else
			{

				_setcolor(152);
				_rectangle(_GFILLINTERIOR, column * CELL_X_SIZE / 4, row * CELL_Y_SIZE / 4,
					column * CELL_X_SIZE / 4 + CELL_X_SIZE / 4 - 1, row * CELL_Y_SIZE / 4 + CELL_Y_SIZE / 4 - 1);

			}

		} // end for column

	} // end for row

}

void Ray_Caster(long x, long y, long view_angle)
{
	// This function casts out 320 rays from the viewer and builds up the video
	// display based on the intersections with the walls. The 320 rays are
	// cast in such a way that they all fit into a 60 degree field of view
	// a ray is cast and then the distance to the first horizontal and vertical
	// edge that has a cell in it is recorded.  The intersection that has the
	// closer distance to the user is the one that is used to draw the bitmap.
	// the distance is used to compute the height of the "sliver" of texture
	// or line that will be drawn on the screen

	// note: this function uses floating point (slow), no optimizations (slower)
	// and finally it makes calls to Microsofts Graphics libraries (slowest!)
	// however, writing it in this manner makes it many orders of magnitude
	// easier to understand.

	int rcolor = 55;

	long xray = 0,        // tracks the progress of a ray looking for Y interesctions
		yray = 0,        // tracks the progress of a ray looking for X interesctions
		next_y_cell,   // used to figure out the quadrant of the ray
		next_x_cell,
		cell_x,        // the current cell that the ray is in
		cell_y,
		x_bound,       // the next vertical and horizontal intersection point
		y_bound,
		xb_save,       // storage to record intersections cell boundaries
		yb_save,
		x_delta,       // the amount needed to move to get to the next cell
		y_delta,       // position
		ray,           // the current ray being cast 0-320
		casting = 2,     // tracks the progress of the X and Y component of the ray
		x_hit_type,    // records the block that was intersected, used to figure
		y_hit_type,    // out which texture to use

		top,           // used to compute the top and bottom of the sliver that
		bottom;        // is drawn symetrically around the bisecting plane of the
	// screens vertical extents


	float xi,           // used to track the x and y intersections
		yi,
		xi_save,      // used to save exact x and y intersection points
		yi_save,
		dist_x,       // the distance of the x and y ray intersections from
		dist_y,       // the viewpoint
		scale;        // the final scale to draw the "sliver" in

	// S E C T I O N  1 /////////////////////////////////////////////////////////v

	// initialization

	// compute starting angle from player.  Field of view is 60 degrees, so
	// subtract half of that current view angle

	if ((view_angle -= ANGLE_30) < 0)
	{
		// wrap angle around
		view_angle = ANGLE_360 + view_angle;
	} // end if

	rcolor = 14;

	// loop through all 320 rays

	// section 2

	
	

	for (ray = 0; ray < SCREEN_WIDTH; ray++)
	{
		
		// S E C T I O N  2 /////////////////////////////////////////////////////////

			// compute first x intersection

			// need to know which half plane we are casting from relative to Y axis
		
		if (view_angle >= ANGLE_0 && view_angle < ANGLE_180)
		{
			
			// compute first horizontal line that could be intersected with ray
			// note: it will be above player

			y_bound = CELL_Y_SIZE + CELL_Y_SIZE * (y / CELL_Y_SIZE);

			// compute delta to get to next horizontal line

			y_delta = CELL_Y_SIZE;

			// based on first possible horizontal intersection line, compute X
			// intercept, so that casting can begin

			//xi = inv_tan_table[view_angle] * (y_bound - y) + x;
			

			//printf("%d %g %g %g \n", view_angle, rad, tanc_rad(rad), tan_table[view_angle]);
			//printf("%d %g %g %g \n\n", view_angle, rad, 1.0f / tanc_rad(rad), inv_tan_table[view_angle]);

			//xi = inv_tan_table[view_angle] * (y_bound - y) + x;
			xi = inv_tan_table[view_angle] * (y_bound - y) + x;


			// set cell delta

			next_y_cell = 0;

		} // end if upper half plane
		else
		{

			// compute first horizontal line that could be intersected with ray
			// note: it will be below player

			y_bound = CELL_Y_SIZE * (y / CELL_Y_SIZE);

			// compute delta to get to next horizontal line

			y_delta = -CELL_Y_SIZE;

			// based on first possible horizontal intersection line, compute X
			// intercept, so that casting can begin
			

			xi = inv_tan_table[view_angle] * (y_bound - y) + x;

			// set cell delta

			next_y_cell = -1;

		} // end else lower half plane


 // S E C T I O N  3 /////////////////////////////////////////////////////////

	 // compute first y intersection

	 // need to know which half plane we are casting from relative to X axis

		if (view_angle < ANGLE_90 || view_angle >= ANGLE_270)
		{

			// compute first vertical line that could be intersected with ray
			// note: it will be to the right of player

			x_bound = CELL_X_SIZE + CELL_X_SIZE * (x / CELL_X_SIZE);

			// compute delta to get to next vertical line

			x_delta = CELL_X_SIZE;

			// based on first possible vertical intersection line, compute Y
			// intercept, so that casting can begin

			yi = tan_table[view_angle]* (x_bound - x) + y;
			//printf("%d, ", x / CELL_X_SIZE);
			// set cell delta

			next_x_cell = 0;

		} // end if right half plane
		else
		{

			// compute first vertical line that could be intersected with ray
			// note: it will be to the left of player

			x_bound = CELL_X_SIZE * (x / CELL_X_SIZE);

			// compute delta to get to next vertical line

			x_delta = -CELL_X_SIZE;

		//	// based on first possible vertical intersection line, compute Y
		//	// intercept, so that casting can begin

			yi = tan_table[view_angle] * (x_bound - x) + y;

		//	// set cell delta

			next_x_cell = -1;

		} // end else right half plane

 // begin cast

		casting = 2;                // two rays to cast simultaneously
		xray = yray = 0;                // reset intersection flags


		// S E C T I O N  4 /////////////////////////////////////////////////////////

		while (casting)
		{

			// continue casting each ray in parallel

			if (xray != INTERSECTION_FOUND)
			{

				// test for asymtotic ray

				// if (view_angle==ANGLE_90 || view_angle==ANGLE_270)

				if (fabs(y_step[view_angle]) == 0)
				{
					xray = INTERSECTION_FOUND;
					casting--;
					dist_x = 1e+8f;

				} // end if asymtotic ray

			 // compute current map position to inspect

				cell_x = ((x_bound + next_x_cell) / CELL_X_SIZE);
				cell_y = (long)(yi / CELL_Y_SIZE);

				// test if there is a block where the current x ray is intersecting

				//if ((x_hit_type = world[(WORLD_ROWS - 1) - cell_y][cell_x]) != 0)
				if (cell_x < 0) cell_x = 0;
				if (cell_x > WORLD_COLUMNS) cell_x = WORLD_COLUMNS - 1; 
				if (cell_y < 0) cell_y = 0; 
				if (cell_y > WORLD_ROWS) cell_y = WORLD_ROWS - 1;
				if ((x_hit_type = world[(WORLD_ROWS - 1) - cell_y][cell_x]) != 0)
				{
					// compute distance

					dist_x = (yi - y) * inv_sin_table[view_angle];
					yi_save = yi;
					xb_save = x_bound;

					// terminate X casting

					xray = INTERSECTION_FOUND;
					casting--;

				} // end if a hit
				else
				{
					// compute next Y intercept

					yi += y_step[view_angle];

				} // end else

			} // end if x ray has intersected

// S E C T I O N  5 /////////////////////////////////////////////////////////

			if (yray != INTERSECTION_FOUND)
			{

				// test for asymtotic ray

				// if (view_angle==ANGLE_0 || view_angle==ANGLE_180)

				if (fabs(x_step[view_angle]) == 0)
				{
					yray = INTERSECTION_FOUND;
					casting--;
					dist_y = 1e+8f;

				} // end if asymtotic ray

			 // compute current map position to inspect

				cell_x = (long)(xi / CELL_X_SIZE);
				cell_y = ((y_bound + next_y_cell) / CELL_Y_SIZE);
								if (cell_x < 0) cell_x = 0;
				if (cell_x > WORLD_COLUMNS) cell_x = WORLD_COLUMNS - 1; 
				if (cell_y < 0) cell_y = 0; 
				if (cell_y > WORLD_ROWS) cell_y = WORLD_ROWS - 1;
				// test if there is a block where the current y ray is intersecting

				if ((y_hit_type = world[(WORLD_ROWS - 1) - cell_y][cell_x]) != 0)
				{
					// compute distance

					dist_y = (xi - x) * inv_cos_table[view_angle];
					xi_save = xi;
					yb_save = y_bound;

					// terminate Y casting

					yray = INTERSECTION_FOUND;
					casting--;

				} // end if a hit
				else
				{
					// compute next X intercept

					xi += x_step[view_angle];

				} // end else

			} // end if y ray has intersected

		 // move to next possible intersection points


			x_bound += x_delta;
			y_bound += y_delta;


			// _settextposition(38,40);
			// printf("x_bound = %ld, y_bound = %ld    ",x_bound,y_bound);

		} // end while not done


// S E C T I O N  6 /////////////////////////////////////////////////////////

	// at this point, we know that the ray has succesfully hit both a
	// vertical wall and a horizontal wall, so we need to see which one
	// was closer and then render it

	// note: latter we will replace the crude monochrome line with a sliver
	// of texture, but this is good enough for now

		if (dist_x < dist_y)
		{

			// there was a vertical wall closer than the horizontal

			// compute actual scale and multiply by view filter so that spherical
			// distortion is cancelled

			scale = cos_table[ray] * 15000.0f / (1e-10f + dist_x);

			// compute top and bottom and do a very crude clip

			if ((top = 100 - scale / 2) < 1)
				top = 1;

			if ((bottom = top + scale) > 199)
				bottom = 199;

			// draw wall sliver and place some dividers up

			if (((long)yi_save) % CELL_Y_SIZE <= 1)
				_setcolor(146);
			else
				_setcolor(137);

			
			if(isMap == 1) {
				// sline(x, y, (long)xb_save, (long)yi_save, rcolor);
			}
			else {
				_moveto((int)(SCREEN_WIDTH - ray - 1), (int)top);
				_linetoh((int)(SCREEN_WIDTH - ray - 1), (int)bottom);
			}
		}
		else // must of hit a horizontal wall first
		{


			// compute actual scale and multiply by view filter so that spherical
			// distortion is cancelled

			scale = cos_table[ray] * 15000.0f / (1e-10f + dist_y);

			// compute top and bottom and do a very crude clip

			if ((top = 100 - scale / 2) < 1)
				top = 1;

			if ((bottom = top + scale) > 199)
				bottom = 199;

			// draw wall sliver and place some dividers up

			if (((long)xi_save) % CELL_X_SIZE <= 1)
				_setcolor(147);
			else
				_setcolor(138);
			if (isMap == 1) {
				// sline(x, y, (long)xi_save, (long)yb_save, rcolor);
			}
			else {
				_moveto((int)(SCREEN_WIDTH - ray - 1), (int)top);
				_linetoh((int)(SCREEN_WIDTH - ray - 1), (int)bottom);
			}

		} // end else

 // S E C T I O N  7 /////////////////////////////////////////////////////////

	 // cast next ray

		if (++view_angle >= ANGLE_360)
		{
			// reset angle back to zero

			view_angle = 0;

		} // end if

	} // end for ray

} // end Ray_Caster

void main()
{
    char key = 0;
	int count=0;
    int quit = 0;
     int row, column, block, t, done = 0;
 
     long x, y, view_angle, x_cell, y_cell, x_sub_cell, y_sub_cell;
 
     float dx, dy;
 
    clean();
    very_long_sleep();
    Build_Tables();
    cls();

    // for (int t = 0; t <= 360; t = t + 20)
    // {
    //     char buf[32] = {0};

    //     npf_snprintf(buf, sizeof(buf), " %d  %0.6f", t, inv_cos_table[t]);
    //     ee_printf("%s\n", buf);
    //     very_long_sleep();
    // }
  


    // for (int x = 0; x < 320; x++)
    // {
    //     int grad = 720 * x / 320;
    //     int y = (60.0f * sin(grad));
    //     plot(x, 120 - y, 200);
    //     very_long_sleep();
    // }


    cls();
    view_angle = ANGLE_60;
    
    x = 8 * 64 + 25;
    y = 3 * 64 + 25;

    // render initial view
    // if (isMap == 0) {
    //     _setcolor(119);
    //     _rectangle(_GFILLINTERIOR, 0, 100, 320, 200);
    // }
    
    //Draw_2D_Map();
	
	_setcolor(115);
	_rectangle(_GBORDER, 0, 0, 319, 199);
	if (isMap == 0) {
		_setcolor(118);
		_rectangle(_GFILLINTERIOR, 0, 100, 319, 199);
	}
	Ray_Caster(x, y, view_angle);

	while (1) {
        count++;


		if (count % 10 == 0)
		{

			key = getchar();
			dx = dy = 0;
			switch (key)
			{
			case 'A':
				dx = dy = 0;
				if ((view_angle += ANGLE_6) >= ANGLE_360)
					view_angle = ANGLE_0;
				break;
			case 'D':
			dx = dy = 0;
				if ((view_angle -= ANGLE_6) < ANGLE_0)
					view_angle = ANGLE_360;
				break;
			case 'W':
				dx = dy = 0;
				dx = cosc_rad(6.28f * view_angle / ANGLE_360) * 10;
			    dy = sinc_rad(6.28f * view_angle / ANGLE_360) * 10;

				break;
			case 'S':
				dx = dy = 0;
				dx = -cosc_rad(6.28f * view_angle / ANGLE_360) * 10;
				dy = -sinc_rad(6.28f * view_angle / ANGLE_360) * 10;
				break;
			case 0x20:
				break;
			case 'M':
				dx = dy = 0;
				_setcolor(0);
			    _rectangle(_GFILLINTERIOR, 0, 0, 320, 240);
				if (isMap==0)
				{
					isMap = 1;
					Draw_2D_Map();

				}
				else
				{
					isMap = 0;
					_setcolor(118);
			        _rectangle(_GFILLINTERIOR, 0, 100, 320, 200);
				}
				break;
			}
			if (key != 0)
			{
				// move player
				if(isMap ==0) {
				_setcolor(0);
				_rectangle(_GFILLINTERIOR, 0, 0, 320, 200);
				_setcolor(118);
				_rectangle(_GFILLINTERIOR, 0, 100, 320, 200);
				}
				x += dx;
				y += dy;

				// test if user has bumped into a wall i.e. test if there
				// is a cell within the direction of motion, if so back up !

				// compute cell position

				x_cell = x / CELL_X_SIZE;
				y_cell = y / CELL_Y_SIZE;

				// compute position relative to cell

				x_sub_cell = x % CELL_X_SIZE;
				y_sub_cell = y % CELL_Y_SIZE;

				// resolve motion into it's x and y components

				if (dx > 0)
				{
					// moving right

					if ((world[(WORLD_ROWS - 1) - y_cell][x_cell + 1] != 0) &&
						(x_sub_cell > (CELL_X_SIZE - OVERBOARD)))
					{
						// back player up amount he steped over the line

						x -= (x_sub_cell - (CELL_X_SIZE - OVERBOARD));

					} // end if need to back up
				}
				else
				{
					// moving left

					if ((world[(WORLD_ROWS - 1) - y_cell][x_cell - 1] != 0) &&
						(x_sub_cell < (OVERBOARD)))
					{
						// back player up amount he steped over the line

						x += (OVERBOARD - x_sub_cell);

					} // end if need to back up

				} // end else

				if (dy > 0)
				{
					// moving up

					if ((world[(WORLD_ROWS - 1) - (y_cell + 1)][x_cell] != 0) &&
						(y_sub_cell > (CELL_Y_SIZE - OVERBOARD)))
					{
						// back player up amount he steped over the line

						y -= (y_sub_cell - (CELL_Y_SIZE - OVERBOARD));

					} // end if need to back up
				}
				else
				{
					// moving down

					if ((world[(WORLD_ROWS - 1) - (y_cell - 1)][x_cell] != 0) &&
						(y_sub_cell < (OVERBOARD)))
					{
						// back player up amount he steped over the line

						y += (OVERBOARD - y_sub_cell);

					} // end if need to back up

				} // end else

				// render the view

				Ray_Caster(x, y, view_angle);
			}
		}
	}

	wfi();
}

void wfi() {
    int state = 0;
    int step = 3;
    while (1)
    {
        if (state == step)
        {
            beep();
            // say_lpt_a();
        }
        HELO(state);
        state = next(state, step);
    }
}

void rainbow()
{
    for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            plot(x, y, y);
        }
    }
}

int abs(int i)
{
    return i < 0 ? -i : i;
}

int sgn(int x)
{
    if (x == 0)
        return 0;
    else if (x > 0)
        return 1;
    else
        return -1;
}
/*
void plot(int x, int y, char color) {
    VGA[(y<<8) + (y<<6) + x] = color;
}
*/

void line_slow(int x1, int y1, int x2, int y2, char color)
{
    int dx, dy, sdx, sdy, px, py, dxabs, dyabs, i;
    int slope;

    dx = x2 - x1; /* the horizontal distance of the line */
    dy = y2 - y1; /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        slope = (int)dy / (int)dx;
        for (i = 0; i != dx; i += sdx)
        {
            px = i + x1;
            py = slope * i + y1;
            plot(px, py, color);
        }
    }
    else /* the line is more vertical than horizontal */
    {
        slope = (int)dx / (int)dy;
        for (i = 0; i != dy; i += sdy)
        {
            px = slope * i + x1;
            py = i + y1;
            plot(px, py, color);
        }
    }
}

void sleep()
{
    for (int i = 0; i < 100; i++)
    {
        clock++;
    }
}

void long_sleep()
{
    for (int i = 0; i < 2000; i++)
    {
        clock++;
    }
}

void very_long_sleep()
{
    for (int i = 0; i < 60000; i++)
    {
        asm("nop");
    }
}

void clean()
{
    port0 = 0xff;
    port1 = 0xf;
    port3 = 0x01;
}

void H()
{
    port1 = LED_3;
    port0 = HEX_H;
    sleep();
    clean();
}

void E()
{
    port1 = LED_2;
    port0 = HEX_E;
    sleep();
    clean();
}

void L()
{
    port1 = LED_1;
    port0 = HEX_L;
    sleep();
    clean();
}

void O()
{
    port1 = LED_0;
    port0 = HEX_O;
    sleep();
    clean();
}

void HELO(int state)
{
    if (state > 50000)
        H();
    if (state > 40000)
        E();
    if (state > 30000)
        L();
    if (state > 20000)
        O();
}

int next(int prev, int step)
{
    if (prev > 60000)
        prev = 0;
    return prev + step;
}

void beep()
{
    for (short i = 0; i < 100; i++)
    {
        port2 = 0xff00;
        long_sleep();
        port2 = 0x0000;
        long_sleep();
    }
}

short time()
{
    return port5;
}

int _lineto(int x2, int y2) {
    // TODO Draw line
    line_slow(global_x, global_y, x2,y2,color);
    _moveto(x2, y2);
    return 0;
 }
 
 int _linetoh(int x2, int y2) {
    // TODO Draw line
	for(int y_t=global_y;y_t<=y2; y_t++) {
		plot(x2, y_t, color);
	}
    // line_slow(global_x, global_y, x2,y2,color);
    _moveto(x2, y2);
    return 0;
 }
 
 
 
 void _setcolor(int new_color) {
     color = new_color;
 
 }
 
 int _setpixel(int x, int y) {
     plot(x,y,100);
     return 0;
 }
 
 void _moveto(int x1, int y1) {
     global_x = x1;
     global_y = y1;
 }
 
 int _rectangle(int border, int x1, int y1, int x2, int y2) {
 
     if (border) {
        line_slow(x1,y1, x2, y1,color);
        line_slow(x2,y1, x2, y2,color);
        line_slow(x2,y2, x1, y2,color);
        line_slow(x1,y2, x1, y1,color);
        return 0;
     }
     else
     {
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y <y2; y++) {
                plot(x,y, color);
            }
        }
         return 0;
     }
 }
 


