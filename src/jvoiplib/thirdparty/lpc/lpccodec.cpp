#include "lpccodec.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
	#define M_PI		3.14159265358979323846
#endif // M_PI

#define FS		8000.0	/* Sampling rate */

#define DOWN		5	/* Decimation for pitch analyzer */
#define PITCHORDER	4	/* Model order for pitch analyzer */
#define FC		600.0	/* Pitch analyzer filter cutoff */
#define MINPIT		50.0	/* Minimum pitch */
#define MAXPIT		300.0	/* Maximum pitch */

#define MINPER		(int)(FS/(DOWN*MAXPIT)+.5)	/* Minimum period */
#define MAXPER		(int)(FS/(DOWN*MINPIT)+.5)	/* Maximum period */

#define WSCALE		1.5863	/* Energy loss due to windowing */

#ifdef WIN32
	#define bcopy(src,dst,num)	memcpy(dst,src,num)
#endif // WIN32

#define GAIN_ADJUST	0.9
#define SILENCEFIX	/* Enable absolute silence fix */

LPCEncoder::LPCEncoder()
{
	Reset();
}

LPCEncoder::~LPCEncoder()
{
}

void LPCEncoder::Reset()
{
	vuv = 0;
	srand(time(NULL));
	lpc_init();
}

void LPCEncoder::Encode(int samples[LPCCODEC_INPUTLEN],unsigned char frame[LPCCODEC_OUTPUTLEN])
{
	lpcparams_t params;

	lpc_analyze(samples,&params);
	
	// encode in network byte order
	frame[0] = (unsigned char)((params.period >> 8) & 0xFF);
	frame[1] = (unsigned char)(params.period & 0xFF);

	frame[2] = params.gain;
	frame[3] = 0;
	memcpy(frame+4,&(params.k),LPCCODEC_FILTORDER);				      
}

void LPCEncoder::lpc_analyze(int *buf, lpcparams_t *params)
{
	int i, j;
	float w[LPCCODEC_MAXWINDOW], r[LPCCODEC_FILTORDER+1];
	float per, G, k[LPCCODEC_FILTORDER+1];

	for (i=0, j=LPCCODEC_BUFLEN-LPCCODEC_INPUTLEN; i < LPCCODEC_INPUTLEN; i++, j++) 
	{
		s[j] = (float)(GAIN_ADJUST*((float)buf[i])/32768.0);
		u = fa[2] * s[j] - fa[1] * u1;
		y[j] = fa[5] * u1 - fa[3] * yp1 - fa[4] * yp2;
		u1 = u;
		yp2 = yp1;
		yp1 = y[j];
	}

  	calc_pitch(y, &per);

  	for (i=0; i < LPCCODEC_BUFLEN; i++) 
		w[i] = s[i] * h[i];
  
  	auto_correl(w, LPCCODEC_BUFLEN, LPCCODEC_FILTORDER, r);
  	durbin(r, LPCCODEC_FILTORDER, k, &G);

  	params->period = (unsigned short)(per * (float)(1<<8));

#ifdef NO_LPC_FIX
	params->gain = (unsigned short)(G * (float)(1<<8));
#else
	i = (int)(G * (float)(1<<8));
	if (i > 255)
		i = 255;
	else if (i < 0)
		i = 0;
	params->gain = (unsigned short)i;
#endif // NO_LPC_FIX

  	for (i=0; i < LPCCODEC_FILTORDER; i++) 
	{
#ifdef NO_LPC_FIX
		params->k[i] = (char)(k[i+1] * (float)(1<<7));
#else
		int val;

		val = (int)(k[i+1] * (float)(1<<7));
		if (val < -128)
			val = -128;
		else if (val > 127)
			val = 127;
		params->k[i] = (char)val;
#endif // NO_LPC_FIX
	}
	bcopy(s+LPCCODEC_INPUTLEN, s, (LPCCODEC_BUFLEN-LPCCODEC_INPUTLEN)*sizeof(s[0]));
	bcopy(y+LPCCODEC_INPUTLEN, y, (LPCCODEC_BUFLEN-LPCCODEC_INPUTLEN)*sizeof(y[0]));
}

void LPCEncoder::calc_pitch(float w[], float *per)
{
	int i, j, rpos;
	float d[LPCCODEC_MAXWINDOW/DOWN], k[PITCHORDER+1], r[MAXPER+1], g, rmax;
	float rval, rm, rp;
	float a, b, c, x, y;

#ifdef NO_LPC_FIX
	/* Old decimation sometimes fails to recognise voiced. */
	for (i=0, j=0; i < LPCCODEC_BUFLEN; i+=DOWN) 
		d[j++] = w[i];
#else
	/* New: average rather than decimating. */
	for (i = 0, j = 0; i < LPCCODEC_BUFLEN; j++) 
	{
		d[j] = 0;
		for (rpos = 0; rpos < DOWN; rpos++) 
			d[j] += w[i++];
	}
#endif // NO_LPC_FIX
		
	auto_correl(d, LPCCODEC_BUFLEN/DOWN, PITCHORDER, r);
  	durbin(r, PITCHORDER, k, &g);
  	inverse_filter(d, k);

#ifdef NO_LPC_FIX
  	auto_correl(d, LPCCODEC_BUFLEN/DOWN, MAXPER+1, r);
#else
	auto_correl(d, LPCCODEC_BUFLEN/DOWN, MAXPER, r);
#endif // NO_LPC_FIX

  	rpos = 0;
  	rmax = 0.0;
  	for (i = MINPER; i <= MAXPER; i++) 
	{
    		if (r[i] > rmax) 
		{
      			rmax = r[i];
     			 rpos = i;
    		}
	}
  
	rm = r[rpos-1];
	rp = r[rpos+1];

#ifdef OLDWAY
	rval = rmax / r[0];
#endif // OLDWAY

	a = 0.5 * rm - rmax + 0.5 * rp;
	b = -0.5*rm*(2.0*rpos+1.0) + 2.0*rpos*rmax + 0.5*rp*(1.0-2.0*rpos);
	c = 0.5*rm*(rpos*rpos+rpos) + rmax*(1.0-rpos*rpos) + 0.5*rp*(rpos*rpos-rpos);

	x = -b / (2.0 * a);
	y = a*x*x + b*x + c;
	x *= DOWN;

	rmax = y;

#ifdef OLDWAY
	rval = rmax / r[0];
#else
	if (r[0] == 0)
		rval = 1.0;
	else
		rval = rmax / r[0];
#endif // OLDWAY

#ifdef NO_LPC_FIX
	if (rval >= 0.4 || (vuv == 3 && rval >= 0.3)) 
#else
	if ((rval >= 0.4 || (vuv == 3 && rval >= 0.3)) && (x > 0))
#endif // NO_LPC_FIX
	{
		*per = x;
		vuv = (vuv & 1) * 2 + 1;
	} 
	else 
	{
		*per = 0.0;
		vuv = (vuv & 1) * 2;
	}
}

void LPCEncoder::auto_correl(float w[], int n, int p, float r[])
{
	int i, k, nk;

	for (k=0; k <= p; k++) 
	{
		nk = n-k;	
		r[k] = 0.0;
		for (i=0; i < nk; i++) 
			r[k] += w[i] * w[i+k];
  	}
}

void LPCEncoder::durbin(float r[], int p, float k[], float *g)
{
	int i, j;
	float a[LPCCODEC_FILTORDER+1], at[LPCCODEC_FILTORDER+1], e;
  
	for (i=0; i <= p; i++) 
		a[i] = at[i] = 0.0;
    
	e = r[0];
	for (i=1; i <= p; i++) 
	{
		k[i] = -r[i];
		for (j=1; j < i; j++) 
		{
			at[j] = a[j];
			k[i] -= a[j] * r[i-j];
		}
#ifdef SILENCEFIX
		if (e == 0)
		{
			*g = 0;
			return;
		}
#endif // SILENCEFIX
		k[i] /= e;
		a[i] = k[i];
		for (j=1; j < i; j++) 
			a[j] = at[j] + k[i] * at[i-j];
    		e *= 1.0 - k[i]*k[i];
	}

#ifdef SILENCEFIX
	if (e < 0)
		e = 0;
#endif // SILENCEFIX

	*g = sqrt(e);
}

void LPCEncoder::inverse_filter(float w[], float k[])
{
	int i, j;
	float b[PITCHORDER+1], bp[PITCHORDER+1], f[PITCHORDER+1];
  
 	for (i = 0; i <= PITCHORDER; i++) 
		b[i] = f[i] = bp[i] = 0.0;
    
	for (i = 0; i < LPCCODEC_BUFLEN/DOWN; i++) 
	{
		f[0] = b[0] = w[i];
		for (j = 1; j <= PITCHORDER; j++) 
		{
			f[j] = f[j-1] + k[j] * bp[j-1];
			b[j] = k[j] * f[j-1] + bp[j-1];
			bp[j-1] = b[j-1];
		}
		w[i] = f[PITCHORDER];
	}
}

void LPCEncoder::lpc_init()
{
	int i;
	float r, v, w, wcT;

	for (i = 0; i < LPCCODEC_BUFLEN; i++) 
	{
		s[i] = 0.0;
		h[i] = WSCALE*(0.54 - 0.46 * cos(2 * M_PI * i / (LPCCODEC_BUFLEN-1.0)));
	}

	wcT = 2 * M_PI * FC / FS;
	r = 0.36891079 * wcT;
	v = 0.18445539 * wcT;
	w = 0.92307712 * wcT;
	fa[1] = -exp(-r);
	fa[2] = 1.0 + fa[1];
	fa[3] = -2.0 * exp(-v) * cos(w);
	fa[4] = exp(-2.0 * v);
	fa[5] = 1.0 + fa[3] + fa[4];

	u1 = 0.0;
	yp1 = 0.0;
	yp2 = 0.0;
}

LPCDecoder::LPCDecoder()
{
	Reset();
}

LPCDecoder::~LPCDecoder()
{
}

void LPCDecoder::Reset()
{
	lpc_init();
}

void LPCDecoder::Decode(unsigned char frame[LPCCODEC_OUTPUTLEN],int samples[LPCCODEC_INPUTLEN])
{
	lpcparams_t params;

	// period was transmitted in network byte order
	params.period = ((unsigned short)frame[1]) | (((unsigned short)frame[0])<<8);
	
	params.gain = frame[2];
	memcpy(&(params.k),frame+4,LPCCODEC_FILTORDER);
	lpc_synthesize(&params,samples);
}

void LPCDecoder::lpc_synthesize(lpcparams_t *params, int *buf)
{
	int i, j ;
	float per, G, k[LPCCODEC_FILTORDER+1];
	float u, NewG, Ginc, Newper, perinc;
	float Newk[LPCCODEC_FILTORDER+1], kinc[LPCCODEC_FILTORDER+1];

	per = (float)params->period / (1<<8);
	G = (float)params->gain / (1<<8);
	k[0] = 0.0;	
	
	for (i=0; i < LPCCODEC_FILTORDER; i++) 
		k[i+1] = (float)params->k[i] / (1<<7);

	if (per == 0.0)
    		G /= sqrt(LPCCODEC_BUFLEN/3.0);
	else 
	{
    		i = (int)((float)LPCCODEC_BUFLEN / per);
    		if (i == 0) 
			i = 1;
    		G /= sqrt((float)i);
  	}

  	Newper = Oldper;
  	NewG = OldG;
  	for (i=1; i <= LPCCODEC_FILTORDER; i++) 
		Newk[i] = Oldk[i];
    
  	if (Oldper != 0 && per != 0) 
	{
    		perinc = (per-Oldper) / LPCCODEC_INPUTLEN;
    		Ginc = (G-OldG) / LPCCODEC_INPUTLEN;
    		for (i=1; i <= LPCCODEC_FILTORDER; i++) 
			kinc[i] = (k[i]-Oldk[i]) / LPCCODEC_INPUTLEN;
  	} 
	else 
	{
		perinc = 0.0;
    		Ginc = 0.0;
    		for (i=1; i <= LPCCODEC_FILTORDER; i++) 
			kinc[i] = 0.0;
 	 }
    
  	if (Newper == 0) 
		pitchctr = 0;
    
	for (i=0; i < LPCCODEC_INPUTLEN; i++) 
	{
    		if (Newper == 0) 
#ifdef NO_LPC_FIX
   			u = (((double)rand())/(((double)RAND_MAX)+1.0)) * NewG;
#else
			u = (((double)rand() / (1.0 + (double)RAND_MAX)) - 0.5) * 1.5874 * NewG;
#endif // NO_LPC_FIX
		else 
		{
      			if (pitchctr == 0) 
			{
        			u = NewG;
        			pitchctr = (int) Newper;
      			} 
			else 
			{
        			u = 0.0;
        			pitchctr--;
      			}
    		}
      
    		f[LPCCODEC_FILTORDER] = u;
    		for (j=LPCCODEC_FILTORDER; j >= 1; j--) 
		{
      			f[j-1] = f[j] - Newk[j] * bp[j-1];
      			b[j] = Newk[j] * f[j-1] + bp[j-1];
      			bp[j] = b[j];
		}
    		b[0] = bp[0] = f[0];
    
		buf[i] = (int) (b[0] * 32768.0);

    		Newper += perinc;
    		NewG += Ginc;
    		for (j=1; j <= LPCCODEC_FILTORDER; j++) 
			Newk[j] += kinc[j];
  	}
  
  	Oldper = per;
  	OldG = G;
  	for (i=1; i <= LPCCODEC_FILTORDER; i++) 
		Oldk[i] = k[i];
}

void LPCDecoder::lpc_init()
{
	int i;

	Oldper = 0.0;
	OldG = 0.0;
	for (i=1; i <= LPCCODEC_FILTORDER; i++) 
		Oldk[i] = 0.0;
	for (i=0; i <= LPCCODEC_FILTORDER; i++) b[i] = bp[i] = f[i] = 0.0;
 		pitchctr = 0;
}

