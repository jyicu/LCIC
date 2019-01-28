#pragma once
#include <stdio.h>

void image_decomposition(int ***C, int ***ODD1, int ***ODD2, int ***EVEN2, int *height, int *width);
void split_image(int ***C, int ***ODD, int ***EVEN, int *height, int *width);
void concat_image(int ***ODD, int ***EVEN, int ***CONCAT, int *height, int *width);
void rotate_image(int ***C, int ***R, int direction, int *height, int *width);
void RGB2YUV(int ***R, int ***G, int ***B,  int ***Y, int ***U, int ***V, int *height, int* width);
void YUV2RGB(int ***Y, int ***U, int ***V,  int ***R, int ***G, int ***B, int *height, int* width);
void check_result();