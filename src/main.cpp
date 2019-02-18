#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <vector>
#include <windows.h>
#include <iostream>

#include "acfile\arithmetic_codec.h"
#include "BMP.h"
#include "preprocess.h"
#include "encoder.h"

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


int test_Kodak() {
	int num_files = 24;
	int bytes = 0;
	int num_pix = 0;
	for (int i = 0; i < num_files; i++) {
		char filename[20];
		sprintf(filename, "./Kodak/kodim%d.bmp", i + 1);
		Hierarchical_coder coder(filename, 3, 6, 40);
		bytes += coder.run();
		num_pix;
	}
	float avg_bpp = float(bytes) / num_pix;
	return 0;
}


void main(int argc, char *argv[]) {
	

	//char infile[] = "./Kodak/kodim05.bmp"; //SS15-17680;1;A1;1_crop3.bmp";
	char infile[] = "./Kodak/test.bmp"; //SS15-17680;1;A1;1_crop3.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";
	FILE *fp;

	int **R;
	int **G;
	int **B;
	int **Y;
	int **U;
	int **V;
	int height, width;

	int **U_o1, **U_o2, **U_e1, **U_e2, **V_o1, **V_o2, **V_e1, **V_e2;

	int T = 3;
	int K = 6;
	int symmax = 40;

	preprocess(infile, &Y, &U_o1, &U_o2, &U_e1, &U_e2, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	Encoder encoder_test(U_o1, U_e1, T, K, symmax, height/2, width);
	encoder_test.run_test();

	int **U_o1_decoded;

	Decoder decoder_test(U_e1, T, K, symmax, height/2, width);
	U_o1_decoded = decoder_test.run_test();

	postprocess("Decoded_result.bmp", &Y, &U_o1_decoded, &U_o2, &U_e2, &V_o1, &V_o2, &V_e2, &height, &width);

	//Hierarchical_coder hc(infile, T, K, symmax);
	//hc.run();

	//bmpRead(infile, &R, &G, &B, &height, &width);

	//RGB2YUV(&R, &G, &B, &Y, &U, &V, &height, &width);
	////Encoder enc(U, T, K, symmax, height, width);
	////enc.run();

	//int **X_o, **X_e;

	//split_image(&U, &X_o, &X_e, &height, &width);

	//Hierarchical_decoder hd(T, K, symmax, height, width);
	//hd.run("code.bin");

	//free2D(R);
	//free2D(G);
	//free2D(B);
	//free2D(Y);
	//free2D(U);
	//free2D(V);
	return;
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