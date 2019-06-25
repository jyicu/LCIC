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

int MAP(int X, int pred) {
	int sym;
	bool inv = 0;
	if (pred < 0) {
		X = -X - 1;
		pred = -pred - 1;
		inv = 1;
	}
	if (inv) {
		if (X > pred)
			sym = 2 * (X - pred) - 1;
		else if (X < 2 * pred - 255)
			sym = 255 - X;
		else
			sym = 2 * (pred - X);
	}
	else {
		if (X >= pred)
			sym = 2 * (X - pred);
		else if (X < 2 * pred - 255)
			sym = 255 - X;
		else
			sym = 2 * (pred - X) - 1;
	}
	return sym;
}

int UNMAP(int sym, int pred) {
	int x;
	bool inv = 0;
	if (pred < 0) {
		pred = -pred - 1;
		inv = 1;
	}
		
	if (inv) {
		if (sym > 510 - 2 * pred)
			x = 255 - sym;
		else if (sym % 2 == 0)
			x = pred - sym / 2;
		else
			x = pred + (sym + 1) / 2;
		x = -x - 1;
	}
	else {
		if (sym > 510 - 2 * pred)
			x = 255 - sym;
		else if (sym % 2 == 0)
			x = pred + sym / 2;
		else
			x = pred - (sym + 1) / 2;
	}

	return x;
}

Encoder::Encoder(int ** _X_o, int ** _X_e, int _T, int _K, int* symMax_, int _height, int _width) {

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

	bool print = false;

	std::copy(sigma[0], sigma[0] + N, temp);
	std::sort(temp, temp + N);

	for (int i = 0; i < K - 1; i++) {
		q[i] = temp[(i + 1) * interval];
	}

	int max = temp[N - 1];

	int *ctx_cnt = new int[max];

	for (int i = 0; i < max + 1; i++) {

		ctx_cnt[i] = 0;

		for (int j = 0; j < N; j++) {
			if (temp[j] == i)
				ctx_cnt[i]++;
		}
	}

	int sum;
	int right_sum, left_sum;
	int j;
	int start_idx;

	for (int i = 0; i < K - 1; i++) {

		if (i == 0)
			start_idx = 0;
		else
			start_idx = q[i - 1] + 1;

		sum = 0;

		for (j = start_idx; j < max; j++) {

			sum += ctx_cnt[j];

			if (sum > interval)
				break;
		}

		right_sum = sum;
		left_sum = sum - ctx_cnt[j];

		if (j == 0) {
			q[i] = 0;
		}
		else {
			if (ABS(right_sum - interval) > ABS(left_sum - interval))
				q[i] = j - 1;
			else
				q[i] = j;
		}
	}

	if (print) {
		printf("Interval %d\n", interval);

		for (int i = 0; i < K - 1; i++) {
			printf("q : %d\n", i, q[i]);
		}

		for (int i = 0; i < max + 1; i++) {
			printf("ctx %d : %d\n", i, ctx_cnt[i]);
		}

		for (int i = 0; i < K - 1; i++) {
			printf("%d ", q[i]);
		}

		printf("\n");

		for (int i = 0; i < K - 1; i++) {

			int cnt = 0;

			if (i == 0) {
				for (int j = 0; j < q[i] + 1; j++) {
					cnt += ctx_cnt[j];
				}

				printf("Num q[%d] : %d\n", i, cnt);
			}
			else {
				for (int j = q[i - 1] + 1; j < q[i] + 1; j++) {
					cnt += ctx_cnt[j];
				}

				printf("Num q[%d] : %d\n", i, cnt);
			}
		}
	}

	delete[] temp;

}

int Encoder::context(int x, int y) {

	for (int i = 0; i < K - 1; i++) {
		if (sigma[y][x] <= q[i]) return i;
	}
	return K - 1;

}

void Encoder::encodeMag(unsigned int sym, int ctx, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {

	if (sym >= symMax[ctx]) {
		pCoder->encode(symMax[ctx], *pDm);
		sym -= symMax[ctx];
		pCoder->put_bit(sym & 1);
		encodeMag(sym >> 1, ctx, pCoder, pDm);
	}
	
	else 
		pCoder->encode(sym, *pDm);
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

	for (int i = 0; i < 3*K; i++) {
		pDm[i].set_alphabet(symMax[i] + 1);
		pDm[i].set_distribution(0.7);
	}

	pCoder->set_buffer(height*width);
	pCoder->start_encoder();
}

int Encoder::run(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp) {

	// Counter Initialize
	int Dir_counter[2] = { 0 };
	int sym_counter[NUM_CTX][512] = { 0 };
	int context_counter[NUM_CTX] = { 0 };
	int x_h_counter = 0;
	int numPix = 0;

	// Encoding variables
	int y, x;
	int x_o, x_h, x_v;
	float x_vf;
	int pred, res, ctx;
	int ctx_res; // x_v.0 = 0, x_v.5 = 1, x_h = 2 
	unsigned int sym;

	// Data model for dir encoding
	Adaptive_Data_Model dir_dm;
	dir_dm.set_alphabet(2);
	dir_dm.set_distribution(0.1);

	// First Pixel
	{
		y = 0;
		x = 0;

		x_o = X_o[y][x];
		x_vf = 0.5*(X_e[y][x] + X_e[y + 1][x]);
		x_v = ROUND(x_vf);
		ctx_res = ((x_vf - x_v) == 0) ? 0 : 1;

		// Set First pixel direction as vertical
		Dir[y][x] = VER;
		pred = x_v;

		// Encode
		sym = MAP(x_o,pred);
		ctx = 3 * context(x, y) + ctx_res;

		encodeMag(sym, ctx, pCoder, &pDm[ctx]);

		// Counter
		Dir_counter[Dir[y][x]]++;
		sym_counter[ctx][sym]++;
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
			
			if (y == height - 1) {
				x_v = X_e[y][x];
				ctx_res = 0;
			}
			else {
				x_vf = 0.5*(X_e[y][x] + X_e[y + 1][x]);
				x_v = ROUND(x_vf);
				ctx_res = ((x_vf - x_v) == 0) ? 0 : 1;
			}

			//===========================//
			//     Encode Direction      //
			//===========================//
			Dir[y][x] = dir(x_o, T, x_v, x_h);

			if (eitherHOR(x, y)) {
				pCoder->encode(Dir[y][x], dir_dm);
				if (Dir[y][x] == HOR) {
					pred = x_h;
					ctx_res = 2;
					x_h_counter++;
				}
				else {
					pred = x_v;
				}
					
			}
			else
				pred = x_v;

			//===========================//
			//      Encode Symbol        //
			//===========================//
			sym = MAP(x_o,pred);
			ctx = 3*context(x, y) + ctx_res;

			encodeMag(sym, ctx, pCoder, &pDm[ctx]);

			Dir_counter[Dir[y][x]]++;
			sym_counter[ctx][sym]++;
			context_counter[ctx]++;

			numPix++;
		}
	}

	//coder
	float proportion = float(x_h_counter) / numPix;
	//printf("freq of selecting H prediction : %f\n", proportion);

	return 0;
}

Decoder::Decoder(int **X_e_, int T_, int K_, int* symmax_, int height_, int width_) {

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

	bool print = false;

	std::copy(sigma[0], sigma[0] + N, temp);
	std::sort(temp, temp + N);

	for (int i = 0; i < K - 1; i++) {
		q[i] = temp[(i + 1) * interval];
	}

	int max = temp[N - 1];

	int *ctx_cnt = new int[max];

	for (int i = 0; i < max + 1; i++) {

		ctx_cnt[i] = 0;

		for (int j = 0; j < N; j++) {
			if (temp[j] == i)
				ctx_cnt[i]++;
		}
	}

	int sum;
	int right_sum, left_sum;
	int j;
	int start_idx;

	for (int i = 0; i < K - 1; i++) {

		if (i == 0)
			start_idx = 0;
		else
			start_idx = q[i - 1] + 1;

		sum = 0;

		for (j = start_idx; j < max; j++) {

			sum += ctx_cnt[j];

			if (sum > interval)
				break;
		}

		right_sum = sum;
		left_sum = sum - ctx_cnt[j];

		if (j == 0) {
			q[i] = 0;
		}
		else {
			if (ABS(right_sum - interval) > ABS(left_sum - interval))
				q[i] = j - 1;
			else
				q[i] = j;
		}
	}

	if (print) {
		printf("Interval %d\n", interval);

		for (int i = 0; i < K - 1; i++) {
			printf("q : %d\n", i, q[i]);
		}

		for (int i = 0; i < max + 1; i++) {
			printf("ctx %d : %d\n", i, ctx_cnt[i]);
		}

		for (int i = 0; i < K - 1; i++) {
			printf("%d ", q[i]);
		}

		printf("\n");

		for (int i = 0; i < K - 1; i++) {

			int cnt = 0;

			if (i == 0) {
				for (int j = 0; j < q[i] + 1; j++) {
					cnt += ctx_cnt[j];
				}

				printf("Num q[%d] : %d\n", i, cnt);
			}
			else {
				for (int j = q[i - 1] + 1; j < q[i] + 1; j++) {
					cnt += ctx_cnt[j];
				}

				printf("Num q[%d] : %d\n", i, cnt);
			}
		}
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

	for (int i = 0; i < 3 * K; i++) {
		pDm[i].set_alphabet(symMax[i] + 1);
		pDm[i].set_distribution(0.7);
	}

	pCoder->set_buffer(height*width);
	pCoder->read_from_file(fp);
}

unsigned int Decoder::decodemag(int ctx, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm) {

	unsigned int sym;

	sym = pCoder->decode(*pDm);

	if (sym == symMax[ctx]) {
		sym += pCoder->get_bit();
		sym += (decodemag(ctx, pCoder, pDm) << 1);
	}

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
	float x_vf;
	int pred, res, ctx;
	int ctx_res;
	unsigned int sym;
	int dir_;

	// Data model for dir encoding
	Adaptive_Data_Model dir_dm;
	dir_dm.set_alphabet(2);
	dir_dm.set_distribution(0.1);

	// First Pixel
	{
		y = 0;
		x = 0;

		Dir[y][x] = VER;

		x_vf = 0.5*(X_e[y][x] + X_e[y + 1][x]);
		x_v = ROUND(x_vf);
		ctx_res = ((x_vf - x_v) == 0) ? 0 : 1;

		pred = x_v;

		ctx = 3 * context(x, y) + ctx_res;
		sym = decodemag(ctx, pCoder, &pDm[ctx]);
		x_o = UNMAP(sym,pred);
		X_o[y][x] = x_o;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			if (x == 0 && y == 0)
				continue;

			x_h = (x == 0) ? X_o[y - 1][x] : X_o[y][x - 1];
			if (y == height - 1) {
				x_v = X_e[y][x];
				ctx_res = 0;
			}
			else {
				x_vf = 0.5*(X_e[y][x] + X_e[y + 1][x]);
				x_v = ROUND(x_vf);
				ctx_res = ((x_vf - x_v) == 0) ? 0 : 1;
			}

			if (eitherHOR(x, y)) {
				dir_ = pCoder->decode(dir_dm);
				if (dir_ == HOR) {
					pred = x_h;
					ctx_res = 2;
				}
				else
					pred = x_v;
			}
			else {
				pred = x_v;
			}

			ctx = 3 * context(x, y) + ctx_res;
			sym = decodemag(ctx, pCoder, &pDm[ctx]);
			x_o = UNMAP(sym,pred);
			Dir[y][x] = dir(x_o, T, x_v, x_h);
			X_o[y][x] = x_o;
		}
	}

	printf("%d bytes\n", pCoder->get_code_bytes());

	return X_o;
}