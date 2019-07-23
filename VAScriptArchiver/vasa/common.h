/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

//#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN2K
//#endif

#include <windows.h>
//#include <strsafe.h>
//#include <unistd.h>
#include <stdio.h>
//#include <tchar.h>
#include <vector>

#pragma pack(push, 1)
struct FileTable
{
	int offset;
	int size;
};
#pragma pack(pop)

extern bool verbose;
extern int windowsize[7];

extern std::vector<wchar_t*> GameIDs;
extern std::vector<char> GameEngine;
extern std::vector<wchar_t*> GameNemes;
extern std::vector<wchar_t*> GameVendors;
extern std::vector<unsigned char*> XorKeys;

//bool verbose = false;
//int windowsize[6] = { 0, 256, 512, 1024, 2048, 4096 };

#define is_space_s(c) (c == '\r' || c == '\n' || c == ' ' || c == '\t')
#define is_space_w(c) (c == L'\r' || c == L'\n' || c == L' ' || c == L'\t')

int open_bin_to_read(FILE **open_f, wchar_t *file_name);
int hum_size(wchar_t *buf, int size);
wchar_t* get_string(wchar_t *out_str, size_t out_size, wchar_t *&in_str);
int get_int_val(wchar_t *findstr, wchar_t *finstr, int type);
wchar_t* get_str_val(wchar_t *findstr, wchar_t *srcstr);
char* strtrim(char *str);
wchar_t* wcstrim(wchar_t *str);
wchar_t* wcsslccut(wchar_t *str);
int get_files_in_dir(wchar_t *dir_name, wchar_t *file_mask, /*wchar_t **&*/std::vector<std::wstring> *file_list);
int wprintf_api(int mode, wchar_t *string, ...);

wchar_t * iconv_string_8to16(const char *instr, int instrlen);