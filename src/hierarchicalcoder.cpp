#pragma warning(disable: 4996)
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "encoder.h"
#include "preprocess.h"
#include "BMP.h"
#include "pgm_io.h"
#include "acfile/arithmetic_codec.h"
#include "hierarchicalcoder.h"

#define NUM_CTX 6*3
#define HOR 1
#define VER 0
#define FIXED_BITS 10
#define ARRAY_MAX 500000


int run_jasper8(int ** img, int height, int width, char* filename) {
	FILE *out = fopen("temp.pgm", "wb");
	unsigned char byte;
	fprintf(out, "P5\n");
	fprintf(out, "%d %d\n", width, height);
	fprintf(out, "255\n");
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			byte = (unsigned char)(img[i][j]);
			fwrite(&byte, sizeof(unsigned char), 1, out);
		}
	}
	fclose(out);

	char command[128];
	sprintf(command, "jasper.exe --input temp.pgm --output %s --output-format jpc", filename);
	int Jasper = system(command);
	if (Jasper) {
		fprintf(stderr, "Jasper error.\n");
		exit(-1);
	}

	struct stat st;
	stat(filename, &st);

	remove("temp.pgm");

	return st.st_size;
}

inline void endian_swap(unsigned short& x)
{
	x = (x >> 8) |
		(x << 8);
}

int run_jasper16(int ** img, int height, int width, char* filename) {
	FILE *out = fopen("temp.pgm", "wb");
	unsigned short byte;
	fprintf(out, "P5\n");
	fprintf(out, "%d %d\n", width, height);
	fprintf(out, "65535\n");
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			byte = (unsigned short)(img[i][j] + 256);
			endian_swap(byte);
			fwrite(&byte, sizeof(unsigned short), 1, out);
		}
	}
	fclose(out);

	char command[128];
	sprintf(command, "jasper.exe --input temp.pgm --output %s --output-format jpc", filename);
	int Jasper = system(command);
	if (Jasper) {
		fprintf(stderr, "Jasper error.\n");
		exit(-1);
	}
	struct stat st;
	stat(filename, &st);

	remove("temp.pgm");

	return st.st_size;
}

Hierarchical_coder::Hierarchical_coder(char filename[], char codename[], int _T, int _K) {

	if (print) {
		std::cout << "======== Encoder ========" << std::endl;
		std::cout << "Image : " << filename << std::endl;
	}

	preprocess(filename, &Y, &U_o1, &U_o2, &U_e1, &U_e2, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	// Encoding variables
	T = _T;
	K = _K;
	codefile = codename;

	for (int i = 0; i < 3 * K; i++) {
		if (i < 6) {
			symMaxes[i] = 10;
			p[i] = 0.6;
		}
		else if (i < 12) {
			symMaxes[i] = 20;
			p[i] = 0.7;
		}
		else {
			symMaxes[i] = 30;
			p[i] = 0.9;
		}
	}

	if(print) printf("(T, K, Height, Width) = (%d, %d, %d, %d)\n", T, K, height, width);
}

Hierarchical_coder::Hierarchical_coder(char filename[], int _T, int _K) {
	Hierarchical_coder(filename, "code.bin", _T, _K);
}

Hierarchical_coder::~Hierarchical_coder() {
	free2D(Y);
	free2D(U_e1);
	free2D(U_e2);
	free2D(U_o1);
	free2D(U_o2);
	free2D(V_e1);
	free2D(V_e2);
	free2D(V_o1);
	free2D(V_o2);
}

int Hierarchical_coder::encode_params(FILE *fp) {

	Arithmetic_Codec coder;

	coder.set_buffer(height*width);
	coder.start_encoder();

	// Encode T, K, height, width

	coder.put_bits(T, 4);
	coder.put_bits(K, 4);
	coder.put_bits(height, 16);
	coder.put_bits(width, 16);

	return coder.write_to_file(fp);;
}

void Hierarchical_coder::initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm) {

	for (int i = 0; i < 3 * K; i++) {
		pDm[i].set_alphabet(symMaxes[i] + 1);
		pDm[i].set_distribution(p[i]);
	}

	pCoder->set_buffer(height*width);
	pCoder->start_encoder();
}

float Hierarchical_coder::run() {

	int jasper_total_bytes = 0, u_total_bytes, v_total_bytes;

	// open output file
	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	// 0. Encode Parameters
	encode_params(fp);

	// 1. Encode Y, U_e2, V_e2 (Japser)
	int bytes, y_bytes;
	int half_height = height % 2 == 0 ? height / 2 : height / 2 + 1;
	int half_width = width % 2 == 0 ? width / 2 : width / 2 + 1;

	bytes = run_jasper8(Y, height, width, "y.jpc");
	if(print) printf("Y   (Jasper) : %d bytes\n", bytes);
	y_bytes = bytes;
	jasper_total_bytes += bytes;

	bytes = run_jasper16(U_e2, half_width, half_height, "u_e2_16.jpc");
	if (print) printf("Ue2 (Jasper) : %d bytes\n", bytes);
	jasper_total_bytes += bytes;

	bytes = run_jasper16(V_e2, half_width, half_height, "v_e2_16.jpc");
	if (print) printf("Ve2 (Jasper) : %d bytes\n", bytes);
	jasper_total_bytes += bytes;

	// Declare coder / data model for U,V channel each
	Arithmetic_Codec U_coder, V_coder;
	Adaptive_Data_Model U_dm[NUM_CTX], V_dm[NUM_CTX];

	initCoder(&U_coder, U_dm);
	initCoder(&V_coder, V_dm);

	// 2. Encode U_o2, U_o1 (LCIC)
	Encoder Uo2(U_o2, U_e2, T, K, symMaxes, width / 2, half_height);
	Encoder Uo1(U_o1, U_e1, T, K, symMaxes, height / 2, width);

	Uo2.run(&U_coder, U_dm, fp);
	Uo1.run(&U_coder, U_dm, fp);

	u_total_bytes = U_coder.write_to_file(fp);
	if (print) printf("Uo2 & Uo1 (LCIC) : %d bytes\n", u_total_bytes);

	// 3. Encode V_o2, V_o1 (LCIC)
	Encoder Vo2(V_o2, V_e2, T, K, symMaxes, width / 2, half_height);
	Encoder Vo1(V_o1, V_e1, T, K, symMaxes, height / 2, width);

	Vo2.run(&V_coder, V_dm, fp);
	Vo1.run(&V_coder, V_dm, fp);

	v_total_bytes = V_coder.write_to_file(fp);
	if (print) printf("Vo2 & Vo1 (LCIC) : %d bytes\n", v_total_bytes);

	fclose(fp);

	// Calculate bpp
	int total_bytes, uv_total_bytes;
	float bpp, uv_bpp;

	total_bytes = jasper_total_bytes + u_total_bytes + v_total_bytes;
	uv_total_bytes = u_total_bytes + v_total_bytes;

	bpp = 8.0*total_bytes / (width*height);
	uv_bpp = 8.0*uv_total_bytes / (0.75*width*height);

	if (print) {
		printf("Y Channel           : %d bytes. %f bpp\n", y_bytes, 8.0*y_bytes / (width*height));
		printf("U Channel           : %d bytes. %f bpp\n", u_total_bytes, 8.0*u_total_bytes / (0.75*width*height));
		printf("V Channel           : %d bytes. %f bpp\n", v_total_bytes, 8.0*v_total_bytes / (0.75*width*height));
		printf("Total (w/o japser)  : %d bytes. %f bpp\n", uv_total_bytes, uv_bpp);
		printf("Total (with jasper) : %d bytes. %f bpp\n", total_bytes, bpp);
	}

	//printf("Total (with jasper) : %d bytes. %f bpp\n", total_bytes, bpp);

	return bpp;
}

Hierarchical_decoder::Hierarchical_decoder() {
	for (int i = 0; i < NUM_CTX; i++) 
	{
		if (i < 6) {
			symMaxes[i] = 10;
			p[i] = 0.6;
		}
		else if (i < 12) {
			symMaxes[i] = 20;
			p[i] = 0.7;
		}
		else {
			symMaxes[i] = 30;
			p[i] = 0.9;
		}
		}
	std::cout << "======== Decoder ========" << std::endl;

}

Hierarchical_decoder::~Hierarchical_decoder() {
	free2D(Y);
	free2D(U_e1);
	free2D(U_e2);
	free2D(U_o1);
	free2D(U_o2);
	free2D(V_e1);
	free2D(V_e2);
	free2D(V_o1);
	free2D(V_o2);
}

int** Hierarchical_decoder::decode_jpeg2000_8(char* filename) {
	char command[128];
	sprintf(command, "jasper.exe --input %s --output temp.pgm", filename);
	int Jasper = system(command);
	if (Jasper) {
		fprintf(stderr, "Jasper error.\n");
		exit(-1);
	}

	unsigned char **data;

	int width, height, bitdepth;

	readPGM("temp.pgm", &width, &height, &bitdepth, &data);

	int **data_int = alloc2D(height, width);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			data_int[y][x] = data[y][x];

		}
	}

	remove("temp.pgm");

	return data_int;
}

int** Hierarchical_decoder::decode_jpeg2000_16(char* filename) {
	char command[128];
	sprintf(command, "jasper.exe --input %s --output temp.pgm", filename);
	int Jasper = system(command);
	if (Jasper) {
		fprintf(stderr, "Jasper error.\n");
		exit(-1);
	}

	unsigned short int **data;

	int width, height, bitdepth;

	readPGM_16bit("temp.pgm", &width, &height, &bitdepth, &data);

	int **data_int = alloc2D(height, width);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			if (data[y][x] % 2 == 0) {
				data_int[y][x] = (data[y][x] >> 8) - 256;
			}
			else {
				data_int[y][x] = data[y][x] >> 8;
			}

		}
	}

	remove("temp.pgm");

	return data_int;
}

int Hierarchical_decoder::decode_params(FILE *fp) {

	Arithmetic_Codec coder;

	coder.set_buffer(100);
	coder.read_from_file(fp);

	// Decode T, K, height, width

	T = coder.get_bits(4);
	K = coder.get_bits(4);
	height = coder.get_bits(16);
	width = coder.get_bits(16);

	printf("(T, K, Height, Width) = (%d, %d, %d, %d)\n", T, K, height, width);

	return coder.get_code_bytes();
}

void Hierarchical_decoder::initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp) {

	for (int i = 0; i < 3 * K; i++) {
		pDm[i].set_alphabet(symMaxes[i] + 1);
		pDm[i].set_distribution(p[i]);
	}

	pCoder->set_buffer(height*width);
	pCoder->read_from_file(fp);

}

int Hierarchical_decoder::run(char filename[], char imagename[]) {

	// Read in compressed data
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fputs("Code file open error.\n", stderr);
		exit(1);
	}

	decode_params(fp);

	// 1. Decode Y, U_e2, V_e2
	Y    = decode_jpeg2000_8("y.jpc");
	U_e2 = decode_jpeg2000_16("u_e2_16.jpc");
	V_e2 = decode_jpeg2000_16("v_e2_16.jpc");

	int half_height = height % 2 == 0 ? height / 2 : height / 2 + 1;
	int half_width = width % 2 == 0 ? width / 2 : width / 2 + 1;

	// 2. Decode U_o2, U_o1
	// a) Declare U_coder / U_dm
	
	Arithmetic_Codec U_coder;
	Adaptive_Data_Model U_dm[NUM_CTX];

	initCoder(&U_coder, U_dm, fp);

	// b) Decode U_o2
	printf("U_o2      : ");
	Decoder Uo2(U_e2, T, K, symMaxes, width / 2, half_height);
	U_o2 = Uo2.run(&U_coder, U_dm, fp);

	// c) Obtain U_e1 from U_o2
	int **U_e1_R;

	concat_image(&U_o2, &U_e2, &U_e1_R, &width, &half_height);
	rotate_image(&U_e1_R, &U_e1, 1, &width, &half_height);

	// d) Decode U_o1
	printf("U Channel : ");
	Decoder Uo1(U_e1, T, K, symMaxes, height / 2, width);

	U_o1 = Uo1.run(&U_coder, U_dm, fp);

	// 3. Decode V_o2, V_o1
	// a) Declare V_coder / V_dm
	Arithmetic_Codec V_coder;
	Adaptive_Data_Model V_dm[NUM_CTX];

	initCoder(&V_coder, V_dm, fp);

	// b) Decode V_o2
	printf("V_o2      : ");
	Decoder Vo2(V_e2, T, K, symMaxes, width / 2, half_height);
	V_o2 = Vo2.run(&V_coder, V_dm, fp);

	// c) Obtain V_e1 from V_o2
	int **V_e1_R;

	concat_image(&V_o2, &V_e2, &V_e1_R, &width, &half_height);
	rotate_image(&V_e1_R, &V_e1, 1, &width, &half_height);

	// d) Decode V_o1
	printf("V Channel : ");
	Decoder Vo1(V_e1, T, K, symMaxes, height / 2, width);
	V_o1 = Vo1.run(&V_coder, V_dm, fp);

	postprocess(imagename, &Y, &U_o1, &U_o2, &U_e2, &V_o1, &V_o2, &V_e2, &height, &width);

	fclose(fp);

	return 0;
}