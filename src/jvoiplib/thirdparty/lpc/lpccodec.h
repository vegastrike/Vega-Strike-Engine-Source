#ifndef LPCCODEC_H

#define LPCCODEC_H

#define LPCCODEC_INPUTLEN					160
#define LPCCODEC_OUTPUTLEN					14
#define LPCCODEC_BUFLEN						((LPCCODEC_INPUTLEN*3)/2)
#define LPCCODEC_FILTORDER					10
#define LPCCODEC_MAXWINDOW					1000	/* Max analysis window length */

struct lpcparams_t 
{
	unsigned short period;
	unsigned char gain;
	char k[LPCCODEC_FILTORDER];
};


class LPCEncoder
{
public:
	LPCEncoder();
	~LPCEncoder();
	void Reset();
	void Encode(int samples[LPCCODEC_INPUTLEN],unsigned char frame[LPCCODEC_OUTPUTLEN]);
private:
	void lpc_analyze(int *buf, lpcparams_t *params);
	void calc_pitch(float w[], float *per);
	void auto_correl(float w[], int n, int p, float r[]);
	void durbin(float r[], int p, float k[], float *g);
	void inverse_filter(float w[], float k[]);
	void lpc_init();
	
	float s[LPCCODEC_MAXWINDOW],y[LPCCODEC_MAXWINDOW],h[LPCCODEC_MAXWINDOW];
	float fa[6],u,u1,yp1,yp2;
	int vuv;	
};

class LPCDecoder
{
public:
	LPCDecoder();
	~LPCDecoder();
	void Reset();
	void Decode(unsigned char frame[LPCCODEC_OUTPUTLEN],int samples[LPCCODEC_INPUTLEN]);
private:
	void lpc_synthesize(lpcparams_t *params, int *buf);
	void lpc_init();

	float Oldper, OldG, Oldk[LPCCODEC_FILTORDER+1];
	float b[LPCCODEC_FILTORDER+1],bp[LPCCODEC_FILTORDER+1],f[LPCCODEC_FILTORDER+1];
	int pitchctr;
};

#endif // LPCCODEC_H
