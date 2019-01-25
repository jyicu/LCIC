// ACFileCoder.h: interface for the CACFileCoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACFILECODER_H__9124D29B_CB24_403F_9B37_4ACC5D5D8CF0__INCLUDED_)
#define AFX_ACFILECODER_H__9124D29B_CB24_403F_9B37_4ACC5D5D8CF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CACFileCoder  
{
public:
	CACFileCoder();
	virtual ~CACFileCoder();
	int encode_file(char *datfile,char *cmpfile);
	int decode_file(char *cmpfile,char *datfile);

	int encode_file_sim(char *datfile,char *cmpfile);
	int decode_file_sim(char *cmpfile,char *datfile);


};

#endif // !defined(AFX_ACFILECODER_H__9124D29B_CB24_403F_9B37_4ACC5D5D8CF0__INCLUDED_)
