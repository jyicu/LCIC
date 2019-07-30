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
#include "hierarchicalcoder.h"

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

int test_Kodak(int T, int K) {
	int num_files = 24;
	float bpp = 0;

	for (int i = 0; i < num_files; i++) {
		char filename[20];
		sprintf(filename, "./Kodak/kodim%02d.bmp", i + 1);
		Hierarchical_coder coder(filename, T, K);
		bpp += coder.run();
	}

	float avg_bpp = bpp / float(num_files);

	std::cout << "================ Kodak ================" << std::endl;
	printf("(%d, %d)", T, K);
	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;
	std::cout << "================ Kodak ================" << std::endl;

	return 0;
}

int test_classic(int T, int K) {
	int num_files = 4;
	float bpp = 0;

	Hierarchical_coder coder1("./classic/lena.bmp", T, K);
	bpp += coder1.run();

	Hierarchical_coder coder2("./classic/peppers.bmp", T, K);
	bpp += coder2.run();

	Hierarchical_coder coder3("./classic/mandrill.bmp", T, K);
	bpp += coder3.run();

	Hierarchical_coder coder4("./classic/barbara.bmp", T, K);
	bpp += coder4.run();

	float avg_bpp = bpp / float(num_files);

	std::cout << "================ Classic ================" << std::endl;
	printf("(%d, %d)", T, K);
	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;
	std::cout << "================ Classic ================" << std::endl;

	return 0;
}

int test_medical(int T, int K) {

	int num_files = 8;
	float bpp = 0;

	Hierarchical_coder coder1("./medical/[0]pet1.bmp", T, K);
	bpp += coder1.run();

	Hierarchical_coder coder2("./medical/[0]pet2.bmp", T, K);
	bpp += coder2.run();

	Hierarchical_coder coder3("./medical/[0]pet3.bmp", T, K);
	bpp += coder3.run();

	Hierarchical_coder coder4("./medical/[1]eye1.bmp", T, K);
	bpp += coder4.run();

	Hierarchical_coder coder5("./medical/[1]eye2.bmp", T, K);
	bpp += coder5.run();

	Hierarchical_coder coder6("./medical/[2]eyeground.bmp", T, K);
	bpp += coder6.run();

	Hierarchical_coder coder7("./medical/endoscope1.bmp", T, K);
	bpp += coder7.run();

	Hierarchical_coder coder8("./medical/endoscope2.bmp", T, K);
	bpp += coder8.run();

	float avg_bpp = bpp / float(num_files);

	std::cout << "================ Medical ================" << std::endl;
	printf("(%d, %d)", T, K);
	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;
	std::cout << "================ Medical ================" << std::endl;

	return 0;
}

int test_digital_cam(int T, int K) {
	int num_files = 8;
	float bpp = 0;

	Hierarchical_coder coder1("./digital_cam/berry.bmp", T, K);
	bpp += coder1.run();

	Hierarchical_coder coder2("./digital_cam/ceiling.bmp", T, K);
	bpp += coder2.run();

	Hierarchical_coder coder3("./digital_cam/fireworks.bmp", T, K);
	bpp += coder3.run();

	Hierarchical_coder coder4("./digital_cam/flamingo.bmp", T, K);
	bpp += coder4.run();

	Hierarchical_coder coder5("./digital_cam/flower.bmp", T, K);
	bpp += coder5.run();

	Hierarchical_coder coder6("./digital_cam/locks.bmp", T, K);
	bpp += coder6.run();

	Hierarchical_coder coder7("./digital_cam/park.bmp", T, K);
	bpp += coder7.run();

	Hierarchical_coder coder8("./digital_cam/sunset.bmp", T, K);
	bpp += coder8.run();

	float avg_bpp = bpp / float(num_files);

	std::cout << "================ Digital Cam ================" << std::endl;
	printf("(%d, %d)", T, K);
	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;
	std::cout << "================ Digital Cam ================" << std::endl;

	return 0;
}

float run_jasper(char* filename, char* filename_jpc) {

	int **R, **G, **B;
	int height, width;

	bmpRead(filename, &R, &G, &B, &height, &width);

	char command[128];
	sprintf(command, "jasper.exe --input %s --output %s --output-format jpc", filename, filename_jpc);
	system(command);

	struct stat st;
	stat(filename_jpc, &st);

	int code_bytes = st.st_size;

	float bpp = 8.0 * code_bytes / (width*height);

	printf("%d bytes. %f bpp\n", code_bytes, bpp);

	return bpp;
}

int test_kodak_jasper() {

	int num_files = 24;
	float bpp = 0;

	char filename[20];
	char filename_jpc[20];

	for (int i = 0; i < num_files; i++) {
		if (i < 9) {
			sprintf(filename, "./Kodak/kodim0%d.pnm", i + 1);
			sprintf(filename_jpc, "./result/kodim0%d.jpc", i + 1);
		}
		else {
			sprintf(filename, "./Kodak/kodim%d.pnm", i + 1);
			sprintf(filename_jpc, "./result/kodim%d.jpc", i + 1);
		}
		
		printf("==== Image : %s ====\n", filename);

		bpp += run_jasper(filename, filename_jpc);

	}

	float avg_bpp = bpp / float(num_files);

	std::cout << "Average bpp : " << avg_bpp << "bpp" << std::endl;

	return 0;
}

void printUsage(char *s) {
	printf("Usage: %s e [source file (bmp)] [compressed file (bin)]\n", s);
	printf("Usage: %s d [compressed file (bin)] [decoded file (bmp)]\n", s);
}

void runEncoder(char *infile, char *codefile) {
	int T = 3;
	int K = 6;
	Hierarchical_coder hc(infile, codefile, T, K);
	hc.run();

}

void runDecoder(char *codefile, char *outfile) {
	Hierarchical_decoder hd;
	hd.run(codefile, outfile);
}

void main(int argc, char *argv[]) {
	if (argc == 4 && argv[1][0] == 'e') {
		runEncoder(argv[2], argv[3]);
	}
	else if (argc == 4 && argv[1][0] == 'd') {
		runDecoder(argv[2], argv[3]);
	}
	else {
		printUsage(argv[0]);
	}
}


void test() {

	char infile[] = "./classic/lena.bmp";

	int T = 3;
	int K = 6;

	//test_Kodak(T,K);
	//test_medical(T, K);
	//test_classic(T, K);;
	//test_digital_cam(T, K);

	//test_kodak_jasper();
	//run_jasper(infile, "result/mandrill.jpc");
	//check_result();

	Hierarchical_coder hc(infile, T, K);
	hc.run();

	Hierarchical_decoder hd;
	hd.run("code.bin", "out.bmp");
	
	return;
}