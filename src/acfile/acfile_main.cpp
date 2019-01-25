
#include "ACFileCoder.h"
#include "ACtest.h"
#include "ACbasicTest.h"
#include <stdio.h>
#include <math.h>


int main(int argc,char** argv)
{
	if ( (!(argc == 4 || argc==3)) || (argv[1][0] != '-') || 
		((argv[1][1] != 'c') && (argv[1][1] != 'd')  && (argv[1][1] != 't'))) {
		printf("\n\t Compression :   acfile -c data_file compressed_file");
		printf("\n\t Decompression : acfile -d compressed_file new_file");
		printf("\n\t AC entrophy test : acfile -t <probability of symbol 0>\n");

		
		return 0;
	}
	
	CACFileCoder coder;
		
	if (argv[1][1] == 'c')
	{		
		coder.encode_file_sim(argv[2],argv[3]);
	}
	else if (argv[1][1] == 'd')
	{		
		coder.decode_file_sim(argv[2],argv[3]);
	}
	else
	{
		CACbasicTest basic;
		basic.testenc();
		basic.testdec();

		CACtest test;
		test.set_prob_0(atof(argv[2]));
		test.runtest();
		return 0;
	}
	return 0;
}
