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

#define NUM_CTX 6
#define HOR 1
#define VER 0
#define FIXED_BITS 10
#define ARRAY_MAX 500000

template <typename T>
T **alloc2D(int height, int width) {
	T *arr = new T[height*width];
	T **pp = new T*[height];
	memset(arr, 0, height*width);

	for (int y = 0; y < height; y++)
		pp[y] = &(arr[y*width]);

	return pp;
}

template <typename T>
void free2D(T **p) {
	delete(p[0]);
	delete(p);
}

inline bool dir(int x_o, int T, int x_v, int x_h) {
	return ((ABS(x_o - x_v)) > (ABS(x_o - x_h) + T) ? (HOR) : (VER));
}

int run_jasper8(int ** img, int height, int width, char* filename) {
	FILE *out = fopen("temp.pgm", "wb");
	unsigned char byte;
	fprintf(out, "P5\n");
	fprintf(out, "%d %d\n", width, height);
	fprintf(out, "255\n");
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			byte = (int)(img[i][j]);
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

Hierarchical_coder::Hierarchical_coder(char filename[], int _T, int _K, int _symMax) {

	std::cout << "======== Encoder ========" << std::endl;
	std::cout << "Image : " << filename << std::endl;

	preprocess(filename, &Y, &U_o1, &U_o2, &U_e1, &U_e2, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	// Encoding variables
	T = _T;
	K = _K;
	symMax = _symMax;

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

int Hierarchical_coder::run() {

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
	jasper_total_bytes += run_jasper8(Y, height, width, "y.jpc");
	jasper_total_bytes += run_jasper8(U_e2, width / 2, height / 2, "u_e2.jpc");
	jasper_total_bytes += run_jasper8(V_e2, width / 2, height / 2, "v_e2.jpc");

	// Declare coder / data model for U,V channel each
	Arithmetic_Codec U_coder, V_coder;
	Adaptive_Data_Model U_dm[NUM_CTX], V_dm[NUM_CTX];

	initCoder(&U_coder, U_dm);
	initCoder(&V_coder, V_dm);

	// 2. Encode U_o2, U_o1
	Encoder Uo2(U_o2, U_e2, T, K, symMax, width / 2, height / 2);
	Encoder Uo1(U_o1, U_e1, T, K, symMax, height / 2, width);

	Uo2.run(&U_coder, U_dm, fp);
	Uo1.run(&U_coder, U_dm, fp);

	u_total_bytes = U_coder.write_to_file(fp);

	// 3. Encode V_o2, V_o1
	Encoder Vo2(V_o2, V_e2, T, K, symMax, width / 2, height / 2);
	Encoder Vo1(V_o1, V_e1, T, K, symMax, height / 2, width);

	Vo2.run(&V_coder, V_dm, fp);
	Vo1.run(&V_coder, V_dm, fp);

	v_total_bytes = V_coder.write_to_file(fp);

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

	coder.put_bits(T,       4);
	coder.put_bits(K,       4);
	coder.put_bits(symMax,  9);
	coder.put_bits(height, 20);
	coder.put_bits(width,  20);

	return coder.write_to_file(fp);;
}

void Hierarchical_coder::initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm) {

	for (int i = 0; i < K; i++) {
		pDm[i].set_alphabet(symMax + 1);
		pDm[i].set_distribution(0.9);
	}

	pCoder->set_buffer(height*width);
	pCoder->start_encoder();
}

Hierarchical_decoder::Hierarchical_decoder() {

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
	sprintf(command, "jasper.exe --input %s --output temp.bmp",filename);
	system(command);
	remove("temp.bmp");
	// Need 1 channel BMP read
	return alloc2D<int>(height, width);
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

	//Y    = decode_jpeg2000(compressed_data);
	//U_e2 = decode_jpeg2000(compressed_data);
	//V_e2 = decode_jpeg2000(compressed_data);

	// 2. Decode U_o2, U_o1
	// a) Declare U_coder / U_dm

	Arithmetic_Codec U_coder;
	Adaptive_Data_Model U_dm[NUM_CTX];

	initCoder(&U_coder, U_dm, fp);

	// b) Decode U_o2
	printf("U_o2      : ");
	Decoder Uo2(U_e2, T, K, symMax, width / 2, height / 2);
	U_o2 = Uo2.run(&U_coder, U_dm, fp);

	// c) Obtain U_e1 from U_o2
	int **U_e1_R;
	int half_height = height / 2;

	concat_image(&U_o2, &U_e2, &U_e1_R, &width, &half_height);
	rotate_image(&U_e1_R, &U_e1, 1, &width, &half_height);

	// d) Decode U_o1
	printf("U Channel : ");
	Decoder Uo1(U_e1, T, K, symMax, height / 2, width);

	U_o1 = Uo1.run(&U_coder, U_dm, fp);

	// 3. Decode V_o2, V_o1
	// a) Declare V_coder / V_dm
	Arithmetic_Codec V_coder;
	Adaptive_Data_Model V_dm[NUM_CTX];
	
	initCoder(&V_coder, V_dm, fp);
	
	// b) Decode V_o2
	printf("V_o2      : ");
	Decoder Vo2(V_e2, T, K, symMax, width / 2, height / 2);
	V_o2 = Vo2.run(&V_coder, V_dm, fp);

	// c) Obtain V_e1 from V_o2
	int **V_e1_R;

	concat_image(&V_o2, &V_e2, &V_e1_R, &width, &half_height);
	rotate_image(&V_e1_R, &V_e1, 1, &width, &half_height);

	// d) Decode V_o1
	printf("V Channel : ");
	Decoder Vo1(V_e1, T, K, symMax, height / 2, width);
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

	T      = coder.get_bits(4);
	K      = coder.get_bits(4);
	symMax = coder.get_bits(9);
	height = coder.get_bits(20);
	width  = coder.get_bits(20);

	printf("(T, K, symMax, Height, Width) = (%d, %d, %d, %d, %d)\n", T, K, symMax, height, width);

	return coder.get_code_bytes();
}

void Hierarchical_decoder::initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp) {

	for (int i = 0; i < K; i++) {
		pDm[i].set_alphabet(symMax + 1);
		pDm[i].set_distribution(0.9);
	}

	pCoder->set_buffer(height*width);
	pCoder->read_from_file(fp);

}

Encoder::Encoder(int ** _X_o, int ** _X_e, int _T, int _K, int symMax_, int _height, int _width) {

	X_o = _X_o;
	X_e = _X_e;
	T = _T;
	K = _K;
	symMax = symMax_;
	height = _height;
	width = _width;
	q = new int[K-1];
	Dir = alloc2D<bool>(height, width);
	init();

}

Encoder::~Encoder() {
	free2D(sigma);
	free2D(Dir);
	delete[] q;
}

void Encoder::init() {
	set_local_activity();
	context_modeling();
}

void Encoder::set_local_activity() {

	int y, x;

	sigma = alloc2D<int>(height, width);

	for (y = 0; y < height - 1; y++) {
		for (x = 0; x < width; x++) {
			sigma[y][x] = ABS(X_e[y][x] - X_e[y + 1][x]);
		}
	}

	// Last row
	for (x = 0; x < width; x++) {
		sigma[height - 1][x] = sigma[height - 2][x];
	}

}

void Encoder::context_modeling() {

	int N = height * width;
	int * temp = new int[N];
	int interval = N / K;

	std::copy(sigma[0], sigma[0] + N, temp);
	std::sort(temp, temp + N);

	for (int i = 0; i < K - 1; i++) {
		q[i] = temp[(i + 1) * interval];
	}

	delete[] temp;

}

int Encoder::context(int x, int y) {

	for (int i = 0; i < K - 1; i++) {
		if (sigma[y][x] <= q[i]) return i;
	}
	return K - 1;

}

void Encoder::encodeMag(unsigned int sym, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {

	if (sym >= symMax) {
		pCoder->encode(symMax, *pDm);
		pCoder->put_bits(sym, FIXED_BITS);
	}
	
	else 
		pCoder->encode(sym, *pDm);
	
}

void Encoder::encodeDir(int dir, Arithmetic_Codec *pCoder) {

	pCoder->put_bit(dir);

}

bool Encoder::eitherHOR(int x, int y) {

	if (y == 0 && x == 0)
		return false;

	else if (y == 0 && x != 0) {
		if (Dir[y][x - 1] == HOR)
			return true;
		else
			return false;
	}

	else if (y != 0 && x == 0) {
		if (Dir[y - 1][x] == HOR)
			return true;
		else
			return false;
	}

	else {
		if (Dir[y - 1][x] == HOR || Dir[y][x - 1] == HOR)
			return true;
		else
			return false;
	}
}

void Encoder::initCoder(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {

	for (int i = 0; i < K; i++) {
		pDm[i].set_alphabet(symMax + 1);
		pDm[i].set_distribution(0.9);
	}

	pCoder->set_buffer(height*width);
	pCoder->start_encoder();
}

int Encoder::run(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp) {

	// Counter Initialize
	int Dir_counter[2] = { 0 };
	int sym_counter[512] = { 0 };
	int context_counter[NUM_CTX] = { 0 };
	int x_h_counter = 0;
	int numPix = 0;

	// Encoding variables
	int y, x;
	int x_o, x_h, x_v;
	int pred, res, ctx;
	unsigned int sym;


	// First Pixel
	{
		y = 0;
		x = 0;

		x_o = X_o[y][x];
		x_v = ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

		// Set First pixel direction as vertical
		Dir[y][x] = VER;
		pred = x_v;

		// Encode
		res = x_o - pred;
		sym = MAP(res);
		ctx = context(x, y);

		encodeMag(sym, pCoder, &pDm[ctx]);

		// Counter
		Dir_counter[Dir[y][x]]++;
		sym_counter[sym]++;
		context_counter[ctx]++;

		numPix++;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			if (x == 0 && y == 0)
				continue;

			x_o = X_o[y][x];

			//===========================//
			//  HOR/VER pixel prediction //
			//===========================//
			x_h = (x == 0) ? X_o[y - 1][x] : X_o[y][x - 1];
			x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

			//===========================//
			//     Encode Direction      //
			//===========================//
			Dir[y][x] = dir(x_o, T, x_v, x_h);

			if (eitherHOR(x, y)) {
				encodeDir(Dir[y][x], pCoder);
				pred = (Dir[y][x] == HOR) ? x_h : x_v;
				if (Dir[y][x] == HOR)
					x_h_counter++;
			}
			else
				pred = x_v;

			//===========================//
			//      Encode Symbol        //
			//===========================//
			res = x_o - pred;
			sym = MAP(res);
			ctx = context(x, y);

			encodeMag(sym, pCoder, &pDm[ctx]);

			Dir_counter[Dir[y][x]]++;
			sym_counter[sym]++;
			context_counter[ctx]++;

			numPix++;
		}
	}

	//coder
	float proportion = float(x_h_counter) / numPix;
	//printf("freq of selecting H prediction : %f\n", proportion);

	return 0;
}

Decoder::Decoder(int **X_e_, int T_, int K_, int symmax_, int height_, int width_) {

	X_e = X_e_;
	T = T_;
	K = K_;
	symMax = symmax_;
	height = height_;
	width = width_;
	q = new int[K - 1];
	Dir = alloc2D<bool>(height, width);
	X_o = alloc2D<int>(height, width);
	init();
}

void Decoder::init() {

	set_local_activity();
	context_modeling();
}

Decoder::~Decoder() {
	free2D(sigma);
	free2D(Dir);
	delete[] q;
}

void Decoder::set_local_activity() {

	int y, x;

	sigma = alloc2D<int>(height, width);

	for (y = 0; y < height - 1; y++) {
		for (x = 0; x < width; x++) {
			sigma[y][x] = ABS(X_e[y][x] - X_e[y + 1][x]);
		}
	}

	// Last row
	for (x = 0; x < width; x++) {
		sigma[height - 1][x] = sigma[height - 2][x];
	}

}

void Decoder::context_modeling() {

	int N = height * width;
	int * temp = new int[N];
	int interval = N / K;

	std::copy(sigma[0], sigma[0] + N, temp);
	std::sort(temp, temp + N);

	for (int i = 0; i < K - 1; i++) {
		q[i] = temp[(i + 1) * interval];
	}

	delete[] temp;

}

int Decoder::context(int x, int y) {

	for (int i = 0; i < K - 1; i++) {
		if (sigma[y][x] <= q[i]) return i;
	}
	return K - 1;

}

void Decoder::initCoder(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm, FILE *fp) {

	for (int i = 0; i < K; i++) {
		pDm[i].set_alphabet(symMax + 1);
		pDm[i].set_distribution(0.9);
	}

	pCoder->set_buffer(height*width);
	pCoder->read_from_file(fp);
}

unsigned int Decoder::decodemag(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {

	unsigned int sym;

	sym = pCoder->decode(*pDm);

	if (sym == symMax)
		return pCoder->get_bits(FIXED_BITS);

	else
		return sym;

}

bool Decoder::eitherHOR(int x, int y) {

	if (y == 0 && x == 0)
		return false;

	else if (y == 0 && x != 0) {
		if (Dir[y][x - 1] == HOR)
			return true;
		else
			return false;
	}

	else if (y != 0 && x == 0) {
		if (Dir[y - 1][x] == HOR)
			return true;
		else
			return false;
	}

	else {
		if (Dir[y - 1][x] == HOR || Dir[y][x - 1] == HOR)
			return true;
		else
			return false;
	}
}

int ** Decoder::run(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp) {

	// Encoding variables
	int y, x;
	int x_o, x_h, x_v;
	int pred, res, ctx;
	unsigned int sym;
	int dir_;

	// First Pixel
	{
		y = 0;
		x = 0;

		Dir[y][x] = VER;

		x_v = ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

		pred = x_v;

		ctx = context(x, y);
		sym = decodemag(pCoder, &pDm[ctx]);
		res = UNMAP(sym);
		x_o = res + pred;
		X_o[y][x] = x_o;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			if (x == 0 && y == 0)
				continue;

			x_h = (x == 0) ? X_o[y - 1][x] : X_o[y][x - 1];
			x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

			if (eitherHOR(x, y)) {
				dir_ = pCoder->get_bit();
				pred = (dir_ == HOR) ? x_h : x_v;
			}
			else {
				pred = x_v;
			}

			ctx = context(x, y);
			sym = decodemag(pCoder, &pDm[ctx]);
			res = UNMAP(sym);
			x_o = res + pred;
			Dir[y][x] = dir(x_o, T, x_v, x_h);
			X_o[y][x] = x_o;
		}
	}

	printf("%d bytes\n", pCoder->get_code_bytes());

	return X_o;
}