#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <windows.h>

#include "acfile\arithmetic_codec.h"
#include "BMP.h"
#include "pgm_io.h"

#pragma warning(disable: 4996)

typedef unsigned char UINT8;

#define NUM_CTX			24
#define SYMBOL_MAX		16
#define ALPHABET_MAX	(SYMBOL_MAX+4)

#define MAX(x,y)	((x)>(y) ? (x) : (y))
#define MIN(x,y)	((x)<(y) ? (x) : (y))
#define MAX4(X)		MAX(MAX((X)[0],(X)[1]),MAX((X)[2],(X)[3]))
#define MIN4(X)		MIN(MIN((X)[0],(X)[1]),MIN((X)[2],(X)[3]))
#define ABS(x)		((x)>0 ? (x) : -(x))
#define CLIP(x,y,z)	MAX((y), MIN((x), (z)))
#define CLIP255(x)	MAX(0, MIN((x), 255))
#define ROUND(x)	((int)((x)+0.5))
#define CEIL(x)		((int)((x)))
#define UINT8(x)	CLIP255(ROUND(x))
#define SQUARE(x)	((x)*(x))


void encodeMag(int mag, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {
	assert(SYMBOL_MAX == 16);

	if (mag < SYMBOL_MAX) {
		pCoder->encode(mag, *pDm);
	}
	else if (mag < 32) {
		pCoder->encode(SYMBOL_MAX, *pDm);
		pCoder->put_bits((mag - 16), 4);
	}
	else if (mag < 64) {
		pCoder->encode(SYMBOL_MAX + 1, *pDm);
		pCoder->put_bits((mag - 32), 5);
	}
	else if (mag < 128) {
		pCoder->encode(SYMBOL_MAX + 2, *pDm);
		pCoder->put_bits((mag - 64), 6);
	}
	else{
		pCoder->encode(SYMBOL_MAX + 3, *pDm);
		pCoder->put_bits((mag - 128), 7);
	}
	/*if (mag >= SYMBOL_MAX) {
		pCoder->encode(SYMBOL_MAX, *pDm);
		mag -= SYMBOL_MAX;
		pCoder->put_bit(mag & 1);
		encodeMag(mag >> 1, pCoder, pDm);
	}
	else {
		pCoder->encode(mag, *pDm);
	}*/
}

int decodeMag(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {
	int mag = pCoder->decode(*pDm);

	switch (mag) {
	case (SYMBOL_MAX + 0): mag = pCoder->get_bits(4) + 16; break;
	case (SYMBOL_MAX + 1): mag = pCoder->get_bits(5) + 32; break;
	case (SYMBOL_MAX + 2): mag = pCoder->get_bits(6) + 64; break;
	case (SYMBOL_MAX + 3): mag = pCoder->get_bits(7) + 128; break;
	}

	return mag;
	/*int mag = pCoder->decode(*pDm);

	if (mag == SYMBOL_MAX) {
		return SYMBOL_MAX + pCoder->get_bit() + (decodeMag(pCoder, pDm) << 1);
	}
	else {
		return mag;
	}*/
}

int makeCurPixel(int sym, int left, float pred) {
	int inverse = 0;
	float Pd = pred + left;
	int P = UINT8(Pd);
	int X;

	if (P >= 128) {
		Pd = 255 - Pd;
		P = 255 - P;
		inverse = 1;
	}

	if (Pd > P) {
		if (sym > 2 * P)
			X = sym;
		else if (sym % 2 == 0)
			X = P - sym / 2;
		else
			X = P + (sym + 1) / 2;
	}
	else {
		if (sym > 2 * P)
			X = sym;
		else if (sym % 2 == 0)
			X = P + sym / 2;
		else
			X = P - (sym + 1) / 2;
	}

	return inverse ? 255 - X : X;
}

int makeSymbol(int X, int left, float pred) {
	//int inverse = 0;
	int sym;
	float Pd = pred + left;
	int P = UINT8(Pd);

	if (P >= 128) {
		Pd = 255 - Pd;
		P = 255 - P;
		X = 255 - X;
		//inverse = 1;
	}

	if (Pd > P) {
		if (X <= P)
			sym = 2 * (P - X);
		else if (X > 2 * P)
			sym = X;
		else
			sym = 2 * (X - P) - 1;
	}
	else {
		if (X < P)
			sym = 2 * (P - X) - 1;
		else if (X > 2 * P)
			sym = X;
		else
			sym = 2 * (X - P);
	}
	/*
	int rec = makeCurPixel(sym, left, pred);
	if (X != rec) {
		makeCurPixel(sym, left, pred);
	}
	assert((inverse ? 255 - X : X) == rec);*/
	return sym;
}

void initModel(Adaptive_Data_Model dm[]) {
	for (int i = 0; i < NUM_CTX; i++) {
		dm[i].set_alphabet(ALPHABET_MAX);
		
		if(i==0)
			dm[i].set_distribution(0.7f);
		else
			dm[i].set_distribution(0.8f);
	}
}

int calcContext(float ctx) {
	float TH[] = {.25, .5, .75, 1, 1.25, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5, 13, 13.5, 14};
	int i;

	for (i = 0; i < NUM_CTX; i++) {
		if(ctx < TH[i]) break;
	}

	return MIN(i, NUM_CTX - 1);
}

// median edge dectector from LOCO-I
int MED(int a, int b, int c) {
	int P;
	
	if (c >= MAX(a, b))			P = MIN(a, b);
	else if (c <= MIN(a, b))	P = MAX(a, b);
	else						P = a + b - c;
	
	return P;
}

int encode(FILE *fp, UINT8 **R, int height, int width) {
	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];
	int ctxCnt[NUM_CTX] = {0};

	int y, x, i;
	int numPix = 0;
	
	initModel(dm);

	int size = width * height;
	coder.set_buffer(size*10);
	coder.start_encoder();

	assert((unsigned short)width == width);
	assert((unsigned short)height == height);

	coder.put_bits((unsigned short)width, 16);
	coder.put_bits((unsigned short)height, 16);

	y = 0; {	// (0,x): the first and the second rows
		x = 0; {	// (0,0)
			coder.put_bits(R[0][0], 8);
			numPix++;
		}

		x = 1; {	// (0,1)
			int X = R[y][x];
			int L = R[y][x - 1];	// left
			int sym = makeSymbol(X, L, 0);
			encodeMag(sym, &coder, &dm[(int)(NUM_CTX/3)]);
			numPix++;
		}

		for (x = 2; x < width; x++) {
			int X = R[y][x];
			int L = R[y][x-1];	// left
			int LL = R[y][x-2];	// left left
			int iCtx = calcContext(ABS(L-LL));
			int sym = makeSymbol(X, L, 0);
			encodeMag(sym, &coder, &dm[iCtx]);
			numPix++;
		}
	}

	y = 1; {	// (1,x): the second rows
		x = 0; {	// (1,0)
			int X = R[y][x];
			int U = R[y][x - 1];	// upper
			int sym = makeSymbol(X, U, 0);
			encodeMag(sym, &coder, &dm[(int)(NUM_CTX / 3)]);
			numPix++;
		}

		int lastErrAbs = MIN(ABS((int)R[1][0] - R[0][0]), ABS((int)R[0][1] - R[0][0]));
		for (x = 1; x < width; x++) {
			// MED: median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int X = R[y][x];
			int P = MED(a, b, c);

			int iCtx = calcContext(lastErrAbs);
			ctxCnt[iCtx]++;

			int sym = makeSymbol(X, a, P - a);
			encodeMag(sym, &coder, &dm[iCtx]);
			lastErrAbs = ABS(X - P);
			numPix++;
		}
	}

	for (y = 2; y < height; y++) {
		x = 0; {	// (y,0): the first column
			int X = R[y][x];
			int U = R[y - 1][x];	// upper
			int UU = R[y - 2][x];	// upper upper
			int iCtx = calcContext(ABS(U - UU));
			int sym = makeSymbol(X, U, 0);
			encodeMag(sym, &coder, &dm[iCtx]);
			numPix++;
		}

		x = 1; {	// (y,1): the second column	-> median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int X = R[y][x];
			int P = MED(a, b, c);

			int lastErrAbs = ABS(b - MED(R[y - 1][x - 1], R[y - 2][x], R[y - 2][x - 1]));	// prediction error of upper pixel
			int iCtx = calcContext(lastErrAbs);
			ctxCnt[iCtx]++;

			int sym = makeSymbol(X, a, P - a);
			encodeMag(sym, &coder, &dm[iCtx]);
			numPix++;
		}

		for (x = 2; x < width - 2; x++) {	// most pixels (not boundary)
			int X = R[y][x];
			int left = R[y][x - 1];

			nbr[0] = R[y - 2][x - 2];
			nbr[1] = R[y - 2][x - 1];
			nbr[2] = R[y - 2][x];
			nbr[3] = R[y - 2][x + 1];
			nbr[4] = R[y - 2][x + 2];
			nbr[5] = R[y - 1][x - 2];
			nbr[6] = R[y - 1][x - 1];
			nbr[7] = R[y - 1][x];
			nbr[8] = R[y - 1][x + 1];
			nbr[9] = R[y - 1][x + 2];
			nbr[10] = R[y][x - 2];

			float pred, ctx;
			pred=...
			ctx=...

			int iCtx = calcContext(ctx);
			ctxCnt[iCtx]++;

			int sym = makeSymbol(X, left, pred);
//			encodeMag(sym, &coder, &dm[iCtx]);
			
			if (iCtx > 10) {
				coder.put_bit(sym & 1);
				encodeMag(sym>>1, &coder, &dm[iCtx]);
			}
			else {
				encodeMag(sym, &coder, &dm[iCtx]);
			}
			
			numPix++;

			//printf("%d %d\n", iCtx, sym);
		}

		for(x = width-2; x<=width-1; x++) {	// the last 2 columns	-> median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int X = R[y][x];
			int P = MED(a, b, c);

			int lastErrAbs = ABS(b - MED(R[y - 1][x - 1], R[y - 2][x], R[y - 2][x - 1]));	// prediction error of upper pixel
			//int lastErrAbs = ABS(a - MED(R[y][x - 2], R[y - 1][x-1], R[y - 1][x - 2]));	// prediction error of left pixel
			int iCtx = calcContext(lastErrAbs);
			ctxCnt[iCtx]++;

			int sym = makeSymbol(X, a, P - a);
			encodeMag(sym, &coder, &dm[iCtx]);
			numPix++;
		}
	}

	/*for (int i = 0; i < NUM_CTX; i++) {
		printf("Context %02d: %10d\n", i, ctxCnt[i]);
		//for(int j=0; j<ALPHABET_MAX; j++)
			//printf("Symbol %02d: %10d\n", j, dm[i].symbol_count[j]);
	}*/

	int bytes = coder.write_to_file(fp);
	printf("%7.4f bpp (%d bytes)\n", 8.0*bytes / numPix, bytes);

	return bytes;
}

void decode(FILE *fp, UINT8 ***pR, int *h, int *w) {
	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];
	int y, x;

	initModel(dm);

	int size = 2000 * 2000;	// width * height;
	coder.set_buffer(size);
	coder.read_from_file(fp);

	int width = *w = coder.get_bits(16);
	int height = *h = coder.get_bits(16);
	UINT8 **R = *pR = alloc2D(height, width);

	y = 0; {	// (0,x): the first and the second rows
		x = 0; {	// (0,0)
			R[0][0] = coder.get_bits(8);
		}

		x = 1; {	// (0,1)
			int L = R[y][x - 1];	// left
			int sym = decodeMag(&coder, &dm[(int)(NUM_CTX / 3)]); 
			R[y][x] = makeCurPixel(sym, L, 0);
		}

		for (x = 2; x < width; x++) {
			int L = R[y][x - 1];	// left
			int LL = R[y][x - 2];	// left left
			int iCtx = calcContext(ABS(L - LL));
			int sym = decodeMag(&coder, &dm[iCtx]);
			R[y][x] = makeCurPixel(sym, L, 0);
		}
	}

	y = 1; {	// (1,x): the second rows
		x = 0; {	// (1,0)
			int U = R[y][x - 1];	// upper
			int sym = decodeMag(&coder, &dm[(int)(NUM_CTX / 3)]);
			R[y][x] = makeCurPixel(sym, U, 0);
		}

		int lastErrAbs = MIN(ABS((int)R[1][0] - R[0][0]), ABS((int)R[0][1] - R[0][0]));
		for (x = 1; x < width; x++) {
			// MED: median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int P = MED(a, b, c);

			int iCtx = calcContext(lastErrAbs);

			int sym = decodeMag(&coder, &dm[iCtx]);
			int X = R[y][x] = makeCurPixel(sym, a, P-a);
			lastErrAbs = ABS(X - P);
		}
	}

	for (y = 2; y < height; y++) {
		x = 0; {	// (y,0): the first column
			int U = R[y - 1][x];	// upper
			int UU = R[y - 2][x];	// upper upper
			int iCtx = calcContext(ABS(U - UU));
			int sym = decodeMag(&coder, &dm[iCtx]);
			R[y][x] = makeCurPixel(sym, 0, U);
		}

		x = 1; {	// (y,1): the second column	-> median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int X = R[y][x];
			int P = MED(a, b, c);

			int lastErrAbs = ABS(b - MED(R[y - 1][x - 1], R[y - 2][x], R[y - 2][x - 1]));	// prediction error of upper pixel
			int iCtx = calcContext(lastErrAbs);

			int sym = decodeMag(&coder, &dm[iCtx]);
			R[y][x] = makeCurPixel(sym, a, P - a);
		}

		for (x = 2; x < width - 2; x++) {
			int X = R[y][x];
			int left = R[y][x - 1];

			nbr[0] = R[y - 2][x - 2];
			nbr[1] = R[y - 2][x - 1];
			nbr[2] = R[y - 2][x];
			nbr[3] = R[y - 2][x + 1];
			nbr[4] = R[y - 2][x + 2];
			nbr[5] = R[y - 1][x - 2];
			nbr[6] = R[y - 1][x - 1];
			nbr[7] = R[y - 1][x];
			nbr[8] = R[y - 1][x + 1];
			nbr[9] = R[y - 1][x + 2];
			nbr[10] = R[y][x - 2];

			float pred, ctx;
			pred = ...
			ctx = ...

			int iCtx = calcContext(ctx);

			int sym = decodeMag(&coder, &dm[iCtx]);
			R[y][x] = makeCurPixel(sym, left, pred);
		}

		for (x = width - 2; x <= width - 1; x++) {	// the last 2 columns	-> median edge detector
			int a = R[y][x - 1];		// left;
			int b = R[y - 1][x];		// upper
			int c = R[y - 1][x - 1];	// left upper
			int P = MED(a, b, c);

			int lastErrAbs = ABS(b - MED(R[y - 1][x - 1], R[y - 2][x], R[y - 2][x - 1]));	// prediction error of upper pixel
			int iCtx = calcContext(lastErrAbs);

			int sym = decodeMag(&coder, &dm[iCtx]);
			R[y][x] = makeCurPixel(sym, a, P-a);
		}
	}

	coder.stop_decoder();
}

void rawRead(char filename[], unsigned char ***R, unsigned char ***G, unsigned char ***B, int h, int w) {
	FILE *f;

	fopen_s(&f, filename, "rb");
	unsigned char *buf = new unsigned char[3 * w*h];
	fread(buf, 1, 3 * h*w, f);
	fclose(f);

	*R = alloc2D(h, w);
	*G = alloc2D(h, w);
	*B = alloc2D(h, w);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			(*R)[y][x] = buf[3 * (w*y + x)];
			(*G)[y][x] = buf[3 * (w*y + x) + 1];
			(*B)[y][x] = buf[3 * (w*y + x) + 2];
		}
	}

	free(buf);
}

void rawWrite(char filename[], unsigned char **R, unsigned char **G, unsigned char **B, int h, int w) {
	FILE *f;

	unsigned char *buf = new unsigned char[3 * w*h];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			buf[3 * (w*y + x)] = R[y][x];
			buf[3 * (w*y + x) + 1] = G[y][x];
			buf[3 * (w*y + x) + 2] = B[y][x];
		}
	}

	fopen_s(&f, filename, "wb");
	fwrite(buf, 1, 3 * h*w, f);
	fclose(f);

	free(buf);
}

void runEncoder(char *infile, char *codefile) {
	FILE *fp;

	unsigned char **R, **G, **B;
	int height, width, bitdepth;

	//bmpRead(infile, &R, &G, &B, &height, &width);
	//rawRead(infile, &R, &G, &B, height = 768, width = 512);
	readPGM(infile, &width, &height, &bitdepth, &R);

	assert(bitdepth == 255);

	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error(encoding).\n");
		exit(-1);
	}

	int bytes = encode(fp, R, height, width);
	//bytes += encode(fp, G, height, width);
	//bytes += encode(fp, B, height, width);

	printf("%d bytes. %f bpp\n", bytes, 8.0*bytes / (width*height));
	fclose(fp);

	free2D(R);
	//free2D(G);
	//free2D(B);
}

void runDecoder(char *codefile, char *outfile) {
	FILE *fp;

	unsigned char **recB, **recG, **recR;

	if (!(fp = fopen(codefile, "rb"))) {
		fprintf(stderr, "Code file open error(decoding).\n");
		exit(-1);
	}

	int height, width;
	decode(fp, &recR, &height, &width);
	//decode(fp, &recG, &height, &width);
	//decode(fp, &recB, &height, &width);

	//rawWrite(outfile, recR, recG, recB, height, width);
	writePGM(outfile, width, height, 255, recR);

	//free2D(recB);
	//free2D(recG);
	free2D(recR);
}

void runCompare(char *file1, char *file2) {
	int height, width, bitdepth;

	unsigned char **A, **B;

	readPGM(file1, &width, &height, &bitdepth, &A);
	readPGM(file2, &width, &height, &bitdepth, &B);

	float sum = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			sum += SQUARE((float)A[y][x] - B[y][x]);
		}
	}

	printf("PSNR = %f\n", 10 * log10(SQUARE(255) / (sum / width / height)));

	free2D(A);
	free2D(B);
}

void main(int argc, char *argv[]) {
	if (argc == 5 && argv[1][0] == 'e') {
		runEncoder(argv[2], argv[3], argv[4]);
	}
	else if (argc == 5 && argv[1][0] == 'd') {
		runDecoder(argv[2], argv[3], argv[4]);
	}
	else if (argc == 4 && argv[1][0] == 'c') {
		runCompare(argv[2], argv[3]);
	}
	else {
		printUsage(argv[0]);
	}
}
