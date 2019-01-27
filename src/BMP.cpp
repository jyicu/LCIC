#include <stdio.h>
#include <string.h>


unsigned char **alloc2D(int height, int width) {
	unsigned char *arr = new unsigned char[height*width];
	unsigned char **pp = new unsigned char*[height];
	memset(arr, 0, height*width);

	for (int y = 0; y < height; y++)
		pp[y] = &(arr[y*width]);

	return pp;
}

void free2D(unsigned char **p) {
	delete(p[0]);
	delete(p);
}


void bmpRead(char filename[], unsigned char ***red, unsigned char ***green, unsigned char ***blue, int *height, int *width) {
	
	FILE *f; 
	fopen_s(&f, filename, "rb");

	unsigned char* header = new unsigned char[54];

	fread(header, sizeof(unsigned char), 54, f);

	int w = *width = *(int*)&header[18];
	int h = *height = *(int*)&header[22];

	unsigned char *img = new unsigned char[3 * w*h];
	memset(img, 0, 3 * w*h);

	for (int i = 0; i < h; i++)
	{
		fread(img + w*i*3, 3, w, f);

		for (int j = 0; j<(4 - (w * 3) % 4) % 4; j++) 
			fgetc(f);
	}

	*red = alloc2D(h, w);
	*green = alloc2D(h, w);
	*blue = alloc2D(h, w);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			
			(*red)[y][x] = img[(x + y * w) * 3 + 2];
			(*green)[y][x] = img[(x + y * w) * 3 + 1];
			(*blue)[y][x] = img[(x + y * w) * 3 + 0];
		}
	}

	delete(img);
	fclose(f);
}


void bmpWrite(char filename[], unsigned char **red, unsigned char **green, unsigned char **blue, int h, int w){
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
		fwrite(img + w * y * 3, 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}

	delete(img);
	fclose(f);
}
