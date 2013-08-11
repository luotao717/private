#ifndef __G711_H_XX__
#define __G711_H_XX__

unsigned char    linear2ulaw(short pcm_val);
short            ulaw2linear(unsigned char u_val);

unsigned char    linear2alaw(short pcm_val);
short            alaw2linear(unsigned char a_val);

unsigned char    alaw2ulaw(unsigned char a_val);
unsigned char    ulaw2alaw(unsigned char u_val);

#endif



