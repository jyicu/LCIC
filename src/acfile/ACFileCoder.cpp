// ACFileCoder.cpp: implementation of the CACFileCoder class.
//
//////////////////////////////////////////////////////////////////////

#include "ACFileCoder.h"
#include "arithmetic_codec.h"
#include <stdio.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CACFileCoder::CACFileCoder() 
{
	
}

CACFileCoder::~CACFileCoder()
{
	
}
int CACFileCoder::encode_file_sim(char *datfile,char *cmpfile)
{
	FILE * data_file = fopen(datfile,"rb");
	FILE * code_file = fopen(cmpfile,"wb");
	
	if(data_file==NULL) {fprintf(stderr,"cannot open data file\n");return 0;}
	if(code_file==NULL) {fprintf(stderr,"cannot open compressed file\n");fclose(data_file);return 0;}
	
	
	unsigned int bytes = 0;       
	fseek(data_file,0,SEEK_END);  // go to end of file
	bytes = ftell(data_file);   // original file size

	rewind(data_file);   

	fwrite(&bytes,1,sizeof(bytes),code_file); // write original size
	
	const int NumModels = 16;                                                         // set data models
	Adaptive_Data_Model dm[NumModels];
	for (unsigned int m = 0; m < NumModels; m++) dm[m].set_alphabet(256);

	//////////////////////////////////////////////////////////////////////////
	// FILL THIS PART - ENCODING CORE
	//////////////////////////////////////////////////////////////////////////


	fflush(code_file);
	
	unsigned data_bytes = ftell(data_file), code_bytes = ftell(code_file);
	printf("data_file size = %d bytes\nCompressed file size = %d bytes\n",
		data_bytes,code_bytes);
	printf("%f\n", (double)code_bytes/data_bytes);
	fclose(data_file);
	fclose(code_file);	
	
	return 0;
}

int CACFileCoder::decode_file_sim(char *cmpfile,char *datfile)
{
	// open files
	FILE * data_file = fopen(datfile,"wb");
	FILE * code_file = fopen(cmpfile,"rb");
	
	if(data_file==NULL) {fprintf(stderr,"cannot open data file\n");return 0;}
	if(code_file==NULL) {fprintf(stderr,"cannot open compressed file\n");fclose(data_file);return 0;}
	
	const int NumModels = 16;                                                         // set data models
	Adaptive_Data_Model dm[NumModels];
	for (unsigned int m = 0; m < NumModels; m++) dm[m].set_alphabet(256);
			
	unsigned int bytes=0;
	fread(&bytes,1,sizeof(bytes),code_file);	// read original file size


	//////////////////////////////////////////////////////////////////////////
	// FILL THIS PART - DECODING CORE
	//////////////////////////////////////////////////////////////////////////


	fclose(data_file);                                     // done: close files
	fclose(code_file);	

	return 0;
}


int CACFileCoder::encode_file(char *datfile,char *cmpfile)
{
	// 64KB endcoding code was here. don't care about this function.
	return 0;
}

int CACFileCoder::decode_file(char *cmpfile,char *datfile)
{
	// 64KB decoding code was here. don't care about this function.
	return 0;
}