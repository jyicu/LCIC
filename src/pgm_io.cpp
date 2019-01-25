#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <windows.h>

#pragma warning(disable: 4996)

int readPGM(char *input_file, int *width, int *height, int *bitDepth, void **buf) {
	FILE *fp;

	if( (fp=fopen(input_file, "rb")) ) {
		char str[100];
		fscanf(fp, "%s", str);

		assert(str[0]=='P' && str[1]=='5');

		fscanf(fp, "%d", width);
		fscanf(fp, "%d", height);
		fscanf(fp, "%d\n", bitDepth);

		assert(*bitDepth==255);

		int size = (*width)*(*height);

		*buf  = (unsigned char*)malloc(sizeof(unsigned char)*size);

		fread(*buf, size, sizeof(unsigned char), fp);
		fclose(fp);

		return 1;
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
