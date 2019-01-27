#pragma once
#include <stdio.h>

void split_image(unsigned char ***C, unsigned char ***ODD, unsigned char ***EVEN, int *height, int *width);
void concat_image(unsigned char ***ODD, unsigned char ***EVEN, unsigned char ***CONCAT, int *height, int *width);
void rotate_image(unsigned char ***C, unsigned char ***R, int direction, int *height, int *width);
void check_result();