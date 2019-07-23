/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

#include "common.h"
#include "compression.h"

struct DBSHead
{
	int data_size;
	int row_cnt;
	int col_cnt;
	int row_off;
	int col_off;
	int idata_off;
	int sdata_off;
};

struct DBSColInfo
{
	int col_id;
	int data_type;
};

struct DBSColData
{
	int   col_id;
	char  data_type;
	void* data;
};

struct DBSRowData
{
	int			row_id;
	DBSColData  *col_dsta;
};

const unsigned int xor_mask1 = 0x89f4622d;

const unsigned int xor_mask2[] = {
0x7190c70e, 0x499bf135, 0x499bf135, 0x7190c70e, 0x7190c70e,
0x499bf135, 0x499bf135, 0x7190c70e, 0x7190c70e, 0x499bf135,
0x7190c70e, 0x7190c70e, 0x7190c70e, 0x499bf135, 0x7190c70e,
0x499bf135, 0x499bf135, 0x7190c70e, 0x499bf135, 0x499bf135,
0x499bf135, 0x499bf135, 0x499bf135, 0x7190c70e, 0x7190c70e
};

int print_dbs_info(wchar_t *file_name, int mode)
{
	errno_t err;
	FILE* dbs_f;
	err = open_bin_to_read(&dbs_f, file_name);
	if (err <= 0) { wprintf_api(2, L"Cannot open file: %s\n", file_name); return 0; }

	int file_size = err;
	int file_size_4 = (file_size - 4) / 4;
	if ((file_size - 4) % 4 > 0) file_size_4++;

	int dbs_format = 0;
	fread(&dbs_format, 4, 1, dbs_f);

	if (dbs_format != 0 && dbs_format != 1 && file_size_4 < 3) { wprintf_api(2, L"Invalid DBS file\n"); fclose(dbs_f); return 0; }

	unsigned char *dbs_data = (unsigned char*) malloc(file_size_4 * 4);
	if (!dbs_data) {  wprintf_api(2, L"malloc error\n"); fclose(dbs_f); return 0; }

	fread(dbs_data, 4, file_size_4, dbs_f);
	fclose(dbs_f);

	char dir_name[] = "\0";
	int strlen = 0;
	wchar_t out_file_name_xor1[MAX_PATH + 1];
	wchar_t out_file_name_raw[MAX_PATH + 1];
	wchar_t out_file_name_xor2[MAX_PATH + 1];
	wchar_t out_file_name[MAX_PATH + 1];
	strlen = swprintf(out_file_name_raw, MAX_PATH, L"%s%s-raw.raw", dir_name, file_name);
	if (strlen == -1) out_file_name_raw[0] = L'\0';
	strlen = swprintf(out_file_name_xor1, MAX_PATH, L"%s%s-xor1.raw", dir_name, file_name);
	if (strlen == -1) out_file_name_xor1[0] = L'\0';
	strlen = swprintf(out_file_name_xor2, MAX_PATH, L"%s%s-xor2.raw", dir_name, file_name);
	if (strlen == -1) out_file_name_xor2[0] = L'\0';
	strlen = swprintf(out_file_name, MAX_PATH, L"%s%s.csv", dir_name, file_name);
	if (strlen == -1) { wprintf_api(2, L"path is very long\r\n"); free(dbs_data); return 0; }

	unsigned int *dbs_data_4 = (unsigned int *) dbs_data;
	for (int i = 0; i < file_size_4; i++) dbs_data_4[i] = dbs_data_4[i] ^ xor_mask1;

	int compress_data_size = dbs_data_4[0];
	int uncompress_data_size = dbs_data_4[1];

	if (compress_data_size > file_size_4 * 4 || uncompress_data_size < compress_data_size || (uncompress_data_size % 4) != 0) {  wprintf_api(2, L"Corrupt DBS file\n"); free(dbs_data); return 0; }

	unsigned char *uncompress_data = (unsigned char*) malloc(uncompress_data_size);
	if (!uncompress_data) { wprintf_api(2, L"malloc error\n"); free(dbs_data); return 0; }

	if (!va_decompress_g00_1(dbs_data + 8, compress_data_size, uncompress_data, uncompress_data_size))
	{
		wprintf_api(2, L"decompress error\n");
		free(dbs_data);
		free(uncompress_data);
		return 0;
	}
	
	free(dbs_data);

	unsigned int *uncompress_data_4 = (unsigned int *) uncompress_data;
	char flag1 = 0;
    char flag2 = 0;
    for (int i = 0; i < uncompress_data_size / 4; i++)
	{
        uncompress_data_4[i] = uncompress_data_4[i] ^ xor_mask2[(flag1 * 5) + (flag2 % 5)];
        flag2++;
		if (flag2 >= 16) { flag1++; flag2 = 0; }
        if (flag1 >= 5) flag1 = 0;
    }

	DBSHead *dbs_head = (DBSHead*) uncompress_data;

	if (dbs_head->data_size > uncompress_data_size) {  wprintf_api(2, L"Corrupt DBS file\n"); free(uncompress_data); return 0; }

	wchar_t buf[MAX_PATH];
	buf[0] = L'\0';
	wchar_t buf1[MAX_PATH];
	wchar_t int_str[MAX_PATH];
	wcscat_s(buf, MAX_PATH, L"Archive type: dbs database\r\n");
	hum_size(int_str, file_size);
	wsprintf(buf1, L"  Archive size: %s (%u B)\r\n", int_str, file_size);
	wcscat_s(buf, MAX_PATH, buf1);
	hum_size(int_str, dbs_head->data_size);
	wsprintf(buf1, L"  Data size:    %s (%u B)\r\n", int_str, dbs_head->data_size);
	wcscat_s(buf, MAX_PATH, buf1);
	wsprintf(buf1, L"  Rows:         %u\r\n", dbs_head->row_cnt);
	wcscat_s(buf, MAX_PATH, buf1);
	wsprintf(buf1, L"  Columns:      %u\r\n", dbs_head->col_cnt);
	wcscat_s(buf, MAX_PATH, buf1);
	wprintf_api(0, L"%s", buf);

	free(uncompress_data);

	return 1;
}


int convert_dbs_to_csv(wchar_t *file_name, wchar_t *dir_name)
{
	errno_t err;
	FILE* dbs_f;
	err = open_bin_to_read(&dbs_f, file_name);
	if (err <= 0) { wprintf_api(2, L"Cannot open file: %s\n", file_name); return 0; }

	int file_size = err;
	int file_size_4 = (file_size - 4) / 4;
	if ((file_size - 4) % 4 > 0) file_size_4++;

	int dbs_format = 0;
	fread(&dbs_format, 4, 1, dbs_f);

	if (dbs_format != 0 && dbs_format != 1 && file_size_4 < 3) { wprintf_api(2, L"Invalid DBS file\n"); fclose(dbs_f); return 0; }

	unsigned char *dbs_data = (unsigned char*) malloc(file_size_4 * 4);
	if (!dbs_data) {  wprintf_api(2, L"malloc error\n"); fclose(dbs_f); return 0; }

	fread(dbs_data, 4, file_size_4, dbs_f);
	fclose(dbs_f);

	int slen = 0;
	if (dir_name[0] != '\0')
	{
		slen = wcslen(dir_name);
		dir_name[slen] = '\\';
		dir_name[slen+1] = '\0';
	}

	slen = wcslen(file_name);
	file_name[slen-4] = '\0';
	
	wchar_t out_file_name_xor1[MAX_PATH + 1];
	wchar_t out_file_name_raw[MAX_PATH + 1];
	wchar_t out_file_name_xor2[MAX_PATH + 1];
	wchar_t out_file_name[MAX_PATH + 1];
	slen = swprintf(out_file_name_raw, MAX_PATH, L"%s%s-raw.raw", dir_name, file_name);
	if (slen == -1) out_file_name_raw[0] = L'\0';
	slen = swprintf(out_file_name_xor1, MAX_PATH, L"%s%s-xor1.raw", dir_name, file_name);
	if (slen == -1) out_file_name_xor1[0] = L'\0';
	slen = swprintf(out_file_name_xor2, MAX_PATH, L"%s%s-xor2.raw", dir_name, file_name);
	if (slen == -1) out_file_name_xor2[0] = L'\0';
	slen = swprintf(out_file_name, MAX_PATH, L"%s%s.csv", dir_name, file_name);
	if (slen == -1) { wprintf_api(2, L"path is very long\r\n"); free(dbs_data); return 0; }

	unsigned int *dbs_data_4 = (unsigned int *) dbs_data;
	for (int i = 0; i < file_size_4; i++) dbs_data_4[i] = dbs_data_4[i] ^ xor_mask1;

	if (verbose)
	{
		if (out_file_name_xor1[0] != L'\0')
		{
			FILE *out_f;
			if (_wfopen_s(&out_f, out_file_name_xor1, L"wb") == 0)
			{
				fwrite(dbs_data, 4, file_size_4, out_f);
				fclose(out_f);
			}
		}
	}

	int compress_data_size = dbs_data_4[0];
	int uncompress_data_size = dbs_data_4[1];

	if (compress_data_size > file_size_4 * 4 || uncompress_data_size < compress_data_size || (uncompress_data_size % 4) != 0) {  wprintf_api(2, L"Corrupt DBS file\n"); free(dbs_data); return 0; }

	unsigned char *uncompress_data = (unsigned char*) malloc(uncompress_data_size);
	if (!uncompress_data) { wprintf_api(2, L"malloc error\n"); free(dbs_data); return 0; }

	if (!va_decompress_g00_1(dbs_data + 8, compress_data_size, uncompress_data, uncompress_data_size))
	{
		wprintf_api(2, L"decompress error\n");
		free(dbs_data);
		free(uncompress_data);
		return 0;
	}

	free(dbs_data);

	if (verbose)
	{
		if (out_file_name_raw[0] != L'\0')
		{
			FILE *out_f;
			if (_wfopen_s(&out_f, out_file_name_raw, L"wb") == 0)
			{
				fwrite(uncompress_data, 1, uncompress_data_size, out_f);
				fclose(out_f);
			}
		}
	}

	unsigned int *uncompress_data_4 = (unsigned int *) uncompress_data;
	char flag1 = 0;
    char flag2 = 0;
    for (int i = 0; i < uncompress_data_size / 4; i++)
	{
        uncompress_data_4[i] = uncompress_data_4[i] ^ xor_mask2[(flag1 * 5) + (flag2 % 5)];
        flag2++;
		if (flag2 >= 16) { flag1++; flag2 = 0; }
        if (flag1 >= 5) flag1 = 0;
    }

	if (verbose)
	{
		if (out_file_name_xor2[0] != L'\0')
		{
			FILE *out_f;
			if (_wfopen_s(&out_f, out_file_name_xor2, L"wb") == 0)
			{
				fwrite(uncompress_data, 1, uncompress_data_size, out_f);
				fclose(out_f);
			}
		}
	}

	DBSHead *dbs_head = (DBSHead*) uncompress_data;

	if (dbs_head->data_size > uncompress_data_size) {  wprintf_api(2, L"Corrupt DBS file\n"); free(uncompress_data); return 0; }

	wchar_t buf[MAX_PATH];
	buf[0] = L'\0';
	wchar_t buf1[MAX_PATH];
	wchar_t int_str[MAX_PATH];
	wcscat_s(buf, MAX_PATH, L"Archive type: dbs database\r\n");
	hum_size(int_str, file_size);
	wsprintf(buf1, L"  Archive size: %s (%u B)\r\n", int_str, file_size);
	wcscat_s(buf, MAX_PATH, buf1);
	hum_size(int_str, dbs_head->data_size);
	wsprintf(buf1, L"  Data size:    %s (%u B)\r\n", int_str, dbs_head->data_size);
	wcscat_s(buf, MAX_PATH, buf1);
	wsprintf(buf1, L"  Rows:         %u\r\n", dbs_head->row_cnt);
	wcscat_s(buf, MAX_PATH, buf1);
	wsprintf(buf1, L"  Columns:      %u\r\n", dbs_head->col_cnt);
	wcscat_s(buf, MAX_PATH, buf1);
	wprintf_api(0, L"%s", buf);


	int dbs_row_cnt = dbs_head->row_cnt;
	int dbs_col_cnt = dbs_head->col_cnt;

	if (dbs_row_cnt <= 0 || dbs_col_cnt <= 0) {  wprintf_api(0, L"  Empty database\n"); free(uncompress_data); return 1; }

	DBSColInfo *dbs_col_info = (DBSColInfo*) (uncompress_data + dbs_head->col_off);
	int *dbs_row_ids = (int*) (uncompress_data + dbs_head->row_off);

	FILE *out_f;
	if (_wfopen_s(&out_f, out_file_name, L"wb") != 0)
	{
		wprintf_api(2, L"could not write file: %s\r\n", out_file_name);
		free(uncompress_data);
		return 0;
	}

	char *out_buf = (char*) malloc(uncompress_data_size);
	out_buf[0] = '\0';
	char *out_buf_tmp = (char*) malloc(uncompress_data_size);
	out_buf_tmp[0] = '\0';

	//strcpy(out_buf, "dbs");
	if (dbs_format == 0) WideCharToMultiByte(932, NULL, file_name, -1, out_buf, MAX_PATH, NULL, NULL);
	else if (dbs_format == 1) WideCharToMultiByte(CP_UTF8, NULL, file_name, -1, out_buf, MAX_PATH, NULL, NULL);
	char * dim_ptr = strrchr(out_buf, '\\');
	if (dim_ptr) fwrite(dim_ptr + 1, 1, strlen(dim_ptr + 1), out_f);
	else fwrite(out_buf, 1, strlen(out_buf), out_f);
	for (int i = 0; i < dbs_col_cnt; i++)
	{
		strcpy(out_buf, ",");
		fwrite(out_buf, 1, strlen(out_buf), out_f);
	}
	strcpy(out_buf, "\r\n");
	fwrite(out_buf, 1, strlen(out_buf), out_f);

	strcpy(out_buf, "\r\n#DATANO");
	fwrite(out_buf, 1, strlen(out_buf), out_f);
	for (int i = 0; i < dbs_col_cnt; i++)
	{
		sprintf(out_buf, ",%u", dbs_col_info[i].col_id);
		fwrite(out_buf, 1, strlen(out_buf), out_f);
	}
	strcpy(out_buf, "\r\n#DATATYPE");
	fwrite(out_buf, 1, strlen(out_buf), out_f);
	for (int i = 0; i < dbs_col_cnt; i++)
	{
		if (dbs_col_info[i].data_type == 0x53) strcpy(out_buf, ",S");
		else if (dbs_col_info[i].data_type == 0x56) strcpy(out_buf, ",V");
		else sprintf(out_buf, ",0x%X", dbs_col_info[i].data_type);
		fwrite(out_buf, 1, strlen(out_buf), out_f);
	}
	strcpy(out_buf, "\r\n\r\n");
	fwrite(out_buf, 1, strlen(out_buf), out_f);

	for (int i = 0; i < dbs_row_cnt; i++)
	{
		sprintf(out_buf, "%u", dbs_row_ids[i]);
		fwrite(out_buf, 1, strlen(out_buf), out_f);
		for (int j = 0; j < dbs_col_cnt; j++)
		{
			out_buf[0] = '\0';
			if (dbs_col_info[j].data_type == 0x53)
			{
				void *str_ptr = uncompress_data + dbs_head->sdata_off + *(int*)(uncompress_data + (dbs_head->idata_off + i * dbs_col_cnt * 4 + j * 4));
				if (dbs_format == 0) sprintf(out_buf, ",\"%s\"", (char*) str_ptr);
				else if (dbs_format == 1)
				{
					WideCharToMultiByte(CP_UTF8, NULL, (wchar_t*) str_ptr, -1, out_buf_tmp, MAX_PATH, NULL, NULL);
					sprintf(out_buf, ",\"%s\"", out_buf_tmp);
				}
			}
			else if (dbs_col_info[j].data_type == 0x56) sprintf(out_buf, ",%u", *(int*)(uncompress_data + (dbs_head->idata_off + i * dbs_col_cnt * 4 + j * 4)));
			fwrite(out_buf, 1, strlen(out_buf), out_f);
		}
		strcpy(out_buf, "\r\n");
		fwrite(out_buf, 1, strlen(out_buf), out_f);
	}

	fclose(out_f);

	free(uncompress_data);
	free(out_buf);
	free(out_buf_tmp);

	return 1;
}