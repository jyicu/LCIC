#include "BMP.h"
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <math.h>

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
void split_image(int ***C, int ***ODD, int ***EVEN, int *height, int *width)
{
	*ODD = alloc2D(*height / 2, *width);
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

	if (direction == COUNTER_CLOCKWISE) {
		for (int y = 0; y < *height; y++) {
			for (int x = 0; x < *width; x++) {
				(*R)[*width - x - 1][y] = (*C)[y][x];
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

			(*Y)[y][x] = (r + 2 * g + b) >> 2;
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
@abstract : Decompose image into ODD1, ODD2, EVEN2
@input
***C    : channel of image
*height : height of image
*width  : width of image
@output
***ODD1, ODD2, EVEN1, EVEN2  : Decomposed ODD1, ODD2, EVEN1, EVEN2
*/
void image_decomposition(int ***C, int ***ODD1, int ***ODD2, int ***EVEN1, int ***EVEN2, int *height, int *width)
{
	split_image(C, ODD1, EVEN1, height, width);

	int **EVEN1_rotate;
	int half_height = *height / 2;

	rotate_image(EVEN1, &EVEN1_rotate, CLOCKWISE, &half_height, width);

	split_image(&EVEN1_rotate, ODD2, EVEN2, width, &half_height);

	free(EVEN1_rotate);
}


/*
@abstract : Decompose RGB image into YUV image (ODD/EVEN line images)
@input
filename : image filename
@output
***Y                : Decomposed Y (h, w)
***U_ODD1, V_ODD1   : Decomposed U,V Odd1 (h/2, w)
***U_ODD2, V_ODD2   : Decomposed U,V Odd2 (w/2, h/2)
***U_EVEN1, V_EVEN1 : Decomposed U,V Even2 (h/2, w)
***U_EVEN2, V_EVEN2 : Decomposed U,V Even2 (w/2, h/2)
*/
void preprocess(char filename[], int ***Y, int ***U_ODD1, int ***U_ODD2, int ***U_EVEN1, int***U_EVEN2, int ***V_ODD1, int ***V_ODD2, int ***V_EVEN1, int ***V_EVEN2, int *height, int *width)
{
	// Read in image file to RGB channels
	int **R, **G, **B;

	bmpRead(filename, &R, &G, &B, height, width);

	assert(*height % 2 == 0);
	assert(*width % 2 == 0);

	// Color transform RGB into YUV image
	int **U, **V;

	RGB2YUV(&R, &G, &B, Y, &U, &V, height, width);

	// Decompose U,V channels into odd1, odd2, even2 images
	image_decomposition(&U, U_ODD1, U_ODD2, U_EVEN1, U_EVEN2, height, width);
	image_decomposition(&V, V_ODD1, V_ODD2, V_EVEN1, V_EVEN2, height, width);

	// free memory
	free(R);
	free(G);
	free(B);
	free(U);
	free(V);
}

/*
@abstract : Compose complete RGB Image(h,w) from decomposed YUVs
@input
filename : output image filename
***Y                : Decomposed Y (h, w)
***U_ODD1, V_ODD1   : Decomposed U,V Odd1 (h/2, w)
***U_ODD2, V_ODD2   : Decomposed U,V Odd2 (w/2, h/2)
***U_EVEN2, V_EVEN2 : Decomposed U,V Even2 (w/2, h/2)
*height             : height of final image
*width              : width of final image
@output
None
*/
void postprocess(char filename[], int ***Y, int ***U_ODD1, int ***U_ODD2, int ***U_EVEN2, int ***V_ODD1, int ***V_ODD2, int ***V_EVEN2, int *height, int *width)
{
	int **U_EVEN1_R, **V_EVEN1_R;
	int half_height = *height / 2;

	concat_image(U_ODD2, U_EVEN2, &U_EVEN1_R, width, &half_height);
	concat_image(V_ODD2, V_EVEN2, &V_EVEN1_R, width, &half_height);

	int **U_EVEN1, **V_EVEN1;

	rotate_image(&U_EVEN1_R, &U_EVEN1, COUNTER_CLOCKWISE, width, &half_height);
	rotate_image(&V_EVEN1_R, &V_EVEN1, COUNTER_CLOCKWISE, width, &half_height);

	int **U, **V;

	concat_image(U_ODD1, &U_EVEN1, &U, height, width);
	concat_image(V_ODD1, &V_EVEN1, &V, height, width);

	int **R, **G, **B;

	YUV2RGB(Y, &U, &V, &R, &G, &B, height, width);

	bmpWrite(filename, R, G, B, *height, *width);

	free(U_EVEN1_R);
	free(V_EVEN1_R);
	free(U_EVEN1);
	free(V_EVEN1);
	free(U);
	free(V);
	free(R);
	free(G);
	free(B);
}


/*
@abstract : Check if split/concat/rotate/RCT functions work well
*/
void check_result() {
	char infile[] = "lena.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";

	int **R, **G, **B;
	int height, width;

	bmpRead(infile, &R, &G, &B, &height, &width);

	assert(height % 2 == 0);
	assert(width % 2 == 0);


	// Split Image Check
	int **ODD_R, **ODD_G, **ODD_B;
	int **EVEN_R, **EVEN_G, **EVEN_B;

	split_image(&R, &ODD_R, &EVEN_R, &height, &width);
	split_image(&G, &ODD_G, &EVEN_G, &height, &width);
	split_image(&B, &ODD_B, &EVEN_B, &height, &width);

	char out_odd[] = "result/Odd.bmp";
	char out_even[] = "result/Even.bmp";

	bmpWrite(out_odd, ODD_R, ODD_G, ODD_B, height / 2, width);
	bmpWrite(out_even, EVEN_R, EVEN_G, EVEN_B, height / 2, width);

	// Concat Image Check
	int **CONCAT_R, **CONCAT_G, **CONCAT_B;

	concat_image(&ODD_R, &EVEN_R, &CONCAT_R, &height, &width);
	concat_image(&ODD_G, &EVEN_G, &CONCAT_G, &height, &width);
	concat_image(&ODD_B, &EVEN_B, &CONCAT_B, &height, &width);

	char concat[] = "result/Concat.bmp";

	bmpWrite(concat, CONCAT_R, CONCAT_G, CONCAT_B, height, width);

	// Rotation Image Check
	int **ROTC_R, **ROTCC_R;
	int **ROTC_G, **ROTCC_G;
	int **ROTC_B, **ROTCC_B;

	rotate_image(&R, &ROTC_R, CLOCKWISE, &height, &width);
	rotate_image(&G, &ROTC_G, CLOCKWISE, &height, &width);
	rotate_image(&B, &ROTC_B, CLOCKWISE, &height, &width);

	rotate_image(&R, &ROTCC_R, COUNTER_CLOCKWISE, &height, &width);
	rotate_image(&G, &ROTCC_G, COUNTER_CLOCKWISE, &height, &width);
	rotate_image(&B, &ROTCC_B, COUNTER_CLOCKWISE, &height, &width);

	char rotation_clock[] = "result/Rot_clock.bmp";
	char rotation_cc[] = "result/Rot_counterclock.bmp";

	bmpWrite(rotation_clock, ROTC_R, ROTC_G, ROTC_B, width, height);
	bmpWrite(rotation_cc, ROTCC_R, ROTCC_G, ROTCC_B, width, height);

	// Color Transform Image Check
	int **Y, **U, **V;
	int **R_n, **G_n, **B_n;

	RGB2YUV(&R, &G, &B, &Y, &U, &V, &height, &width);
	YUV2RGB(&Y, &U, &V, &R_n, &G_n, &B_n, &height, &width);

	char color_file[] = "result/Color.bmp";

	bmpWrite(color_file, R_n, G_n, B_n, height, width);

	// Image Decomposition Check
	int **ODD1_R, **ODD2_R, **EVEN1_R, **EVEN2_R;
	int **ODD1_G, **ODD2_G, **EVEN1_G, **EVEN2_G;
	int **ODD1_B, **ODD2_B, **EVEN1_B, **EVEN2_B;

	image_decomposition(&R, &ODD1_R, &ODD2_R, &EVEN1_R, &EVEN2_R, &height, &width);
	image_decomposition(&G, &ODD1_G, &ODD2_G, &EVEN1_G, &EVEN2_G, &height, &width);
	image_decomposition(&B, &ODD1_B, &ODD2_B, &EVEN1_B, &EVEN2_B, &height, &width);

	char odd1_file[] = "result/Odd1.bmp";
	char odd2_file[] = "result/Odd2.bmp";
	char even1_file[] = "result/Even1.bmp";
	char even2_file[] = "result/Even2.bmp";

	bmpWrite(odd1_file, ODD1_R, ODD1_G, ODD1_B, height / 2, width);
	bmpWrite(odd2_file, ODD2_R, ODD2_G, ODD2_B, width / 2, height / 2);
	bmpWrite(even1_file, EVEN1_R, EVEN1_G, EVEN1_B, height / 2, width);
	bmpWrite(even2_file, EVEN2_R, EVEN2_G, EVEN2_B, width / 2, height / 2);

	// Preprocess Check
	int **Y_, **U_ODD1_, **U_ODD2_, **U_EVEN1_, **U_EVEN2_, **V_ODD1_, **V_ODD2_, **V_EVEN1_, **V_EVEN2_;

	char preprocess_file[] = "result/Preprocess.bmp";

	preprocess(infile, &Y_, &U_ODD1_, &U_ODD2_, &U_EVEN1_, &U_EVEN2_, &V_ODD1_, &V_ODD2_, &V_EVEN1_, &V_EVEN2_, &height, &width);
	postprocess(preprocess_file, &Y_, &U_ODD1_, &U_ODD2_, &U_EVEN2_, &V_ODD1_, &V_ODD2_, &V_EVEN2_, &height, &width);

	// Free
	free(R);
	free(G);
	free(B);
	free(ODD_R);
	free(ODD_G);
	free(ODD_B);
	free(EVEN_R);
	free(EVEN_G);
	free(EVEN_B);
	free(CONCAT_R);
	free(CONCAT_G);
	free(CONCAT_B);
	free(ROTC_R);
	free(ROTC_G);
	free(ROTC_B);
	free(ROTCC_R);
	free(ROTCC_G);
	free(ROTCC_B);
	free(Y);
	free(U);
	free(V);
	free(R_n);
	free(G_n);
	free(B_n);
	free(ODD1_R);
	free(ODD1_G);
	free(ODD1_B);
	free(ODD2_R);
	free(ODD2_G);
	free(ODD2_B);
	free(EVEN1_R);
	free(EVEN1_G);
	free(EVEN1_B);
	free(EVEN2_R);
	free(EVEN2_G);
	free(EVEN2_B);
	free(Y_);
	free(U_ODD1_);
	free(U_ODD2_);
	free(U_EVEN1_);
	free(U_EVEN2_);
	free(V_ODD1_);
	free(V_ODD2_);
	free(V_EVEN1_);
	free(V_EVEN2_);
}