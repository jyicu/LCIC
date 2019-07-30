#pragma once

class Hierarchical_coder {
public:
	Hierarchical_coder(char filename[], char codename[], int T, int K);
	Hierarchical_coder(char filename[], int _T, int _K);
	~Hierarchical_coder();

	float run();

private:
	int height, width;
	char * codefile;
	bool print = true;

	int ** Y;
	int ** U_o1, ** U_o2, ** U_e1, ** U_e2;
	int ** V_o1, ** V_o2, ** V_e1, ** V_e2;
	FILE *fp;

	int T, K;
	int symMaxes[NUM_CTX];
	float p[NUM_CTX];
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

	int T, K;
	int symMaxes[NUM_CTX];
	float p[NUM_CTX];
	int ** decode_jpeg2000_8(char* filename);
	int** decode_jpeg2000_16(char* filename);
	void initCoder(Arithmetic_Codec* pCoder, Adaptive_Data_Model* pDm, FILE *fp);
	int decode_params(FILE *fp);
};