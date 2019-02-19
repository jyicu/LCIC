#pragma warning(disable: 4996)
#include <algorithm>
#include <string.h>
#include <assert.h>
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

Hierarchical_coder::Hierarchical_coder(char filename[], int _T, int _K, int _symMax) {

	preprocess(filename, &Y, &U_o1, &U_o2, &U_e1, &U_e2, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	// Encoding variables
	T = _T;
	K = _K;
	symMax = _symMax;
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

	// open output file
	char codefile[] = "code.bin";

	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	/**TO DO **/
	// 1. Encode Y, U_e2, V_e2

	// 2. Encode U_o2, V_o2
	Encoder Uo2(U_o2, U_e2, T, K, symMax, width / 2, height / 2);
	Encoder Vo2(V_o2, V_e2, T, K, symMax, width / 2, height / 2);
	Uo2.run(fp);
	Vo2.run(fp);

	// 3. Encode U_o1, V_o1
	Encoder Uo1(U_o1, U_e1, T, K, symMax, height / 2, width);
	Encoder Vo1(V_o1, V_e1, T, K, symMax, height / 2, width);
	Uo1.run(fp);
	Vo1.run(fp);

	fclose(fp);

	return 0;
}

Hierarchical_decoder::Hierarchical_decoder(int _T, int _K, int _symMax, int _height, int _width) {

	// Encoding variables
	T = _T;
	K = _K;
	symMax = _symMax;
	height = _height;
	width = _width;
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

unsigned char* Hierarchical_decoder::readBinFile(char filename[]) {

	if (!(fp = fopen(filename, "rb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	std::vector<unsigned char> data_vec;
	int ch;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fputs("error", stderr);
		exit(1);
	}

	while ((ch = fgetc(fp)) != EOF) {
		data_vec.push_back(ch);
	}

	unsigned char compressed_data[ARRAY_MAX];

	std::copy(data_vec.begin(), data_vec.end(), compressed_data);

	return compressed_data;
}

int** Hierarchical_decoder::decode_jpeg2000(unsigned char* compressed_data) {

	//**TO DO**//
	// Read Data
	// Cut compressed data

	return alloc2D<int>(height, width);
}

int Hierarchical_decoder::run(char filename[]) {

	Arithmetic_Codec coder;

	// Read in compressed data
	unsigned char* compressed_data = readBinFile(filename);

	//** TO DO **//
	// 1. Decode Y, U_e2, V_e2
	char infile[] = "./Kodak/kodim05.bmp";
	int **temp1, **temp2, **temp3, **temp4, **temp5, **temp6;

	preprocess(infile, &Y, &temp1, &temp2, &temp3, &U_e2, &temp4, &temp5, &temp6, &V_e2, &height, &width);
	
	//Y    = decode_jpeg2000(compressed_data);
	//U_e2 = decode_jpeg2000(compressed_data);
	//V_e2 = decode_jpeg2000(compressed_data);

	
	// 2. Decode U_o2, V_o2
	Decoder Uo2(U_e2, T, K, symMax, width / 2, height / 2);
	Decoder Vo2(V_e2, T, K, symMax, width / 2, height / 2);
	U_o2 = Uo2.run(compressed_data);
	V_o2 = Vo2.run(compressed_data);

	// 3. Build U_o1, V_o1 from (U_o2, U_e2) and (V_o2, V_e2)
	int **U_e1_R, **V_e1_R;
	int half_height = height / 2;

	concat_image(&U_o2, &U_e2, &U_e1_R, &width, &half_height);
	concat_image(&V_o2, &V_e2, &V_e1_R, &width, &half_height);

	rotate_image(&U_e1_R, &U_e1, 0, &width, &half_height);
	rotate_image(&V_e1_R, &V_e1, 0, &width, &half_height);

	// 4. Decode U_o1, V_o1
	Decoder Uo1(U_e1, T, K, symMax, height / 2, width);
	Decoder Vo1(V_e1, T, K, symMax, height / 2, width);
	U_o1 = Uo1.run(compressed_data);
	V_o1 = Uo1.run(compressed_data);

	postprocess("Decoded_Result.bmp", &Y, &U_o1, &U_o2, &U_e2, &V_o1, &V_o2, &V_e2, &height, &width);

	return 0;
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
	//free2D(X_e);
	//free2D(X_o);
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

	int size = width * height;
	pCoder->set_buffer(size);
	pCoder->start_encoder();

}

int Encoder::run(FILE *fp) {

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];

	initCoder(&coder, dm);

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

		encodeMag(sym, &coder, &dm[ctx]);

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
			x_h = (x == 0)          ? X_o[y - 1][x] : X_o[y][x - 1];
			x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

			//===========================//
			//     Encode Direction      //
			//===========================//
			Dir[y][x] = dir(x_o, T, x_v, x_h);

			if (eitherHOR(x, y)) {
				encodeDir(Dir[y][x], &coder);
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

			//std::cout << "(y,x,sym) : " << y << ", " << x << ", " << sym << std::endl;

			encodeMag(sym, &coder, &dm[ctx]);

			Dir_counter[Dir[y][x]]++;
			sym_counter[sym]++;
			context_counter[ctx]++;

			numPix++;
		}
	}

	//coder
	float proportion = float(x_h_counter) / numPix;
	printf("freq of selecting H prediction : %f\n", proportion);
	
	int bytes = coder.write_to_file(fp);
	printf("%d bytes. %f bpp\n", bytes, 8.0*bytes / numPix);

	return bytes;
}

int Encoder::run_test() {

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];

	initCoder(&coder, dm);

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

		encodeMag(sym, &coder, &dm[ctx]);

		// Counter
		Dir_counter[Dir[y][x]]++;
		sym_counter[sym]++;
		context_counter[ctx]++;

		numPix++;

		std::cout << x_o << std::ends;
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
				encodeDir(Dir[y][x], &coder);
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

			//std::cout << "(y,x,sym) : " << y << ", " << x << ", " << sym << std::endl;

			encodeMag(sym, &coder, &dm[ctx]);

			Dir_counter[Dir[y][x]]++;
			sym_counter[sym]++;
			context_counter[ctx]++;

			numPix++;

			std::cout << x_o << std::ends;
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	//coder
	float proportion = float(x_h_counter) / numPix;
	printf("freq of selecting H prediction : %f\n", proportion);

	// open output file
	FILE *fp;
	char codefile[] = "code_test.bin";

	if (!(fp = fopen(codefile, "wb"))) {
		fprintf(stderr, "Code file open error.\n");
		exit(-1);
	}

	int bytes = coder.write_to_file(fp);
	printf("%d bytes. %f bpp\n", bytes, 8.0*bytes / numPix);

	fclose(fp);

	return bytes;
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

	int size = width * height;
	pCoder->set_buffer(size);
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

int ** Decoder::run(unsigned char* compressed_data) {

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];

	initCoder(&coder, dm, NULL);

	// Encoding variables
	int y, x;
	int x_o, x_h, x_v;
	int pred, res, ctx;
	unsigned int sym;
	int dir;

	// First Pixel
	{
		y = 0;
		x = 0;

		ctx = context(x, y);

		sym = decodemag(&coder, &dm[ctx]);

		res = UNMAP(sym);

		Dir[y][x] = VER;

		x_v = ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

		pred = x_v;

		x_o = res + pred;

		X_o[y][x] = x_o;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			if (x == 0 && y == 0)
				continue;

			x_h = (x == 0)          ? X_o[y - 1][x] : X_o[y][x - 1];
			x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

			ctx = context(x, y);

			sym = decodemag(&coder, &dm[ctx]);

			res = UNMAP(sym);

			if (eitherHOR(x, y)) {
				dir = coder.get_bit();
				pred = (dir == HOR) ? x_h : x_v;
				Dir[y][x] = dir;
			}
			else {
				pred = x_v;
				Dir[y][x] = VER;
			}

			x_o = res + pred;

			X_o[y][x] = x_o;
		}
	}

	//** TO DO **//
	// Cut compressed data
	unsigned int code_bytes = coder.get_code_bytes();
	compressed_data = compressed_data + code_bytes;

	return X_o;
}

int ** Decoder::run_test() {

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[NUM_CTX];

	FILE *fp;

	if ((fp = fopen("code_test.bin", "rb")) == NULL) {
		fputs("error", stderr);
		exit(1);
	}

	initCoder(&coder, dm, fp);

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

		ctx = context(x, y);

		sym = decodemag(&coder, &dm[ctx]);

		res = UNMAP(sym);

		Dir[y][x] = VER;

		x_v = ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

		pred = x_v;

		x_o = res + pred;

		X_o[y][x] = x_o;

		std::cout << x_o << std::ends;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			if (x == 0 && y == 0)
				continue;

			x_h = (x == 0) ? X_o[y - 1][x] : X_o[y][x - 1];
			x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));

			ctx = context(x, y);

			sym = decodemag(&coder, &dm[ctx]);

			res = UNMAP(sym);

			if (eitherHOR(x, y)) {
				dir_ = coder.get_bit();
				pred = (dir_ == HOR) ? x_h : x_v;
			}
			else {
				pred = x_v;
			}

			x_o = res + pred;

			Dir[y][x] = dir(x_o, T, x_v, x_h);

			X_o[y][x] = x_o;

			std::cout << x_o << std::ends;
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	return X_o;
}