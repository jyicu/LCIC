#pragma once
#pragma warning(disable: 4996)

#include "acfile/arithmetic_codec.h"

#define MAX(x,y)	((x)>(y) ? (x) : (y))
#define MIN(x,y)	((x)<(y) ? (x) : (y))
#define MAX4(X)		MAX(MAX((X)[0],(X)[1]),MAX((X)[2],(X)[3]))
#define MIN4(X)		MIN(MIN((X)[0],(X)[1]),MIN((X)[2],(X)[3]))
#define ABS(x)		((x)>0 ? (x) : -(x))
#define CLIP(x,y,z)	MAX((y), MIN((x), (z)))
#define CLIP255(x)	MAX(0, MIN((x), 255))
#define ROUND(x)	((int)((x)+0.5))
#define UINT8(x)	CLIP255(ROUND(x))
//#define MAP(res)    res >= 0 ? 2 * res : -2 * res - 1;
//#define UNMAP(sym)  (sym % 2 ==0) ? sym/2 : (sym+1)/2 * (-1);

#define NUM_CTX 6*3

template <typename T>
T **alloc2D(int height, int width);
template <typename T>
void free2D(T **p);


class Encoder {
public:
	Encoder(int ** X_o, int ** X_e, int T, int K, int* symmax, int height, int width);
	~Encoder();

	int run(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp);

private:
	int height, width;
	int T, K;
	int ** X_e;
	int ** X_o;
	int ** sigma;
	bool ** Dir;
	int * q;
	int* symMax;

	void init();
	void set_local_activity();
	void context_modeling();
	int context(int x, int y);
	void initCoder(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm);
	void encodeMag(unsigned int sym, int ctx, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm);
	bool eitherHOR(int x, int y);
};


class Decoder {
public:
	Decoder(int **X_e, int T, int K, int* symmax, int height, int width);
	~Decoder();

	int** run(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp);

private:
	int height, width;
	int T, K;
	int ** X_e;
	int ** X_o;
	int ** sigma;
	bool ** Dir;
	int * q;
	int* symMax;

	void init();
	void set_local_activity();
	void context_modeling();
	int context(int x, int y);
	void initCoder(Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm, FILE *fp);
	unsigned int decodemag(int ctx, Arithmetic_Codec *pCoder, Adaptive_Data_Model *pDm);
	bool eitherHOR(int x, int y);
};
