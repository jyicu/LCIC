#pragma once
#pragma warning(disable: 4996)

#define MAX(x,y)	((x)>(y) ? (x) : (y))
#define MIN(x,y)	((x)<(y) ? (x) : (y))
#define MAX4(X)		MAX(MAX((X)[0],(X)[1]),MAX((X)[2],(X)[3]))
#define MIN4(X)		MIN(MIN((X)[0],(X)[1]),MIN((X)[2],(X)[3]))
#define ABS(x)		((x)>0 ? (x) : -(x))
#define CLIP(x,y,z)	MAX((y), MIN((x), (z)))
#define CLIP255(x)	MAX(0, MIN((x), 255))
#define ROUND(x)	((int)((x)+0.5))
#define UINT8(x)	CLIP255(ROUND(x))


template <typename T>
T **alloc2D(int height, int width);
template <typename T>
void free2D(T **p);

class Hierarchical_coder {
public:
	Hierarchical_coder(char filename[]);
	~Hierarchical_coder();

	int run();

private:
	int height, width;

	int ** Y;
	int ** U_o1, ** U_o2, ** U_e1, ** U_e2;
	int ** V_o1, ** V_o2, ** V_e1, ** V_e2;
};


class Encoder {
public:
	Encoder(int ** I, int T, int K, int height, int width);
	~Encoder();
	
	int run();

private:
	int height, width;
	int T, K;
	int ** X_e;
	int ** X_o;
	int ** sigma;
	bool ** Dir;
	int * q;

	void init();
	void set_local_activity();
	void context_modeling();
	int context(int x, int y);
};




