#pragma once

int readPGM(char *input_file, int *width, int *height, int *bitDepth, unsigned char ***buf);
int readPGM_16bit(char *input_file, int *width, int *height, int *bitDepth,  unsigned short int ***buf);
int writePGM(char *output_file, int width, int height, int bitDepth, void **buf);
