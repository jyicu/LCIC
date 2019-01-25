#pragma once
#include <stdio.h>
#include <string.h>

void bmpRead(char filename[], unsigned char ***red, unsigned char ***green, unsigned char ***blue, int *height, int *width);
void bmpWrite(char filename[], unsigned char **red, unsigned char **green, unsigned char **blue, int h, int w);
unsigned char **alloc2D(int height, int width);
void free2D(unsigned char **p);
