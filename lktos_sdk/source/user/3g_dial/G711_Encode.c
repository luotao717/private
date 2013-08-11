/* g.711 u-law, A-law define */
#define	SIGN_BIT		(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS			(8)			/* Number of A-law segments. */
#define	SEG_SHIFT		(4)			/* Left shift for segment number. */
#define	SEG_MASK		(0x70)		/* Segment field mask. */
#define	BIAS			(0x84)		/* Bias for linear code. */
#define 	CLIP			(8159)

static int seg_aend[8] = {0x1F, 0x3F, 0x7F, 0xFF,
                            0x1FF, 0x3FF, 0x7FF, 0xFFF};
static int seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF,
                            0x3FF, 0x7FF, 0xFFF, 0x1FFF};

/* declare wave encoder functions */
int linear2alaw(int pcm_val);
int linear2ulaw( int pcm_val);
int search(int val, int *table, int size) ;


/*
 * linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * linear2alaw() accepts an 16-bit integer and encodes it as A-law data.
 *
 *              Linear Input Code       Compressed Code
 *      ------------------------        ---------------
 *      0000000wxyza                    000wxyz
 *      0000001wxyza                    001wxyz
 *      000001wxyzab                    010wxyz
 *      00001wxyzabc                    011wxyz
 *      0001wxyzabcd                    100wxyz
 *      001wxyzabcde                    101wxyz
 *      01wxyzabcdef                    110wxyz
 *      1wxyzabcdefg                    111wxyz
 */
  int linear2alaw(int pcm_val)        /* 2's complement (16-bit range) */
                                        /* changed from "short" *drago* */
{
	int mask;	/* changed from "short" *drago* */
	int seg;		/* changed from "short" *drago* */
	int aval;

	pcm_val = pcm_val >> 3;

	if (pcm_val >= 0) {
		mask = 0xD5;            /* sign (7th) bit = 1 */
	} else {
		mask = 0x55;            /* sign bit = 0 */
		pcm_val = -pcm_val - 1;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_aend, 8);

	/* Combine the sign, segment, and quantization bits. */
	if (seg >= 8)           /* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		aval = seg << SEG_SHIFT;
		if (seg < 2)
			aval |= (pcm_val >> 1) & QUANT_MASK;
		else
			aval |= (pcm_val >> seg) & QUANT_MASK;
		return (aval ^ mask);
	}
}

/*
 * linear2ulaw() - Convert a linear PCM value to u-law
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *      Biased Linear Input Code        Compressed Code
 *      ------------------------        ---------------
 *      00000001wxyza                   000wxyz
 *      0000001wxyzab                   001wxyz
 *      000001wxyzabc                   010wxyz
 *      00001wxyzabcd                   011wxyz
 *      0001wxyzabcde                   100wxyz
 *      001wxyzabcdef                   101wxyz
 *      01wxyzabcdefg                   110wxyz
 *      1wxyzabcdefgh                   111wxyz
 */
int linear2ulaw( int    pcm_val)        /* 2's complement (16-bit range) */
{
	int mask;
	int seg;
	int uval;

	/* Get the sign and the magnitude of the value. */
	pcm_val = pcm_val >> 2;
	if (pcm_val < 0) {
		pcm_val = -pcm_val;
		mask = 0x7F;
	} else {
		mask = 0xFF;
	}
	if ( pcm_val > CLIP ) pcm_val = CLIP;           /* clip the magnitude */
	pcm_val += (BIAS >> 2);

	/* Convert the scaled magnitude to segment number. */
        seg = search(pcm_val, seg_uend, 8);

	/*
	* Combine the sign, segment, quantization bits;
	* and complement the code word.
	*/
	if (seg >= 8)           /* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		uval = (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
	return (uval ^ mask);
	}
}

static int search(int val, int *table, int size)   /* changed from "short" *drago* */
{
	int i;
	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

