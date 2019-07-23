/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

#include "common.h"

int wprintf_api(int mode, wchar_t *string, ...)
{
	//wchar_t *buf = new wchar_t[2048];
	//wchar_t *buf1 = new wchar_t[2048];
	//char *utf8_buf = new char[2048 * 2];
	wchar_t buf[2048];
	wchar_t buf1[2048];
	char utf8_buf[2048 * 2];
	va_list ap;
	va_start(ap, string);
	_vsnwprintf_s(buf1, 2048, -1, string, ap);
	//wsprintf(buf, string, ap);
	va_end(ap);
	wcscpy_s(buf, 2048, L"  ");
	if (mode == 1) wcscat_s(buf, 2048, L"Warning: ");
	if (mode == 2) wcscat_s(buf, 2048, L"Error: ");
	wcscat_s(buf, 2048, buf1);
	int utf8_buf_len = WideCharToMultiByte(CP_OEMCP, NULL, buf, -1, utf8_buf, 2048 * 2, NULL, NULL);
	/*HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwCharsWritten;
	//WriteConsoleW(hStdOut, buf, wcslen(buf), &dwCharsWritten, NULL);
	WriteFile(hStdOut, utf8_buf, utf8_buf_len - 1, &dwCharsWritten, NULL);*/
	printf("%s", utf8_buf);
	//delete[] buf;
	//delete[] buf1;
	//delete[] utf8_buf;
	return utf8_buf_len;
}

int open_bin_to_read(FILE **open_f, wchar_t *file_name)
{
	errno_t err;
	err = _wfopen_s(open_f, file_name, L"rb");
	if (err != 0) return -1;
	fseek(*open_f, 0, SEEK_END);
	int file_size = ftell(*open_f);
	fseek(*open_f, 0, SEEK_SET);
	return file_size;
}

int hum_size(wchar_t *buf, int size)
{
	if (size < 0) { wcscpy(buf, L"arg error"); return 0; }
	double sizef = (double) size;
	wchar_t *sizes[] = {L"B", L"kB", L"MB", L"GB" , L"TB"};
	int i = 0;
	for (i = 0; sizef > 1000 && i < 5; i++) { sizef /= 1024; }
	swprintf(buf, L"%0.2f %s", sizef, sizes[i]);
	return i;
}

wchar_t* get_string(wchar_t *out_str, size_t out_size, wchar_t *&in_str)
{
	if (*in_str == L'\0') return 0;
	wchar_t *rv = out_str;
	out_size -= 2;
	while (*in_str != L'\0' && *in_str != L'\n' && out_size > 0) { *out_str++ = *in_str++; out_size--; }
	if (*in_str == L'\n') *out_str++ = *in_str++;
	*out_str = L'\0';
	return rv;
}

int get_int_val(wchar_t *findstr, wchar_t *srcstr, int type)
{
	int elements = 0;
	wchar_t buf[64];
	wchar_t *buf_prt = buf;
	//wcsncpy_s(buf, 5, finstr, -1);
	while (is_space_w(*srcstr)) srcstr++;
	while (!is_space_w(*srcstr) && *srcstr != L'=') *buf_prt++ = *srcstr++;
	*buf_prt++ = L'\0';
	//wprintf_api(0, L"%s\n", buf);
	if (wcscmp(buf, findstr) == 0)
	{
		//finstr += 4;
		while (is_space_w(*srcstr)) srcstr++;
		if (*srcstr == L'=')
		{
			srcstr++;
			while (is_space_w(*srcstr)) srcstr++;
			wchar_t *end_ptr;
			elements = wcstol(srcstr, &end_ptr, type);
			if (errno == 34 || *end_ptr != 0) elements = 0;
		}
	}
	//printf("%d\n", elements);
	return elements;
}

wchar_t* get_str_val(wchar_t *findstr, wchar_t *srcstr)
{
	if (findstr == NULL || srcstr == NULL) return NULL;
	wchar_t *out_str = NULL;
	//wchar_t *out_prt = out_str;
	wchar_t buf[MAX_PATH];
	wchar_t *buf_prt = buf;
	//wcsncpy_s(buf, 5, finstr, -1);
	while (is_space_w(*srcstr)) srcstr++;
	while (!is_space_w(*srcstr) && *srcstr != L'=') *buf_prt++ = *srcstr++;
	*buf_prt = L'\0';
	//wprintf_api(0, L"%s\n", buf);
	if (wcscmp(buf, findstr) == 0)
	{
		//finstr += 4;
		while (is_space_w(*srcstr)) srcstr++;
		if (*srcstr == L'=')
		{
			srcstr++;
			while (is_space_w(*srcstr)) srcstr++;
			//if(*srcstr == L'"') srcstr++;
			//while (*srcstr) *out_prt++ = *srcstr++;
			out_str = srcstr;
			//if(*(out_prt - 1) == L'"') out_prt--;
			//*out_prt = L'\0';
		}
	}
	//printf("%d\n", elements);
	return out_str;
}

char* strtrim(char *str)
{
	if (str == NULL) return NULL;
	while (is_space_s(*str)) str++;
	int l = strlen(str);
	while (is_space_s(str[l - 1])) str[--l] = '\0';
	return str;
}

wchar_t* wcstrim(wchar_t *str)
{
	if (str == NULL) return NULL;
	while (is_space_w(*str)) str++;
	int l = wcslen(str);
	while (is_space_w(str[l - 1])) str[--l] = L'\0';
	return str;
}

// single-line comment cut
wchar_t* wcsslccut(wchar_t *str)
{
	if (str == NULL) return NULL;
	wchar_t *retstr = str;
	while (*str != L'\0')
	{
		if (*str == L'/' && *(str+1) == L'/') { *str = L'\0'; break; }
		str++;
	}
	return retstr;
}


int get_files_in_dir(wchar_t *dir_name, wchar_t *file_mask, /*wchar_t **&*/std::vector<std::wstring> *file_list)
{
	errno_t err = 0;

	WIN32_FIND_DATAW FindFileData;
    HANDLE hFind;

	wchar_t mask[MAX_PATH];

	wsprintf(mask, L"%s\\%s", dir_name, file_mask);

	DWORD dwFileCount = 0;

	hFind = FindFirstFileW(mask, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	//std::vector<std::wstring> temp_file_list;
	//wchar_t *temp_file_name;

	//temp_file_name = new wchar_t[MAX_PATH];
	//wcscpy_s(temp_file_name, MAX_PATH, FindFileData.cFileName);
	//temp_file_list.push_back(temp_file_name);
	file_list->push_back(FindFileData.cFileName);
	dwFileCount++;
	//wprintf_api(0, L"0x%X  0x%X 0x%X\n", FindFileData.cFileName, temp_file_list[0].c_str(), &temp_file_list[0]);

	while(FindNextFile(hFind, &FindFileData))
	{
		/*temp_file_name = new wchar_t[MAX_PATH];
		wcscpy_s(temp_file_name, MAX_PATH, FindFileData.cFileName);
		temp_file_list.push_back(temp_file_name);*/
		file_list->push_back(FindFileData.cFileName);
		//wprintf_api(0, L"0x%X  0x%X 0x%X\n", FindFileData.cFileName, temp_file_list[0].c_str(), &temp_file_list[0]);
		dwFileCount++;
	}
	FindClose(hFind);

	if (file_list->size() != dwFileCount) { file_list->clear(); return 0; }

	//wprintf_api(0, L"%s\n", temp_file_list[1].c_str());

	return dwFileCount;
}

wchar_t * iconv_string_8to16(const char *instr, int instrlen)
{
	if (instr == NULL || instrlen == 0) return NULL;

	SetLastError(ERROR_SUCCESS);
	int outstrlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, instr, instrlen, NULL, 0);
	//printf("%d\n", outstrlen);

	if (outstrlen == 0 || GetLastError() == ERROR_NO_UNICODE_TRANSLATION) return NULL;

	wchar_t *ret = (wchar_t*) malloc(outstrlen * sizeof(wchar_t));

	SetLastError(ERROR_SUCCESS);
	outstrlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, instr, instrlen, ret, outstrlen);
	//printf("%d\n", outstrlen);
	if (outstrlen == 0 || GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
	{
		free(ret);
		return NULL;
	}
	//printf("%d\n", outstrlen);
	return ret;
}