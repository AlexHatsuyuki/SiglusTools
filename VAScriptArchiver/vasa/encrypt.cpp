/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

#include "common.h"

void data_xor(unsigned char *data, int data_len, unsigned char *key, int key_len)
{
	unsigned char *key_start = key;
	const unsigned char *key_end = key + key_len;
	while(data_len--)
	{
		*data = *data++^*key++;
		if (key >= key_end) key = key_start;
	}
}
