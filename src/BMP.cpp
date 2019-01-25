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
	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmppad[3] = { 0,0,0 };

	fopen_s(&f, filename, "rb");
	fread(bmpfileheader, 1, 14, f);
	fread(bmpinfoheader, 1, 40, f);

	int w = *width = bmpinfoheader[4] + (bmpinfoheader[5] << 8) + (bmpinfoheader[6] << 16) + (bmpinfoheader[7] << 24);
	int h = *height = bmpinfoheader[8] + (bmpinfoheader[9] << 8) + (bmpinfoheader[10] << 16) + (bmpinfoheader[11] << 24);

	unsigned char *img = new unsigned char[3 * w*h];
	memset(img, 0, 3 * w*h);

	for (int i = 0; i < h; i++)
	{
		fread(img + (w*(h - i - 1) * 3), 3, w, f);
		for(int j=0; j<(4 - (w * 3) % 4) % 4; j++) fgetc(f);
	}

	*red = alloc2D(h, w);
	*green = alloc2D(h, w);
	*blue = alloc2D(h, w);

	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int x = i;
			int y = (h - 1) - j;
			(*red)[i][j] = img[(x + y * w) * 3 + 2];
			(*green)[i][j] = img[(x + y * w) * 3 + 1];
			(*blue)[i][j] = img[(x + y * w) * 3 + 0];
		}
	}

	delete(img);
	fclose(f);
}

void bmpWrite(char filename[], unsigned char **red, unsigned char **green, unsigned char **blue, int h, int w) {
	FILE *f;
	int filesize = 54 + 3 * w*h;  //w is your image width, h is image height, both int

	unsigned char *img = new unsigned char[3 * w*h];
	memset(img, 0, 3 * w*h);

	for (int i = 0; i < w; i++)	{
		for (int j = 0; j < h; j++)		{
			int x = i;
			int y = (h - 1) - j;
			img[(x + y * w) * 3 + 2] = red[i][j];
			img[(x + y * w) * 3 + 1] = green[i][j];
			img[(x + y * w) * 3 + 0] = blue[i][j];
		}
	}

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

	fopen_s(&f, filename, "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for (int i = 0; i < h; i++)
	{
		fwrite(img + (w*(h - i - 1) * 3), 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}

	delete(img);
	fclose(f);
}
