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
	system(command);

	struct stat st;
	stat(filename, &st);

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
	system(command);

	struct stat st;
	stat(filename, &st);

	return st.st_size;
}

Hierarchical_coder::Hierarchical_coder(char filename[], int _T, int _K, int _symMax) {

	std::cout << "======== Encoder ========" << std::endl;
	std::cout << "Image : " << filename << std::endl;

	preprocess(filename, &Y, &U_o1, &U_o2, &U_e1, &U_e2, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	// Encoding variables
	T = _T;
	K = _K;
	symMax = _symMax;
	for (int i = 0; i < NUM_CTX; i++) {
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

	printf("(T, K, symMax, Height, Width) = (%d, %d, %d, %d, %d)\n", T, K, symMax, height, width);
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

float Hierarchical_coder::run() {

	int jasper_total_bytes = 0, u_total_bytes, v_total_bytes;

	// open output file
	char codefile[] = "code.bin";

	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	// 0. Encode Parameters
	encode_params(fp);

	// 1. Encode Y, U_e2, V_e2
	int bytes;
	bytes = run_jasper8(Y, height, width, "y.jpc");
	printf("Y:%d\n", bytes);
	jasper_total_bytes += bytes;
	bytes = run_jasper16(U_e2, width / 2, height / 2, "u_e2_16.jpc");
	printf("Ue2:%d\n", bytes);
	jasper_total_bytes += bytes;
	bytes = run_jasper16(V_e2, width / 2, height / 2, "v_e2_16.jpc");
	printf("Ve2:%d\n", bytes);
	jasper_total_bytes += bytes;
	// Declare coder / data model for U,V channel each
	Arithmetic_Codec U_coder, V_coder;
	Adaptive_Data_Model U_dm[NUM_CTX], V_dm[NUM_CTX];

	initCoder(&U_coder, U_dm);
	initCoder(&V_coder, V_dm);

	// 2. Encode U_o2, U_o1
	Encoder Uo2(U_o2, U_e2, T, K, symMaxes, width / 2, height / 2);
	Encoder Uo1(U_o1, U_e1, T, K, symMaxes, height / 2, width);

	Uo2.run(&U_coder, U_dm, fp);
	Uo1.run(&U_coder, U_dm, fp);

	u_total_bytes = U_coder.write_to_file(fp);
	printf("Ut:%d\n", u_total_bytes);
	// 3. Encode V_o2, V_o1
	Encoder Vo2(V_o2, V_e2, T, K, symMaxes, width / 2, height / 2);
	Encoder Vo1(V_o1, V_e1, T, K, symMaxes, height / 2, width);

	Vo2.run(&V_coder, V_dm, fp);
	Vo1.run(&V_coder, V_dm, fp);

	v_total_bytes = V_coder.write_to_file(fp);
	printf("Vt:%d\n", v_total_bytes);
	fclose(fp);

	// Calculate bpp
	int total_bytes, uv_total_bytes;
	float bpp, uv_bpp;

	total_bytes = jasper_total_bytes + u_total_bytes + v_total_bytes;
	uv_total_bytes = u_total_bytes + v_total_bytes;

	bpp = 8.0*total_bytes / (width*height);
	uv_bpp = 8.0*uv_total_bytes / (0.75*width*height);

	printf("U Channel           : %d bytes. %f bpp\n", u_total_bytes, 8.0*u_total_bytes / (0.75*width*height));
	printf("V Channel           : %d bytes. %f bpp\n", v_total_bytes, 8.0*v_total_bytes / (0.75*width*height));
	printf("Total (w/o japser)  : %d bytes. %f bpp\n", uv_total_bytes, uv_bpp);
	printf("Total (with jasper) : %d bytes. %f bpp\n", total_bytes, bpp);

	return bpp;
}

int Hierarchical_coder::encode_params(FILE *fp) {

	Arithmetic_Codec coder;

	coder.set_buffer(height*width);
	coder.start_encoder();

	// Encode T, K, symMax, height, width

	coder.put_bits(T, 4);
	coder.put_bits(K, 4);
	coder.put_bits(symMax, 8);
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

int** Hierarchical_decoder::decode_jpeg2000(char* filename) {
	char command[128];
	sprintf(command, "jasper.exe --input %s --output temp.bmp", filename);
	system(command);

	int **data;

	bmpRead_1c("temp.bmp", &data);

	remove("temp.bmp");

	return data;
}

int Hierarchical_decoder::run(char filename[], char imagename[]) {

	// Read in compressed data
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fputs("error", stderr);
		exit(1);
	}

	decode_params(fp);

	// 1. Decode Y, U_e2, V_e2
	printf("Image : %s\n", imagename);

	int **temp1, **temp2, **temp3, **temp4, **temp5, **temp6;

	preprocess(imagename, &Y, &temp1, &temp2, &temp3, &U_e2, &temp4, &temp5, &temp6, &V_e2, &height, &width);

	//Y    = decode_jpeg2000("y.jpc");
	//U_e2 = decode_jpeg2000("u_e2_16.jpc");
	//V_e2 = decode_jpeg2000(compressed_data);

	// 2. Decode U_o2, U_o1
	// a) Declare U_coder / U_dm
	
	Arithmetic_Codec U_coder;
	Adaptive_Data_Model U_dm[NUM_CTX];

	initCoder(&U_coder, U_dm, fp);

	// b) Decode U_o2
	printf("U_o2      : ");
	Decoder Uo2(U_e2, T, K, symMaxes, width / 2, height / 2);
	U_o2 = Uo2.run(&U_coder, U_dm, fp);

	// c) Obtain U_e1 from U_o2
	int **U_e1_R;
	int half_height = height / 2;

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
	Decoder Vo2(V_e2, T, K, symMaxes, width / 2, height / 2);
	V_o2 = Vo2.run(&V_coder, V_dm, fp);

	// c) Obtain V_e1 from V_o2
	int **V_e1_R;

	concat_image(&V_o2, &V_e2, &V_e1_R, &width, &half_height);
	rotate_image(&V_e1_R, &V_e1, 1, &width, &half_height);

	// d) Decode V_o1
	printf("V Channel : ");
	Decoder Vo1(V_e1, T, K, symMaxes, height / 2, width);
	V_o1 = Vo1.run(&V_coder, V_dm, fp);

	postprocess("Decoded_Result.bmp", &Y, &U_o1, &U_o2, &U_e2, &V_o1, &V_o2, &V_e2, &height, &width);

	fclose(fp);

	return 0;
}

int Hierarchical_decoder::decode_params(FILE *fp) {

	Arithmetic_Codec coder;

	coder.set_buffer(100);
	coder.read_from_file(fp);

	// Decode T, K, symMax, height, width

	T = coder.get_bits(4);
	K = coder.get_bits(4);
	symMax = coder.get_bits(8);
	height = coder.get_bits(16);
	width = coder.get_bits(16);

	printf("(T, K, symMax, Height, Width) = (%d, %d, %d, %d, %d)\n", T, K, symMax, height, width);

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