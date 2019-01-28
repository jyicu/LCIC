#include "BMP.h"
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <math.h>

#define CLOCKWISE         0
#define COUNTER_CLOCKWISE 1

/*
	@abstract : Decompose image into ODD1, ODD2, EVEN2
	@input
		***C    : channel of image
		*height : height of image
		*width  : width of image
	@output
		***ODD1, ODD2, EVEN2  : Decomposed ODD1, ODD2, EVEN2
*/
void image_decomposition(int ***C, int ***ODD1, int ***ODD2, int ***EVEN2, int *height, int *width)
{
	int **EVEN1;

	split_image(C, ODD1, &EVEN1, height, width);

	int **EVEN1_rotate;
	int half_height = *height / 2;
	int half_width = *width / 2;

	rotate_image(&EVEN1, &EVEN1_rotate, CLOCKWISE, &half_height, width);

	split_image(&EVEN1_rotate, ODD2, EVEN2, height, &half_width);
}

/*
	@abstract : Split image into (1.Odd line image / 2. Even line image)
	@input
		***C    : channel of image
		*height : height of image
		*width  : width of image
	@output
		***ODD  : Odd line image  (Size : height/2, width)
		***EVEN : Even line image (Size : height/2, width)
*/
void split_image(int ***C, int ***ODD, int ***EVEN, int *height, int *width)
{
	*ODD  = alloc2D(*height / 2, *width);
	*EVEN = alloc2D(*height / 2, *width);

	for (int y = 0; y < *height; y++) {
		for (int x = 0; x < *width; x++) {
			if (y % 2 == 0)
				(*EVEN)[y / 2][x] = (*C)[y][x];
			else
				(*ODD)[(y - 1) / 2][x] = (*C)[y][x];
		}
	}
}


/*
	@abstract : Concat (1.Odd line image / 2.Even line image) into CONCAT image
	@input
		***ODD  : Odd line image  (Size : height/2, width)
		***EVEN : Even line image (Size : height/2, width)
		*height : height of CONCAT image
		*width  : width of CONCAT image
	@output
		***CONCAT : CONCAT image of ODD/EVEN image
*/
void concat_image(int ***ODD, int ***EVEN, int ***CONCAT, int *height, int *width)
{
	*CONCAT = alloc2D(*height, *width);

	for (int y = 0; y < *height; y++) {
		for (int x = 0; x < *width; x++) {
			if (y % 2 == 0)
				(*CONCAT)[y][x] = (*EVEN)[y / 2][x];
			else
				(*CONCAT)[y][x] = (*ODD)[(y - 1) / 2][x];
		}
	}
}


/*
	@abstract : Rotate image 90 degrees (Clockwise / Counter clockwise)
	@input
		***C      : channel of image
		direction : direction of rotation (0 : Clockwise, 1 : Counter clockwise)
		*height   : height of channel of image
		*width    : width of channel of image
	@output
		***R : Rotated image
*/
void rotate_image(int ***C, int ***R, int direction, int *height, int *width)
{
	*R = alloc2D(*width, *height);

	if (direction == CLOCKWISE) {
		for (int y = 0; y < *height; y++) {
			for (int x = 0; x < *width; x++) {
				(*R)[x][y] = (*C)[y][x];
			}
		}
	}
	else {
		for (int y = 0; y < *height; y++) {
			for (int x = 0; x < *width; x++) {
				(*R)[x][*height - y - 1] = (*C)[y][x];
			}
		}
	}
}


/*
	@abstract : Apply RGB to YUV color transform
	@input
		***R,G,B : R,G,B channels of image	
		*height  : height of image
		*width   : width of image
	@output
		***Y,U,V : Y,U,V channels of image
*/
void RGB2YUV(int ***R, int ***G, int ***B, int ***Y, int ***U, int ***V, int *height, int* width)
{

	*Y = alloc2D(*height, *width);
	*U = alloc2D(*height, *width);
	*V = alloc2D(*height, *width);

	int r, g, b;

	for (int y = 0; y < *height; y++) {
		for (int x = 0; x < *width; x++) {

			r = (*R)[y][x];
			g = (*G)[y][x];
			b = (*B)[y][x];

			(*Y)[y][x] = int(floor((float)(r + 2 * g + b) / 4));
			(*U)[y][x] = r - g;
			(*V)[y][x] = b - g;
		}
	}
}


/*
	@abstract : Apply YUV to RGB color transform
	@input
		***Y,U,V : Y,U,V channels of image
		*height  : height of image
		*width   : width of image
	@output
		***R,G,B : R,G,B channels of image
*/
void YUV2RGB(int ***Y, int ***U, int ***V, int ***R, int ***G, int ***B, int *height, int* width)
{
	*R = alloc2D(*height, *width);
	*G = alloc2D(*height, *width);
	*B = alloc2D(*height, *width);

	int y_, u, v;

	for (int y = 0; y < *height; y++) {
		for (int x = 0; x < *width; x++) {

			y_ = (*Y)[y][x];
			u = (*U)[y][x];
			v = (*V)[y][x];

			(*G)[y][x] = y_ - int(floor((float)(u + v) / 4));
			(*R)[y][x] = u + (*G)[y][x];
			(*B)[y][x] = v + (*G)[y][x];
		}
	}
}

/*
	@abstract : Check if split/concat/rotate/RCT functions work well
*/
void check_result() {
	char infile[] = "suzy.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";
	FILE *fp;

	int **R, **G, **B;
	int height, width;

	bmpRead(infile, &R, &G, &B, &height, &width);

	assert(height % 2 == 0);
	assert(width % 2 == 0);

	int **ODD_R, **ODD_G, **ODD_B;
	int **EVEN_R, **EVEN_G, **EVEN_B;

	split_image(&R, &ODD_R, &EVEN_R, &height, &width);
	split_image(&G, &ODD_G, &EVEN_G, &height, &width);
	split_image(&B, &ODD_B, &EVEN_B, &height, &width);

	int **CONCAT_R, **CONCAT_G, **CONCAT_B;

	concat_image(&ODD_R, &EVEN_R, &CONCAT_R, &height, &width);
	concat_image(&ODD_G, &EVEN_G, &CONCAT_G, &height, &width);
	concat_image(&ODD_B, &EVEN_B, &CONCAT_B, &height, &width);


	char out_odd[] = "odd.bmp";
	char out_even[] = "even.bmp";
	char sum[] = "Concat.bmp";

	bmpWrite(out_odd, ODD_R, ODD_G, ODD_B, height / 2, width);
	bmpWrite(out_even, EVEN_R, EVEN_G, EVEN_B, height / 2, width);
	bmpWrite(sum, CONCAT_R, CONCAT_G, CONCAT_B, height, width);

	int **ROTC_R, **ROTCC_R;
	int **ROTC_G, **ROTCC_G;
	int **ROTC_B, **ROTCC_B;

	rotate_image(&R, &ROTC_R, 0, &height, &width);
	rotate_image(&G, &ROTC_G, 0, &height, &width);
	rotate_image(&B, &ROTC_B, 0, &height, &width);

	rotate_image(&R, &ROTCC_R, 1, &height, &width);
	rotate_image(&G, &ROTCC_G, 1, &height, &width);
	rotate_image(&B, &ROTCC_B, 1, &height, &width);

	bmpWrite("Rot_clock.bmp", ROTC_R, ROTC_G, ROTC_B, width, height);
	bmpWrite("Rot_counterclock.bmp", ROTCC_R, ROTCC_G, ROTCC_B, width, height);

	int **Y, **U, **V;
	int **R_n, **G_n, **B_n;

	RGB2YUV(&R, &G, &B, &Y, &U, &V, &height, &width);
	YUV2RGB(&Y, &U, &V, &R_n, &G_n, &B_n, &height, &width);

	bmpWrite("Color.bmp", R_n, G_n, B_n, height, width);
}