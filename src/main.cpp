#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <windows.h>

#include "acfile\arithmetic_codec.h"
#include "BMP.h"
#include "preprocess.h"

#pragma warning(disable: 4996)

typedef unsigned char UINT8;

#define NUM_CTX	10

#define MAX(x,y)	((x)>(y) ? (x) : (y))
#define MIN(x,y)	((x)<(y) ? (x) : (y))
#define MAX4(X)		MAX(MAX((X)[0],(X)[1]),MAX((X)[2],(X)[3]))
#define MIN4(X)		MIN(MIN((X)[0],(X)[1]),MIN((X)[2],(X)[3]))
#define ABS(x)		((x)>0 ? (x) : -(x))
#define CLIP(x,y,z)	MAX((y), MIN((x), (z)))
#define CLIP255(x)	MAX(0, MIN((x), 255))
#define ROUND(x)	((int)((x)+0.5))
#define UINT8(x)	CLIP255(ROUND(x))

void encodeMag(int mag, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {
	int symMax = 10;

	if (mag >= symMax) {
		pCoder->encode(symMax, *pDm);
		mag -= symMax;
		pCoder->put_bit(mag & 1);
		encodeMag(mag >> 1, pCoder, pDm);
	}
	else {
		pCoder->encode(mag, *pDm);
	}
}

int encode_odd(FILE *fp, int **R, int **G, int **B, int height, int width) {
	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];
	int ctxCnt[NUM_CTX];

	int y, x;
	int numPix = 0;

	for (int i = 0; i<NUM_CTX; i++) {
		dm[i].set_alphabet(11);
		dm[i].set_distribution(0.7);
		ctxCnt[i] = 0;
	}

	int size = width * height;
	coder.set_buffer(size);
	coder.start_encoder();

	// first row

	// middle rows
	for (y = 1; y < height-1; y++) {
		for (x = 1; x < width-1; x += 2) {
			float sumG = G[y - 1][x - 1] + G[y - 1][x] + G[y - 1][x + 1] + G[y][x - 1] + G[y][x + 1] + G[y + 1][x - 1] + G[y + 1][x + 1];
			float sumR = R[y - 1][x - 1] + R[y - 1][x] + R[y - 1][x + 1] + R[y][x - 1] + R[y][x + 1] + R[y + 1][x - 1] + R[y + 1][x + 1];
			float sumG2 = (float)G[y - 1][x - 1] * G[y - 1][x - 1] + G[y - 1][x] * G[y - 1][x] + G[y - 1][x + 1] * G[y - 1][x + 1] + G[y][x - 1] * G[y][x - 1]
				+ G[y][x + 1] * G[y][x + 1] + G[y + 1][x - 1] * G[y + 1][x - 1] + G[y + 1][x + 1] * G[y + 1][x + 1];
			float sumR2 = (float)R[y - 1][x - 1] * R[y - 1][x - 1] + R[y - 1][x] * R[y - 1][x] + R[y - 1][x + 1] * R[y - 1][x + 1] + R[y][x - 1] * R[y][x - 1]
				+ R[y][x + 1] * R[y][x + 1] + R[y + 1][x - 1] * R[y + 1][x - 1] + R[y + 1][x + 1] * R[y + 1][x + 1];
			float sumGR = (float)G[y - 1][x - 1] * R[y - 1][x - 1] + G[y - 1][x] * R[y - 1][x] + G[y - 1][x + 1] * R[y - 1][x + 1] + G[y][x - 1] * R[y][x - 1]
				+ G[y][x + 1] * R[y][x + 1] + G[y + 1][x - 1] * R[y + 1][x - 1] + G[y + 1][x + 1] * R[y + 1][x + 1];

			// correlation coefficient and parameters (a,b) for model (R=aG+b)
			float CC = (7.0*sumGR - sumG * sumR) / sqrt((7.0*sumG2 - sumG * sumG)*(7.0*sumR2 - sumR * sumR));
			float a = (7.0*sumGR - sumG * sumR) / (7.0*sumG2 - sumG * sumG);
			float b = (sumR - a * sumG) / 7.0;	// typo in the paper(interband CALIC)

			int pred;

			if (ABS(a) <0.1 || ABS(CC)<0.5) {
				pred = CLIP255(ROUND(sumR / 7.0));
			}
			else {
				pred = CLIP255(ROUND(a*G[y][x] + b));
			}
			int res = R[y][x] - pred;
			int sym = res >= 0 ? 2 * res : -2 * res - 1;

			//int ctx = CLIP(10 * (ABS(CC) - 0.4), 0, NUM_CTX - 1);
			int ctx = CLIP(ABS(R[y][x-1]-R[y][x+1])/4, 0, NUM_CTX - 1);
			
			encodeMag(sym, &coder, &dm[ctx]);

			numPix++;
		}
	}

	// last col

	// last row
	
	int bytes = coder.write_to_file(fp);
	printf("%d bytes. %f bpp\n", bytes, 8.0*bytes / numPix);

	return bytes;
}

void main(int argc, char *argv[]) {
	
	//char infile[] = "SS15-17680;1;A1;1_crop3.bmp";
	char infile[] = "suzy.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";
	char codefile_out[] = "code_out.bin";
	FILE *fp;
	FILE *fp2;

	int **R, **G, **B;

	int height, width;

	bmpRead(infile, &R, &G, &B, &height, &width);

	assert(height% 2 == 0);
	assert(width % 2 == 0);

	check_result();

	int **R2 = alloc2D(height/2, width);
	int **G2 = alloc2D(height/2, width);
	int **B2 = alloc2D(height/2, width);

	for (int y = 0; y < height; y++) {
		for (int x =0; x < width ; x+=1) {
			R2[y/2][x] = R[y][x];
			G2[y/2][x] = G[y][x];
			B2[y/2][x] = B[y][x];
		}
	}
	bmpWrite(outfile, R2, G2, B2, height/2, width);

	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	if (!(fp2 = fopen(codefile_out, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	int bytes = encode_odd(fp, R, G, B, height, width);

//	printf("%d bytes. %f bpp\n", bytes, 8.0*bytes / (height - 2) / (width / 2-1));

	fclose(fp);
	free2D(R);
	free2D(G);
	free2D(B);
	free2D(R2);
	free2D(G2);
	free2D(B2);
}


/* TODO
YUV로 변환해서 decomposition
last col, first/last row 처리
계산 최적화
16bit version
res->sym 최적화
heavy-tail pdf
context 최적화 -> 2차원?

medical image는 흑백적 위주 -> 이걸 context로?
의료영상에 적합한 트랜스폼 찾기 (G+R/2, R-G, B)?

큰 파일 처리
*/