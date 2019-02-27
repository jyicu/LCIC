#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"
#include <vector>
#include <iostream>

#include "acfile\arithmetic_codec.h"
#include "BMP.h"
#include "preprocess.h"
#include "encoder.h"

#pragma warning(disable: 4996)

typedef unsigned char UINT8;

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
	float bpp = 0;

	for (int i = 0; i < num_files; i++) {
		char filename[20];
		if (i < 9)
			sprintf(filename, "./Kodak/kodim0%d.bmp", i + 1);
		else
			sprintf(filename, "./Kodak/kodim%d.bmp", i + 1);
		Hierarchical_coder coder(filename, 3, 6, 40);
		bpp += coder.run();
	}

	float avg_bpp = bpp / float(num_files);

	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;

	return 0;
}


void main(int argc, char *argv[]) {

	//test_Kodak();

	//check_result();

	//char infile[] = "./Kodak/kodim05.bmp"; //SS15-17680;1;A1;1_crop3.bmp";
	char infile[] = "lena.bmp"; //SS15-17680;1;A1;1_crop3.bmp";
	char outfile[] = "lev2.bmp";
	char codefile[] = "code.bin";
	FILE *fp;

	int T = 3;
	int K = 6;
	int symmax = 40;

	Hierarchical_coder hc(infile, T, K, symmax);
	hc.run();

	Hierarchical_decoder hd(T, K, symmax, 512, 512);
	hd.run("code.bin");
	
	return;
}


/* TODO
YUV�� ��ȯ�ؼ� decomposition
last col, first/last row ó��
��� ����ȭ
16bit version
res->sym ����ȭ
heavy-tail pdf
context ����ȭ -> 2����?

medical image�� ����� ���� -> �̰� context��?
�Ƿ῵�� ������ Ʈ������ ã�� (G+R/2, R-G, B)?

ū ���� ó��
*/