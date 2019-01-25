#include "lifting.h"
#include "malloc.h"

void jpc_qmfb_split_row(jpc_fix_t *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	jpc_fix_t splitbuf[QMFB_SPLITBUFSIZE];
	jpc_fix_t *buf = splitbuf;
	register jpc_fix_t *srcptr;
	register jpc_fix_t *dstptr;
	register int n;
	register int m;
	int hstartcol;

	if (numcols >= 2) {
		hstartcol = (numcols + 1 - parity) >> 1;
		m = (parity) ? hstartcol : (numcols - hstartcol);
		/* Save the samples destined for the highpass channel. */
		n = m;
		dstptr = buf;
		srcptr = &a[1 - parity];
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the appropriate samples into the lowpass channel. */
		dstptr = &a[1 - parity];
		srcptr = &a[2 - parity];
		n = numcols - m - (!parity);
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the saved samples into the highpass channel. */
		dstptr = &a[hstartcol];
		srcptr = buf;
		n = m;
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			++srcptr;
		}
	}
}

void jpc_qmfb_split_row_float(float *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	float splitbuf[QMFB_SPLITBUFSIZE];
	float *buf = splitbuf;
	register float *srcptr;
	register float *dstptr;
	register int n;
	register int m;
	int hstartcol;

	if (numcols >= 2) {
		hstartcol = (numcols + 1 - parity) >> 1;
		m = (parity) ? hstartcol : (numcols - hstartcol);
		/* Save the samples destined for the highpass channel. */
		n = m;
		dstptr = buf;
		srcptr = &a[1 - parity];
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the appropriate samples into the lowpass channel. */
		dstptr = &a[1 - parity];
		srcptr = &a[2 - parity];
		n = numcols - m - (!parity);
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the saved samples into the highpass channel. */
		dstptr = &a[hstartcol];
		srcptr = buf;
		n = m;
		while (n-- > 0) {
			*dstptr = *srcptr;
			++dstptr;
			++srcptr;
		}
	}
}

void jpc_qmfb_join_row(jpc_fix_t *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	jpc_fix_t joinbuf[QMFB_JOINBUFSIZE];
	jpc_fix_t *buf = joinbuf;
	register jpc_fix_t *srcptr;
	register jpc_fix_t *dstptr;
	register int n;
	int hstartcol;

	hstartcol = (numcols + 1 - parity) >> 1;

	/* Save the samples from the lowpass channel. */
	n = hstartcol;
	srcptr = &a[0];
	dstptr = buf;
	while (n-- > 0) {
		*dstptr = *srcptr;
		++srcptr;
		++dstptr;
	}
	/* Copy the samples from the highpass channel into place. */
	srcptr = &a[hstartcol];
	dstptr = &a[1 - parity];
	n = numcols - hstartcol;
	while (n-- > 0) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
	/* Copy the samples from the lowpass channel into place. */
	srcptr = buf;
	dstptr = &a[parity];
	n = hstartcol;
	while (n-- > 0) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
}

void jpc_qmfb_join_row_float(float *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	float joinbuf[QMFB_JOINBUFSIZE];
	float *buf = joinbuf;
	register float *srcptr;
	register float *dstptr;
	register int n;
	int hstartcol;

	hstartcol = (numcols + 1 - parity) >> 1;

	/* Save the samples from the lowpass channel. */
	n = hstartcol;
	srcptr = &a[0];
	dstptr = buf;
	while (n-- > 0) {
		*dstptr = *srcptr;
		++srcptr;
		++dstptr;
	}
	/* Copy the samples from the highpass channel into place. */
	srcptr = &a[hstartcol];
	dstptr = &a[1 - parity];
	n = numcols - hstartcol;
	while (n-- > 0) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
	/* Copy the samples from the lowpass channel into place. */
	srcptr = buf;
	dstptr = &a[parity];
	n = hstartcol;
	while (n-- > 0) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
}

void jpc_ft_fwdlift_row(jpc_fix_t *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;
	register int n;
	int llen;

	llen = (numcols + 1 - parity) >> 1;

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] -= lptr[0];
			++hptr;
		}
		n = numcols - llen - parity - (parity == (numcols & 1));
		while (n-- > 0) {
			hptr[0] -= (lptr[0] + lptr[1]) >> 1;
			++hptr;
			++lptr;
		}
		if (parity == (numcols & 1)) {
			hptr[0] -= lptr[0];
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			lptr[0] += (hptr[0] + 1) >> 1;
			++lptr;
		}
		n = llen - (!parity) - (parity != (numcols & 1));
		while (n-- > 0) {
			lptr[0] += (hptr[0] + hptr[1] + 2) >> 2;
			++lptr;
			++hptr;
		}
		if (parity != (numcols & 1)) {
			lptr[0] += (hptr[0] + 1) >> 1;
		}

	} else {

		if (parity) {
			lptr = &a[0];
			lptr[0] <<= 1;
		}

	}
}

void jpc_ft_fwdlift_row_float(float *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	register float *lptr;
	register float *hptr;
	register int n;
	int llen;

	llen = (numcols + 1 - parity) >> 1;

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] -= lptr[0];
			++hptr;
		}
		n = numcols - llen - parity - (parity == (numcols & 1));
		while (n-- > 0) {
			hptr[0] -= (lptr[0] + lptr[1]) / 2;
			++hptr;
			++lptr;
		}
		if (parity == (numcols & 1)) {
			hptr[0] -= lptr[0];
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			lptr[0] += (hptr[0]) / 2;
			++lptr;
		}
		n = llen - (!parity) - (parity != (numcols & 1));
		while (n-- > 0) {
			lptr[0] += (hptr[0] + hptr[1]) / 4;
			++lptr;
			++hptr;
		}
		if (parity != (numcols & 1)) {
			lptr[0] += (hptr[0]) / 2;
		}

	} else {

		if (parity) {
			lptr = &a[0];
			lptr[0] *= 2;
		}

	}
}

void jpc_ft_invlift_row(jpc_fix_t *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;
	register int n;
	int llen;

	llen = (numcols + 1 - parity) >> 1;

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			lptr[0] -= (hptr[0] + 1) >> 1;
			++lptr;
		}
		n = llen - (!parity) - (parity != (numcols & 1));
		while (n-- > 0) {
			lptr[0] -= (hptr[0] + hptr[1] + 2) >> 2;
			++lptr;
			++hptr;
		}
		if (parity != (numcols & 1)) {
			lptr[0] -= (hptr[0] + 1) >> 1;
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] += lptr[0];
			++hptr;
		}
		n = numcols - llen - parity - (parity == (numcols & 1));
		while (n-- > 0) {
			hptr[0] += (lptr[0] + lptr[1]) >> 1;
			++hptr;
			++lptr;
		}
		if (parity == (numcols & 1)) {
			hptr[0] += lptr[0];
		}

	} else {

		if (parity) {
			lptr = &a[0];
			lptr[0] >>= 1;
		}

	}
}

void jpc_ft_invlift_row_float(float *a, int numcols)
{
	const int parity=0;		// assuming even numcols and start point

	register float *lptr;
	register float *hptr;
	register int n;
	int llen;

	llen = (numcols + 1 - parity) >> 1;

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			lptr[0] -= (hptr[0]) / 2;
			++lptr;
		}
		n = llen - (!parity) - (parity != (numcols & 1));
		while (n-- > 0) {
			lptr[0] -= (hptr[0] + hptr[1]) / 4;
			++lptr;
			++hptr;
		}
		if (parity != (numcols & 1)) {
			lptr[0] -= (hptr[0]) /2;
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] += lptr[0];
			++hptr;
		}
		n = numcols - llen - parity - (parity == (numcols & 1));
		while (n-- > 0) {
			hptr[0] += (lptr[0] + lptr[1]) /2;
			++hptr;
			++lptr;
		}
		if (parity == (numcols & 1)) {
			hptr[0] += lptr[0];
		}

	} else {

		if (parity) {
			lptr = &a[0];
			lptr[0] /= 2;
		}

	}
}

#define FLOATING_OPERATION
#ifdef FLOATING_OPERATION
int jpc_ft_analyze(jpc_fix_t *a, int numcols)
{
	float *b = (float*)malloc(sizeof(float)*numcols);

	for(int i=0; i<numcols; i++) b[i] = (float)a[i];

	jpc_qmfb_split_row_float(b, numcols);
	jpc_ft_fwdlift_row_float(b, numcols);

	for(int i=0; i<numcols; i++) a[i] = (jpc_fix_t)b[i];

	free(b);
	return 0;
}

int jpc_ft_synthesize(jpc_fix_t *a, int numcols)
{
	float *b = (float*)malloc(sizeof(float)*numcols);

	for(int i=0; i<numcols; i++) b[i] = (float)a[i];

	jpc_ft_invlift_row_float(b, numcols);
	jpc_qmfb_join_row_float(b, numcols);

	for(int i=0; i<numcols; i++) a[i] = (jpc_fix_t)b[i];

	free(b);
	return 0;
}
#else
int jpc_ft_analyze(jpc_fix_t *a, int numcols)
{
	jpc_qmfb_split_row(a, numcols);
	jpc_ft_fwdlift_row(a, numcols);

	return 0;
}

int jpc_ft_synthesize(jpc_fix_t *a, int numcols)
{
	jpc_ft_invlift_row(a, numcols);
	jpc_qmfb_join_row(a, numcols);

	return 0;
}
#endif	// FLOATING_OPERATION