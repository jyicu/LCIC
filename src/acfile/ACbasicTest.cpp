// ACbasicTest.cpp: implementation of the CACbasicTest class.
//
//////////////////////////////////////////////////////////////////////

#include "ACbasicTest.h"
#include "arithmetic_codec.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CACbasicTest::CACbasicTest()
{

}

CACbasicTest::~CACbasicTest()
{

}

void CACbasicTest::testenc()
{
	FILE *fp=fopen("test.cmp","wb");

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[2];

	int num_symbol=3;
	int sym[10]={0,1,2,0,1,2,1,1,1,0};
	
	dm[0].set_alphabet(num_symbol);
	dm[1].set_alphabet(num_symbol);

	coder.set_buffer(1024);
	coder.start_encoder();

	int context=0;
	for(int i=0;i<10;++i)
	{
		coder.encode(sym[i],dm[context]);
		context = sym[i]%2; // context=0 if previous symbol is even, context=1 if odd.
	}
	coder.write_to_file(fp);
	fclose(fp);
}

void CACbasicTest::testdec()
{
	FILE *fp=fopen("test.cmp","rb");

	Arithmetic_Codec coder;
	Adaptive_Data_Model dm[2];
	
	int num_symbol=3;
	
	dm[0].set_alphabet(num_symbol);
	dm[1].set_alphabet(num_symbol);

	coder.set_buffer(1024);
	coder.read_from_file(fp);
	unsigned int decoded,context=0;
	for(int i=0;i<10;++i)
	{
		decoded=coder.decode(dm[context]);
		printf("sym = %d\n",decoded);
		context = decoded%2;
	}
	coder.stop_decoder();
	
}