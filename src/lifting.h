#include "math.h"

#ifndef __LIFING_H__
#define __LIFING_H__

//typedef int jpc_fix_t;
typedef short jpc_fix_t;

#define QMFB_SPLITBUFSIZE		(4096<<1)
#define QMFB_JOINBUFSIZE		(QMFB_SPLITBUFSIZE)

int jpc_ft_analyze(jpc_fix_t *a, int numcols);
int jpc_ft_synthesize(jpc_fix_t *a, int numcols);

#endif	//ndef __LIFING_H__

