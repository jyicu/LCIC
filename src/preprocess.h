#pragma once
#include <stdio.h>

void split_image(int ***C, int ***ODD, int ***EVEN, int *height, int *width);
void concat_image(int ***ODD, int ***EVEN, int ***CONCAT, int *height, int *width);
void rotate_image(int ***C, int ***R, int direction, int *height, int *width);
void RGB2YUV(int ***R, int ***G, int ***B,  int ***Y, int ***U, int ***V, int *height, int* width);
void YUV2RGB(int ***Y, int ***U, int ***V,  int ***R, int ***G, int ***B, int *height, int* width);
void image_decomposition(int ***C, int ***ODD1, int ***ODD2, int ***EVEN1, int ***EVEN2, int *height, int *width);
void preprocess(char filename[], int ***Y, int ***U_ODD1, int ***U_ODD2, int ***U_EVEN1, int***U_EVEN2, int ***V_ODD1, int ***V_ODD2, int ***V_EVEN1, int ***V_EVEN2, int *height, int *width);
void postprocess(char filename[], int ***Y, int ***U_ODD1, int ***U_ODD2, int ***U_EVEN2, int ***V_ODD1, int ***V_ODD2, int ***V_EVEN2, int *height, int *width);
void check_result();