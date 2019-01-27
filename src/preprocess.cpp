#include "BMP.h"
#include <stdio.h>
#include <assert.h>

#define CLOCKWISE         0
#define COUNTER_CLOCKWISE 1

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
void split_image(unsigned char ***C, unsigned char ***ODD, unsigned char ***EVEN, int *height, int *width)
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
void concat_image(unsigned char ***ODD, unsigned char ***EVEN, unsigned char ***CONCAT, int *height, int *width)
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
void rotate_image(unsigned char ***C, unsigned char ***R, int direction, int *height, int *width)
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
	@abstract : Check if split/concat/rotate functions work well
*/
void check_result() {
	char infile[] = "suzy.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";
	FILE *fp;

	unsigned char **R;
	unsigned char **G;
	unsigned char **B;
	int height, width;

	bmpRead(infile, &R, &G, &B, &height, &width);

	assert(height % 2 == 0);
	assert(width % 2 == 0);

	unsigned char **ODD_R, **ODD_G, **ODD_B;
	unsigned char **EVEN_R, **EVEN_G, **EVEN_B;

	split_image(&R, &ODD_R, &EVEN_R, &height, &width);
	split_image(&G, &ODD_G, &EVEN_G, &height, &width);
	split_image(&B, &ODD_B, &EVEN_B, &height, &width);

	unsigned char **CONCAT_R;
	unsigned char **CONCAT_G;
	unsigned char **CONCAT_B;

	concat_image(&ODD_R, &EVEN_R, &CONCAT_R, &height, &width);
	concat_image(&ODD_G, &EVEN_G, &CONCAT_G, &height, &width);
	concat_image(&ODD_B, &EVEN_B, &CONCAT_B, &height, &width);


	char out_odd[] = "odd.bmp";
	char out_even[] = "even.bmp";
	char sum[] = "Concat.bmp";

	bmpWrite(out_odd, ODD_R, ODD_G, ODD_B, height / 2, width);
	bmpWrite(out_even, EVEN_R, EVEN_G, EVEN_B, height / 2, width);
	bmpWrite(sum, CONCAT_R, CONCAT_G, CONCAT_B, height, width);

	unsigned char **ROTC_R, **ROTCC_R;
	unsigned char **ROTC_G, **ROTCC_G;
	unsigned char **ROTC_B, **ROTCC_B;

	rotate_image(&R, &ROTC_R, 0, &height, &width);
	rotate_image(&G, &ROTC_G, 0, &height, &width);
	rotate_image(&B, &ROTC_B, 0, &height, &width);

	rotate_image(&R, &ROTCC_R, 1, &height, &width);
	rotate_image(&G, &ROTCC_G, 1, &height, &width);
	rotate_image(&B, &ROTCC_B, 1, &height, &width);

	bmpWrite("Rot_clock.bmp", ROTC_R, ROTC_G, ROTC_B, width, height);
	bmpWrite("Rot_counterclock.bmp", ROTCC_R, ROTCC_G, ROTCC_B, width, height);
}