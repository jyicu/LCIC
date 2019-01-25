// ACtest.cpp: implementation of the CACtest class.
//
//////////////////////////////////////////////////////////////////////

#include "ACtest.h"
#include "arithmetic_codec.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CACtest::CACtest()
{
	m_dprob_0=0.1;
}

CACtest::~CACtest()
{

}
void CACtest::set_prob_0(double x)
{
	m_dprob_0=x;
	if(m_dprob_0<0) m_dprob_0=0;
	if(m_dprob_0>1) m_dprob_0=1;
}
void CACtest::runtest()
{
	Adaptive_Bit_Model bitmodel;
	Arithmetic_Codec accoder;

	srand(clock());
	unsigned int bit;

	accoder.set_buffer(1000000/8+1024);
	accoder.start_encoder();

	for(int i=0;i<1000000;++i)
	{
		bit = (rand() < (RAND_MAX+1)*m_dprob_0) ? 0 : 1;
		accoder.encode(bit,bitmodel);
	}	

	unsigned int code_bytes=accoder.stop_encoder();

	double entrophy = -m_dprob_0*log(m_dprob_0)/log(2.0)+-(1-m_dprob_0)*log(1-m_dprob_0)/log(2.0);

	double bit_per_symbol = code_bytes*8/1000000; // bit/symbol

	printf("theoretical entrophy = %f\n",entrophy);
	printf("bit per symbol in AC = %f\n",entrophy);
}


