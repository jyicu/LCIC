#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"

#include "acfile\arithmetic_codec.h"
#include "global.h"


int cabac_encode(header_t *header, pixel_t *dctCoef, int bufsize, FILE *fp)
{
	int parts = header->parts;
	int size  = header->size;
	int crop  = header->crop;
	int target_rate = header->target_rate-HEADER_SIZE;

	int i, j, k, l;
	int ctx;		// context
	int bp;			// bitplane
	int sym;		// symbol
	int sign;
	int last_sign=0;	//positive

	int *sig = (int*)malloc(sizeof(int)*size);

	Arithmetic_Codec coder;
	Adaptive_Bit_Model bm[CONTEXTS];
	
	memset(sig, 0, sizeof(int)*size);

	coder.set_buffer(bufsize);
	coder.start_encoder();

	for(j=header->maxBP; j>=0; j--)
	{
		bp = 1<<j;

		for(k=0; k<parts; k++)
		{
			i = k*(size/parts);

			for(l=0; l<crop; l++)
			{
				if(i==0)
					ctx = (sig[i]<<1) + sig[i+1];
				else if(i==size-1)
					ctx = (sig[i-1]<<2) + (sig[i]<<1);
				else
					ctx = (sig[i-1]<<2) + (sig[i]<<1) + sig[i+1];

				//ctx = CTX_SIGN;
				//ctx = sig[i]+1;

				ctx = sig[i];
				ctx += ((i>0) ? sig[i-1] : 1) << 1;
				ctx += ((i>1) ? sig[i-2] : 1) << 2;
				ctx += ((i<size-1) ? sig[i+1] : 0) << 3;
				ctx += ((i<size-2) ? sig[i+2] : 0) << 4;

				sym = ABS(dctCoef[i]) & bp;
				coder.encode(sym, bm[ctx]);

				if(!sig[i] && sym) {		// first significant bit
					sig[i] = 1;
					sign = SIGN(dctCoef[i]);
					coder.encode(ABS(sign-last_sign),bm[CTX_SIGN]);
					last_sign = sign;
				}

				if( coder.get_code_bytes()>target_rate )
				{
					header->nSymbol = j*size+i;
					free(sig);
					return coder.write_to_file(fp);
				}

				i++;
			}
		}
	}

	header->nSymbol = i-1;
	free(sig);

	return coder.write_to_file(fp);
}

void cabac_decode(header_t *header, FILE *fp, pixel_t *dctCoef)
{
	int parts = header->parts;
	int size  = header->size;
	int crop  = header->crop;
	int target_rate = header->target_rate-10;
	int minBP   = header->nSymbol / size;
	int lastIdx = header->nSymbol % size;

	int i, j, k, l;
	int ctx;		// context
	int sym;		// symbol
	int sign;
	int last_sign=0;	//positive

	int *sig = (int*)malloc(sizeof(int)*size);

	Arithmetic_Codec coder;
	Adaptive_Bit_Model bm[CONTEXTS];
	
	memset(sig, 0, sizeof(int)*size);
	memset(dctCoef, 0, sizeof(pixel_t)*size);

	coder.set_buffer(BUFSIZE);
	coder.read_from_file(fp);

	for(j=header->maxBP; j>=minBP; j--)
	{
		for(k=0; k<parts; k++)
		{
			i = k*(size/parts);

			for(l=0; l<crop; l++)
			{
				if(i==0)
					ctx = (sig[i]<<1) + sig[i+1];
				else if(i==size-1)
					ctx = (sig[i-1]<<2) + (sig[i]<<1);
				else
					ctx = (sig[i-1]<<2) + (sig[i]<<1) + sig[i+1];

				//ctx = CTX_SIGN;
				//ctx = sig[i]+1;

				ctx = sig[i];
				ctx += ((i>0) ? sig[i-1] : 1) << 1;
				ctx += ((i>1) ? sig[i-2] : 1) << 2;
				ctx += ((i<size-1) ? sig[i+1] : 0) << 3;
				ctx += ((i<size-2) ? sig[i+2] : 0) << 4;

				sym = coder.decode(bm[ctx]);

				if(!sig[i] && sym) {		// first significant bit
					sig[i] = 1;
					sign = coder.decode(bm[CTX_SIGN]) ? (1-last_sign) : last_sign;
					dctCoef[i] = sign ? sym<<j : -(sym<<j);
					last_sign = sign;
				}
				else
				{
					sign = SIGN(dctCoef[i]);
					dctCoef[i] += sign ? sym<<j : -(sym<<j);
				}

				if( j==minBP && i==lastIdx )
				{
					// rounding
					for(++i; i<size; i++)
						if( sig[i] ) dctCoef[i] += SIGN(dctCoef[i]) ? 1<<j : -(1<<j);

					if( j>0 )
					{
						j--;
						for(i=0; i<size; i++)
							if( sig[i] ) dctCoef[i] += SIGN(dctCoef[i]) ? 1<<j : -(1<<j);
					}

					coder.stop_decoder();
					free(sig);
					return;
				}

				i++;
			}
		}
	}

	coder.stop_decoder();
	free(sig);
}
