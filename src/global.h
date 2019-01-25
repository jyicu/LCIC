#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//#define PRINT_TIME
#define RATIO		2

#define	BUFSIZE			(1024*100)		// CABAC buffer, 100kBytes
#define	HEADER_SIZE		(1+2)				// in bytes

typedef struct {
	int parts;
	int size;
	int nSymbol;
	int maxBP;
	int qp;
	int crop;
	int target_rate;
} header_t;

typedef short pixel_t;
#define TO_PIXEL(x)	((x)>32767 ? 32767 : ((x)<-32768 ? -32768 : (x)))

#define CONTEXTS		(32+1)
#define CTX_SIGN		(CONTEXTS-1)

#define ABS(x)			((x)<0 ? (-(x)) : (x))
#define SIGN(x)			((x)<0 ? (0) : (1))
#define MAX(x,y)		((x)<(y) ? (y) : (x))


void init_dct(int dctSize, int crop);
void close_dct();
void dct(pixel_t *in, pixel_t *out);
void idct(pixel_t *in, pixel_t *out);

int cabac_encode(header_t *header, pixel_t *dctCoef, int bufsize, FILE *fp);
void cabac_decode(header_t *header, FILE *fp, pixel_t *dctCoef);


#endif	//ndef __GLOBAL_H__
