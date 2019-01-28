#pragma once
#include <stdio.h>
#include <string.h>

void bmpRead(char filename[], int ***red, int ***green, int ***blue, int *height, int *width);
void bmpWrite(char filename[], int **red, int **green, int **blue, int h, int w);
int **alloc2D(int height, int width);
void free2D(int **p);
