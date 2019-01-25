#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"


#define SCALE		(4)


typedef short pixel_t;
#define FLOAT2PIXEL(x)	((x)>32767 ? 32767 : ((x)<-32768 ? -32768 : ((pixel_t)(x+0.5))))


float **T;
int dctSize;
int crop;

void init_dct(int size, int crp)
{
	const float PI = 3.141592f;
	int i, j;
	float *pp;
	float a,b;

	T  = (float**)malloc(sizeof(float**)*crp);
	pp = (float*)malloc(sizeof(float)*crp*size);

	for(i=0; i<crp; i++)
		T[i] = &pp[size*i];

	a = (float)(1.0/sqrt((float)size));
	b = (float)(sqrt(2.0f)*a);

	for(i=0; i<crp; i++)
	{
		T[i][0] = a;

		for(j=1; j<size; j++)
			T[i][j] = (float)(b*cos(PI/size*(i+0.5)*j));
	}

	dctSize = size;
	crop    = crp;
}

void close_dct()
{
	free(T[0]);
	free(T);
}

void dct(pixel_t *in, pixel_t *out)
{
	int i, j;

	for(i=0; i<crop; i++)
	{
		float sum=0;

		for(j=0; j<dctSize; j++)
			sum += T[i][j] * in[j];

		out[i] = FLOAT2PIXEL(sum/SCALE);
	}

	for(i=crop; i<dctSize; i++)
		out[i] = 0;
}

void idct(pixel_t *in, pixel_t *out)
{
	int i, j;

	for(i=0; i<dctSize; i++)
	{
		float sum=0;

		for(j=0; j<crop; j++)
			sum += T[j][i] * in[j];

		out[i] = FLOAT2PIXEL(sum*SCALE);
	}
}

