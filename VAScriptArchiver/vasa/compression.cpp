/*
vagconv: g00 compression library
Copyright (C) 2006 Haeleth.
2014-2016 DENDentar.
2017-2019 AlexHatsuyuki

Based on code by Ed Keyes, with a few minor optimisations.
*/

#include "common.h"

// ----------------  G00 format 0 compression  ------------------//
/* Compress RGB data in g00 format 0 */
inline int ek_findbestmatchType0(unsigned char *source, unsigned int sourcelen, unsigned char *currentpos, int *foundoffset, int windowsize)
{
	int bestlength = 0;
	int i, j;

	i = 1;
	while ((currentpos - i * 3 >= source) &&   // stop at beginning of data
		(i < windowsize) &&   // limit to 12-bit offset
		(bestlength < 16)) {  // quit if we already find an optimal match
			j = 0;
			// only check if this position might hold a longer match
			if (currentpos[3 * bestlength] == (currentpos - 3 * i)[3 * bestlength] &&
				currentpos[3 * bestlength + 1] == (currentpos - 3 * i)[3 * bestlength + 1] &&
				currentpos[3 * bestlength + 2] == (currentpos - 3 * i)[3 * bestlength + 2]) {
					while ((currentpos + 3 * j < source + sourcelen) &&  // stop at end of data
						(currentpos[3 * j] == (currentpos - 3 * i)[3 * j]) &&  // do we match?
						(currentpos[3 * j + 1] == (currentpos - 3 * i)[3 * j + 1]) &&
						(currentpos[3 * j + 2] == (currentpos - 3 * i)[3 * j + 2]) &&
						(j < 16)) {   // limit to 4-bit size
							++j;
							if (j > bestlength) {
								bestlength = j;
								*foundoffset = i;
							}
					}
			}
			++i;
	}
	return bestlength;
}

unsigned int va_compress_g00_0(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize)
//unsigned int ek_LZSSCompressType0(unsigned char *source, unsigned int sourcelen, unsigned char *dest)
{
	int size, offset, bitcount;
	unsigned int compressedsize;
	unsigned char *control;
	unsigned char *cursrc, *curdest;

	compressedsize = 0;
	cursrc = source;
	bitcount = 8;
	curdest = dest;
	while (cursrc < source + sourcelen) {
		if (bitcount >  7) {
			control = curdest++;
			*control = 0;
			bitcount = 0;
		}
		size = ek_findbestmatchType0(source, sourcelen, cursrc, &offset, windowsize);
		if (size > 0) {  // use an offset for 1-pixel matches or above
			// a zero flag bit in the control byte is already there
			*curdest++ = ((offset & 0x0f) << 4) | (size - 1);
			*curdest++ = (offset >> 4);
			cursrc += size * 3;
		}
		else {  // store raw pixel
			*control = (*control) | (0x01 << bitcount);  // flag for raw byte
			*curdest++ = *cursrc++;
			*curdest++ = *cursrc++;
			*curdest++ = *cursrc++;
		}
		++bitcount;
	}
	return curdest - dest;
}

/* Decompress RGB data in g00 format 0 */
char va_decompress_g00_0(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len)
{
	const unsigned char* srcend = src + src_len;
	unsigned char* dststart = dst;
	unsigned char* dstend = dst + dst_len;
	
	unsigned int bit = 1;
	unsigned char flag = *src++;
	while ((src < srcend) && (dst < dstend)) {
		if (bit == 256) {
			flag = *src++;
			bit = 1;
		}
		if (flag & bit) {
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
		else {
			unsigned int i, count;
			unsigned char *repeat;
			count = *src++;
			count += (*src++) << 8;
			repeat = dst - (count >> 4) * 3;
			count = ((count & 0x0f) + 1) * 3;
			if (repeat < dststart || repeat >= dst) return 0;
			for (i = 0; i < count && dst < dstend; ++i) *dst++ = *repeat++;
		}
		bit <<= 1;
	}
	return 1;
}
// ---------------------------------------------------------------------- //


// ----  Compress SiglusEngine Scene.pck/Gameexe.dat end g00 ver 1/2 ---- //
inline int ek_findbestmatch(unsigned char *source, unsigned int sourcelen, unsigned char *currentpos, int *foundoffset, int windowsize)
{
	int bestlength = 0;
	int i, j;

	i = 1;
	while ((currentpos - i >= source) &&   // stop at beginning of data
		(i < windowsize) &&   // limit to 12-bit offset
		(bestlength < 17)) {  // quit if we already find an optimal match
			j = 0;
			// only check if this position might hold a longer match
			if (currentpos[bestlength] == (currentpos - i)[bestlength]) {
				while ((currentpos + j < source + sourcelen) &&  // stop at end of data
					(currentpos[j] == (currentpos - i)[j]) &&  // do we actually match?
					(j < 17)) {   // limit to 4-bit size
						++j;
						if (j > bestlength) {
							bestlength = j;
							*foundoffset = i;
						}
				}
			}
			++i;
	}
	return bestlength;
}

/* Compress SiglusEngine Scene.pck/Gameexe.dat and g00 ver 1/2  */
unsigned int va_compress_g00_1(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize)
{
	int size, offset, bitcount;
	unsigned int compressedsize;
	unsigned char *control;
	unsigned char *cursrc, *curdest;

	compressedsize = 0;
	cursrc = source;
	bitcount = 8;
	curdest = dest;
	while (cursrc < source + sourcelen) {
		if (bitcount > 7) {
			control = curdest++;
			*control = 0;
			bitcount = 0;
		}
		size = ek_findbestmatch (source, sourcelen, cursrc, &offset, windowsize);
		if (size > 1) {  // use an offset for 2-byte matches or above
			// a zero flag bit in the control byte is already there
			*curdest++ = ((offset & 0x0f) << 4) | (size - 2);
			*curdest++ = (offset >> 4);
			cursrc += size;
		}
		else {  // store raw byte
			*control = (*control) | (0x01 << bitcount);  // flag for raw byte
			*curdest++ = *cursrc++;
		}
		++bitcount;
	}
	return curdest - dest;
}


/* Decompress SiglusEngine Scene.pck/Gameexe.dat and g00 ver 1/2  */
char va_decompress_g00_1(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len)
{
	const unsigned char* srcend = src + src_len;
	unsigned char* dststart = dst;
	unsigned char* dstend = dst + dst_len;
	
	unsigned int bit = 1;
	unsigned char flag = *src++;
	while ((src < srcend) && (dst < dstend)) {
		if (bit == 256) {
			flag = *src++;
			bit = 1;
		}
		if (flag & bit) {
			*dst++ = *src++;
		}
		else {
			unsigned int i, count;
			unsigned char *repeat;
			count = *src++;
			count += (*src++) << 8;
			repeat = dst - (count >> 4); // *.dbs, *.g00 format 1 and 2, SiglusEngine Scene.pck/Gameexe.dat
			//repeat = dst - ((count >> 4) - 1) - 1; // RealLive SEEN.TXT
			count = (count & 0x0f) + 2;
			if (repeat < dststart || repeat >= dst) return 0;
			for (i = 0; i < count && dst < dstend; ++i) *dst++ = *repeat++;
		}
		bit <<= 1;
	}
	return 1;
}
// ---------------------------------------------------------------------- //

// --------------------- Compress RealLive SEEN.TXT --------------------- //
unsigned int va_compress_seen_txt(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize)
{
	int size, offset, bitcount;
	unsigned int compressedsize;
	unsigned char *control;
	unsigned char *cursrc, *curdest;

	compressedsize = 0;
	cursrc = source;
	bitcount = 8;
	curdest = dest;
	while (cursrc < source + sourcelen) {
		if (bitcount > 7) {
			control = curdest++;
			*control = 0;
			bitcount = 0;
		}
		size = ek_findbestmatch (source, sourcelen, cursrc, &offset, windowsize);
		if (size > 1) {  // use an offset for 2-byte matches or above
			// a zero flag bit in the control byte is already there
			*curdest++ = ((offset & 0x0f) << 4) | (size - 2);
			*curdest++ = ((offset + 2) >> 4);
			cursrc += size;
		}
		else {  // store raw byte
			*control = (*control) | (0x01 << bitcount);  // flag for raw byte
			*curdest++ = *cursrc++;
		}
		++bitcount;
	}
	return curdest - dest;
}

/* Decompress RealLive SEEN.TXT */
char va_decompress_seen_txt(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len)
{
	const unsigned char* srcend = src + src_len;
	unsigned char* dststart = dst;
	unsigned char* dstend = dst + dst_len;
	
	unsigned int bit = 1;
	unsigned char flag = *src++;
	while ((src < srcend) && (dst < dstend)) {
		if (bit == 256) {
			flag = *src++;
			bit = 1;
		}
		if (flag & bit) {
			*dst++ = *src++;
		}
		else {
			unsigned int i, count;
			unsigned char *repeat;
			count = *src++;
			count += (*src++) << 8;
			//repeat = dst - (count >> 4); // *.dbs, *.g00 format 1 and 2, SiglusEngine Scene.pck/Gameexe.dat
			repeat = dst - ((count >> 4) - 1) - 1; // RealLive SEEN.TXT
			count = (count & 0x0f) + 2;
			if (repeat < dststart || repeat >= dst) return 0;
			for (i = 0; i < count && dst < dstend; ++i) *dst++ = *repeat++;
		}
		bit <<= 1;
	}
	return 1;
}
// ---------------------------------------------------------------------- //
