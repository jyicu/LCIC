#pragma once


#include "src/"

class Hierarchical_coder {
public:
	Hierarchical_coder(char filename[], int T, int K, int symMax);
	~Hierarchical_coder();

	float run();

private:
	int height, width;

	int ** Y;
	int ** U_o1, ** U_o2, ** U_e1, ** U_e2;
	int ** V_o1, ** V_o2, ** V_e1, ** V_e2;
	FILE *fp;

	int T, K, symMax;
	int symMaxes[3];
	float p[3];
	void initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm);
	int encode_params(FILE *fp);
};

class Hierarchical_decoder {
public:
	Hierarchical_decoder();
	~Hierarchical_decoder();

	int run(char filename[], char imagename[]);

private:
	int height, width;

	int ** Y;
	int ** U_o1, ** U_o2, ** U_e1, ** U_e2;
	int ** V_o1, ** V_o2, ** V_e1, ** V_e2;
	FILE *fp;

	int T, K, symMax;
	int symMaxes[3];
	float p[3];
	int ** decode_jpeg2000(char* filename);
	void initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp);
	int decode_params(FILE *fp);
};