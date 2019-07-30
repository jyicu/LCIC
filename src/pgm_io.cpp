#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <windows.h>
#include "BMP.h"

#pragma warning(disable: 4996)

unsigned char **alloc2D_uc(int height, int width) {
	unsigned char *arr = new unsigned char[height*width];
	unsigned char **pp = new unsigned char*[height];
	memset(arr, 0, height*width);

	for (int y = 0; y < height; y++)
		pp[y] = &(arr[y*width]);

	return pp;
}

unsigned short int **alloc2D_usi(int height, int width) {
	unsigned short int *arr = new unsigned short int[height*width];
	unsigned short int **pp = new unsigned short int*[height];
	memset(arr, 0, height*width);

	for (int y = 0; y < height; y++)
		pp[y] = &(arr[y*width]);

	return pp;
}

int readPGM(char *input_file, int *width, int *height, int *bitDepth, unsigned char ***buf) {
	FILE *fp;

	if ((fp = fopen(input_file, "rb"))) {
		char str[100];
		fscanf(fp, "%s", str);

		assert(str[0] == 'P' && str[1] == '5');

		fscanf(fp, "%d", width);
		fscanf(fp, "%d", height);
		fscanf(fp, "%d\n", bitDepth);

		assert(*bitDepth == 255);

		int size = (*width)*(*height);

		*buf = alloc2D_uc(*height, *width);

		fread(&(*buf[0][0]), size, sizeof(unsigned char), fp);
		fclose(fp);

		return 1;
	}
	else {
		printf("readPGM: cannot open %s\n", input_file);
	}

	return -1;
}

int readPGM_16bit(char *input_file, int *width, int *height, int *bitDepth, unsigned short int ***buf) {
	FILE *fp;

	if ((fp = fopen(input_file, "rb"))) {
		char str[100];
		fscanf(fp, "%s", str);

		assert(str[0] == 'P' && str[1] == '5');

		fscanf(fp, "%d", width);
		fscanf(fp, "%d", height);
		fscanf(fp, "%d\n", bitDepth);

		assert(*bitDepth == 65535);

		int size = (*width)*(*height);

		*buf = alloc2D_usi(*height, *width);

		fread(&(*buf[0][0]), size*2, sizeof(unsigned char), fp);
		fclose(fp);

		return 1;
	}
	else {
		printf("readPGM: cannot open %s\n", input_file);
	}

	return -1;
}

int writePGM(char *output_file, int width, int height, int bitDepth, void **buf) {
	FILE *fp;

	if( (fp=fopen(output_file, "wb")) ) {
		fprintf(fp, "P5\n");
		fprintf(fp, "%d\n", width);
		fprintf(fp, "%d\n", height);
		fprintf(fp, "%d\n", bitDepth);

		assert(bitDepth==255);

		int size = width*height;
		fwrite(*buf, size, sizeof(unsigned char), fp);
		fclose(fp);

		return 1;
	}

	return -1;
}
