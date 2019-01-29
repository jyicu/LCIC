#include <algorithm>
#include <string.h>
#include <assert.h>

#include "encoder.h"
#include "preprocess.h"
#include "BMP.h"

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
	return ((ABS(x_o - x_v)) > (ABS(x_o - x_h) + T) ? (1) : (0));
}

Hierarchical_coder::Hierarchical_coder(char filename[]) {
	// Read in image file to RGB channels
	int **R, **G, **B;

	bmpRead(filename, &R, &G, &B, &height, &width);

	assert(height % 2 == 0);
	assert(width % 2 == 0);

	// Color transform RGB into YUV image
	int **U, **V;

	RGB2YUV(&R, &G, &B, &Y, &U, &V, &height, &width);

	// Decompose U,V channels into odd1, odd2, even2 images
	image_decomposition(&U, &U_o1, &U_o2, &U_e1, &U_e2, &height, &width);
	image_decomposition(&V, &V_o1, &V_o2, &V_e1, &V_e2, &height, &width);

	// free memory
	free(R);
	free(G);
	free(B);
	free(U);
	free(V);
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
	return 0;
}




Encoder::Encoder(int ** I, int _T, int _K, int _height, int _width) {
	split_image(&I, &X_o, &X_e, &_height, &_width);
	T = _T;
	K = _K;
	height = _height/2;
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
	//split image()
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
		sigma[height-1][x] = sigma[height - 2][x];
	}
}

void Encoder::context_modeling() {
	int N = height*width;
	int * temp = new int[N];
	int interval = N / 6;
	
	std::copy(sigma[0], sigma[0] + N, temp);
	std::sort(temp, temp + N);
	
	for (int i = 0; i < K-1; i++) {
		q[i] = temp[(i+1)*interval];
	}
	delete[] temp;
}

int Encoder::context(int x, int y) {
	for (int i = 0; i < K - 1; i++) {
		if (sigma[y][x] <= q[i]) return i;
	}
	return K - 1;
}


int Encoder::run() {
	int y, x;
	int numPix = 0;
	int pred;

	int Dir_counter[2] = { 0 };
	int sym_counter[512] = { 0 };
	int context_counter[6] = { 0 };
	int x_h_counter = 0;

	y = 0; x = 0;
	pred = ROUND(0.5*(X_e[0][0] + X_e[1][0]));
	Dir[0][0] = 0;
	{
		int x_o = X_o[0][0];
		int res = x_o - pred;
		int sym = res >= 0 ? 2 * res : -2 * res - 1;
		int ctx = context(x, y);

		Dir_counter[Dir[y][x]]++;
		sym_counter[sym]++;
		context_counter[ctx]++;

		// TODO : Arithmetic coding for sym

		numPix++;
	}
	for (x = 1; x < width; x++) {
		int x_o = X_o[0][x];
		int x_h = X_o[0][x - 1];
		int x_v = ROUND(0.5*(X_e[0][x] + X_e[1][x]));
		if (Dir[y][x - 1]) {
			Dir[y][x] = dir(x_o, T, x_v, x_h);
			// TODO : Encode Dir[y][x]
			if (Dir[y][x]) {
				pred = x_h;
				x_h_counter++;
			}
			else
				pred = x_v;
		}
		else {
			pred = x_v;
			Dir[y][x] = dir(x_o, T, x_v, x_h);
		}

		//////// TODO : wrapping
		int res = x_o - pred;
		int sym = res >= 0 ? 2 * res : -2 * res - 1;
		int ctx = context(x, y);

		Dir_counter[Dir[y][x]]++;
		sym_counter[sym]++;
		context_counter[ctx]++;

		// TODO : Arithmetic coding for sym

		numPix++;

	}

	// middle rows
	for (y = 1; y < height; y++) {
		for (x = 0; x < width; x++) {
			int x_o = X_o[y][x];
			int x_h = (x == 0) ? X_o[y - 1][x] : X_o[y][x - 1];
			int x_v = (y == height - 1) ? X_e[y][x] : ROUND(0.5*(X_e[y][x] + X_e[y + 1][x]));
			if (Dir[y-1][x] || Dir[y][x-1]) {
				Dir[y][x] = dir(x_o, T, x_v, x_h);
				// TODO : Encode Dir[y][x]
				if (Dir[y][x]) {
					pred = x_h;
					x_h_counter++;
				}
				else
					pred = x_v;	
			}
			else {
				pred = x_v;
				Dir[y][x] = dir(x_o, T, x_v, x_h);
			}

			int res = x_o - pred;
			//printf("Dir : %d, res : %d \n", Dir[y][x], res);
			int sym = res >= 0 ? 2 * res : -2 * res - 1;
			int ctx = context(x, y);

			Dir_counter[Dir[y][x]]++;
			sym_counter[sym]++;
			context_counter[ctx]++;

			// TODO : Arithmetic coding for sym

			numPix++;
		}
	}

	//coder
	float proportion = float(x_h_counter) / numPix;
	printf("freq of selecting H prediction : %f\n", proportion);
	int bytes = 0;

	return bytes;
}

