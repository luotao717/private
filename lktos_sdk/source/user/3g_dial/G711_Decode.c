/* g.711 u-law, A-law define */
#define	SIGN_BIT		(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS			(8)			/* Number of A-law segments. */
#define	SEG_SHIFT		(4)			/* Left shift for segment number. */
#define	SEG_MASK		(0x70)		/* Segment field mask. */
#define	BIAS			(0x84)		/* Bias for linear code. */

/* define the num of block header for MS_ADPCM and IMA_ADPCM */
#define IMA_ADPCM_PREAMBLE_SIZE 	4
#define MS_ADPCM_PREAMBLE_SIZE 	7

/* define useful macros for MS_ADPCM and IMA_ADPCM */
//change two 8bits to one 16bits
#define LE_16(x) ((x)[0]+(256*((x)[1])))

// clamp a number between 0 and 88
#define CLAMP_0_TO_88(x)  if (x < 0) x = 0; else if (x > 88) x = 88;

// clamp a number within a signed 16-bit range
#define CLAMP_S16(x)  if (x < -32768) x = -32768; else if (x > 32767) x = 32767;
  
// clamp a number above 16
#define CLAMP_ABOVE_16(x)  if (x < 16) x = 16;

// sign extend a 16-bit value
#define SE_16BIT(x)  if (x & 0x8000) x -= 0x10000;

// sign extend a 4-bit value
#define SE_4BIT(x)  if (x & 0x8) x -= 0x10;

/* Microsoft MS_ADPCM table */
static int ms_adapt_table[] =
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};

static int ms_adapt_coeff1[] =
{
	256, 512, 0, 192, 240, 460, 392
};

static int ms_adapt_coeff2[] =
{
	0, -256, 0, 64, 0, -208, -232
};

/* Intel IMA_ADPCM step variation table */
static int ima_adpcm_step[89] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int ima_adpcm_index[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

/* declare wave decoder functions */
static void ima_decode_nibbles(unsigned short *output, unsigned int output_size, int channels, int predictor_l, int index_l, int predictor_r, int index_r);
static int ima_adpcm_block_decoder(unsigned char *input, unsigned short *output, int channels, unsigned int block_size);
static int ms_adpcm_block_decoder(unsigned char *input, unsigned short *output, int channels, unsigned int block_size);
short alaw2linear(unsigned char a_val);
short ulaw2linear(unsigned char u_val);
	

/*
 * alaw2linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
short alaw2linear(unsigned char a_val)
{
	short t;
	short seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) {
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

/*
 * ulaw2linear() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
short ulaw2linear(unsigned char	u_val)
{
	short t;

	/* Complement to obtain normal u-law value. */
	u_val = ~u_val;

	/*
	 * Extract and bias the quantization bits. Then
	 * shift up by the segment number and subtract out the bias.
	 */
	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

