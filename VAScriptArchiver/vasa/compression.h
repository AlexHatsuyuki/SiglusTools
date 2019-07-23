/*
vagconv: g00 compression library
Copyright (C) 2006 Haeleth.
2014-2016 DENDentar.
2017-2019 AlexHatsuyuki

Based on code by Ed Keyes, with a few minor optimisations.
*/

unsigned int va_compress_g00_0(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize);
unsigned int va_compress_g00_1(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize);
unsigned int va_compress_seen_txt(unsigned char *source, unsigned int sourcelen, unsigned char *dest, int windowsize);

char va_decompress_g00_0(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len);
char va_decompress_g00_1(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len);
char va_decompress_seen_txt(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int dst_len);
