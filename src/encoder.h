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



class Encoder {
public:
	Encoder(unsigned char ** I, int T, int K, int height, int width);
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




