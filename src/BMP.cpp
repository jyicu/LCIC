#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int **alloc2D(int height, int width) {

	signed int *arr = new signed int[height*width];
	signed int **pp = new signed int*[height];
	memset(arr, 0, height*width);

	for (int y = 0; y < height; y++)
		pp[y] = &(arr[y*width]);

	return pp;
}

void free2D(int **p) {
	delete(p[0]);
	delete(p);
}


void bmpRead(char filename[], int ***red, int ***green, int ***blue, int *height, int *width){
	
	FILE *f; 
	fopen_s(&f, filename, "rb");
	if (f == NULL) {
		fputs("Image file read error.\n", stderr);
		exit(1);
	}

	unsigned char* header = new unsigned char[54];

	fread(header, sizeof(unsigned char), 54, f);

	int w = *width = *(int*)&header[18];
	int h = *height = *(int*)&header[22];
	short c = *(short*)&header[28];

	if (c != 24) {
		fprintf(stderr, "BMP image depth must be 24.\n");
		exit(-1);
	}

	unsigned char *img = new unsigned char[3 * w*h];
	memset(img, 0, 3 * w*h);

	for (int i = 0; i < h; i++)
	{
		fread(img + w * i * 3, 3, w, f);

		for (int j = 0; j<(4 - (w * 3) % 4) % 4; j++) 
			fgetc(f);
	}

	*red = alloc2D(h, w);
	*green = alloc2D(h, w);
	*blue = alloc2D(h, w);

	int idx;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			
			idx = (x + (h - y - 1)*w) * 3;

			(*red)[y][x] = img[idx + 2];
			(*green)[y][x] = img[idx + 1];
			(*blue)[y][x] = img[idx + 0];
		}
	}

	delete(img);
	fclose(f);
}

void bmpRead_1c(char filename[], int ***data) {

	FILE *f;
	fopen_s(&f, filename, "rb");
	if (f == NULL) {
		fputs("Image file read error.\n", stderr);
		exit(1);
	}
	unsigned char* header = new unsigned char[54];

	fread(header, sizeof(unsigned char), 54, f);

	int w = *(int*)&header[18];
	int h = *(int*)&header[22];

	unsigned char *img = new unsigned char[3 * w * h];
	memset(img, 0, w*h);

	fread(img, 2, w+3, f);

	for (int i = 0; i < h; i++)
	{
		fread(img + w * i * 3, 3, w, f);

		for (int j = 0; j < (4 - (w * 3) % 4) % 4; j++) {
			fgetc(f);
		}
	}

	*data = alloc2D(h, w);

	int idx;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {

			idx = (x + (h - y - 1)*w);

			(*data)[y][x] = img[idx];

		}
	}

	delete(img);
	fclose(f);

}


void bmpWrite(char filename[], int **red, int **green, int **blue, int h, int w){
	FILE *f;
	int filesize = 54 + 3 * w*h;  //w is your image width, h is image height, both int

	unsigned char *img = new unsigned char[3 * w*h];
	memset(img, 0, 3 * w*h);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			img[(x + y * w) * 3 + 2] = red[y][x];
			img[(x + y * w) * 3 + 1] = green[y][x];
			img[(x + y * w) * 3 + 0] = blue[y][x];
		}
	}

	fopen_s(&f, filename, "wb");
	
	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmppad[3] = { 0,0,0 };

	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);
	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	
	for (int y = 0; y < h; y++)
	{
		fwrite(img + w * (h-y-1) * 3, 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}

	delete(img);
	fclose(f);
}
