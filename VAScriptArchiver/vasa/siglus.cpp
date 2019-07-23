/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

#include "siglus.h"
#include "common.h"
#include "encrypt.h"
#include "compression.h"
#include "siglustextdump.h"

#include "xml\sxmlc.h"

//// ---- Scene.pck ----

#pragma pack(push, 1)
struct VarStruct
{
	unsigned int var1;
	unsigned int var2;
};

/*
struct VarStruct
{
	unsigned int type;
	unsigned int size;
};

struct FuncStruct
{
	unsigned int fileno;
	unsigned int offset;
};
*/

struct ScenePCKhead
{
	int HeadSize;
	// Global variables
	int VarTableVarOffset;    // Смещение данных
	int VarTableVarCount;     // Кол-во элементов
	int VarTableNameTOffset;  // Смещение таблицы имён
	int VarTableNameTCount;   // Кол-во элементов
	int VarTableNamesOffset;  // Смещение имён переменных
	int VarTableNamesCount;   // Кол-во элементов
	// Global functions
	int FuncTableVarOffset;    // Смещение данных
	int FuncTableVarCount;     // Кол-во элементов
	int FuncTableNameTOffset;  // Смещение таблицы имён
	int FuncTableNameTCount;   // Кол-во элементов
	int FuncTableNamesOffset;  // Смещение имён функций
	int FuncTableNamesCount;   // Кол-во элементов
	// Files table
	int FileTableNameTOffset;
	int FileTableNameTCount;
	int FileTableNamesOffset;
	int FileTableNamesCount;
	int FileTableFilesTOffset;
	int FileTableFilesTCount;
	int FileTableDataOffset;
	int FileTableDataCount;
	int Encrypt1;               // шифрование 1 - 2 раунда, 0 - 1 раунд
	int Encrypt2;				// шифрование 0 - вроде, вообще без шифрования и сжатия?
};
#pragma pack(pop)

void clear_pck_info(ScenePCKhead scene_header, VarStruct *var1, VarStruct *var2, wchar_t **var1_name, wchar_t **var2_name, FileTable *file_table, wchar_t **file_name_str);
int write_names(FILE *pck_f, int startoffset, int *NameTOffset, int *NamesOffset, int NamesCount, wchar_t **name_str);
int write_vars(FILE *pck_f, int startsection, int *VarTableVarOffset, int *VarTableNameTOffset, int *VarTableNamesOffset, int VarTableVarCount, VarStruct *var, wchar_t **var_name);



int read_chek_scene_head(FILE *scene_pck_f, int file_size, ScenePCKhead *scene_header)
{
	// 
	int header_size;
	fread(&header_size, 4, 1, scene_pck_f);
	if (header_size >= file_size) return 0;
	if (header_size != 92) return 0;
	fseek(scene_pck_f, 0, SEEK_SET);

	fread(scene_header, sizeof(ScenePCKhead), 1, scene_pck_f);

	if (scene_header->VarTableVarOffset >= file_size
		|| scene_header->VarTableNameTOffset >= file_size
		|| scene_header->VarTableNamesOffset >= file_size
		|| scene_header->FuncTableVarOffset >= file_size
		|| scene_header->FuncTableNameTOffset >= file_size
		|| scene_header->FuncTableNamesOffset >= file_size
		|| scene_header->FileTableNameTOffset >= file_size
		|| scene_header->FileTableNamesOffset >= file_size
		|| scene_header->FileTableFilesTOffset >= file_size
		|| scene_header->FileTableDataOffset >= file_size
		) return 0;

	if (scene_header->VarTableVarCount != scene_header->VarTableNameTCount
		|| scene_header->VarTableNameTCount != scene_header->VarTableNamesCount) return 0;

	if (scene_header->FuncTableVarCount != scene_header->FuncTableNameTCount
		|| scene_header->FuncTableNameTCount != scene_header->FuncTableNamesCount) return 0;


	return scene_header->HeadSize;
}

int load_scene_head(FILE* pck_f, ScenePCKhead *scene_header, int *ss_comp_size, wchar_t **&var1_name, VarStruct *&var1, wchar_t **&var2_name, VarStruct *&var2, wchar_t **&file_name_str, FileTable *&file_data_table)
{
	var1 = new VarStruct[scene_header->VarTableVarCount];
	fseek(pck_f, scene_header->VarTableVarOffset, SEEK_SET);
	fread(var1, sizeof(VarStruct), scene_header->VarTableVarCount, pck_f);

	FileTable *var1_name_table = new FileTable[scene_header->VarTableNameTCount];
	fseek(pck_f, scene_header->VarTableNameTOffset, SEEK_SET);
	fread(var1_name_table, sizeof(FileTable), scene_header->VarTableNameTCount, pck_f);

	var1_name = new wchar_t*[scene_header->VarTableNameTCount];
	memset(var1_name, 0, scene_header->VarTableNameTCount * sizeof(wchar_t*));
	for (int i = 0; i < scene_header->VarTableNameTCount; i++)
	{
		var1_name[i] = new wchar_t[var1_name_table[i].size + 1];
		memset(var1_name[i], 0, (var1_name_table[i].size + 1) * 2);
		fseek(pck_f, scene_header->VarTableNamesOffset + (var1_name_table[i].offset * 2), SEEK_SET);
		fread(var1_name[i], 2, var1_name_table[i].size, pck_f);

	}
	delete[] var1_name_table;


	var2 = new VarStruct[scene_header->FuncTableVarCount];
	fseek(pck_f, scene_header->FuncTableVarOffset, SEEK_SET);
	fread(var2, sizeof(VarStruct), scene_header->FuncTableVarCount, pck_f);

	FileTable *var2_name_table = new FileTable[scene_header->FuncTableNameTCount];
	fseek(pck_f, scene_header->FuncTableNameTOffset, SEEK_SET);
	fread(var2_name_table, sizeof(FileTable), scene_header->FuncTableNameTCount, pck_f);

	var2_name = new wchar_t*[scene_header->FuncTableNamesCount];
	memset(var2_name, 0, scene_header->FuncTableNameTCount * sizeof(wchar_t*));
	for (int i = 0; i < scene_header->FuncTableNamesCount; i++)
	{
		var2_name[i] = new wchar_t[var2_name_table[i].size + 1];
		memset(var2_name[i], 0, (var2_name_table[i].size + 1) * 2);
		fseek(pck_f, scene_header->FuncTableNamesOffset + (var2_name_table[i].offset * 2), SEEK_SET);
		fread(var2_name[i], 2, var2_name_table[i].size, pck_f);
	}
	delete[] var2_name_table;


	file_data_table = new FileTable[scene_header->FileTableFilesTCount];
	fseek(pck_f, scene_header->FileTableFilesTOffset, SEEK_SET);
	fread(file_data_table, sizeof(FileTable), scene_header->FileTableFilesTCount, pck_f);

	file_name_str = new wchar_t*[scene_header->FileTableNameTCount];
	memset(file_name_str, 0, scene_header->FileTableNameTCount * sizeof(wchar_t*));
	for (int i = 0; i < scene_header->FileTableNameTCount; i++)
	{
		file_name_str[i] = new wchar_t[MAX_PATH];
		memset(file_name_str[i], 0, MAX_PATH * 2);
	}
		
	FileTable *file_name_table = new FileTable[scene_header->FileTableFilesTCount];
	fseek(pck_f, scene_header->FileTableNameTOffset, SEEK_SET);
	fread(file_name_table, sizeof(FileTable), scene_header->FileTableNameTCount, pck_f);
	
	for (int i = 0; i < scene_header->FileTableFilesTCount; i++)
	{
		*ss_comp_size = *ss_comp_size + file_data_table[i].size;
		fseek(pck_f, scene_header->FileTableNamesOffset + (file_name_table[i].offset * 2), SEEK_SET);
		fread(file_name_str[i], 2, file_name_table[i].size, pck_f);
	}
	delete[] file_name_table;

	return *ss_comp_size;
}


int print_se_arc_info(wchar_t *file_name, int mode)
{
	errno_t err;

	FILE* pck_f;
	err = open_bin_to_read(&pck_f, file_name);
	if (err <= 0) { wprintf_api(2, L"cannot open file: %s.\r\n", file_name); return 1; }

	int file_size = err;

	ScenePCKhead scene_header;
	err = read_chek_scene_head(pck_f, file_size, &scene_header);
	if (err == 0) { wprintf_api(2, L"invalid SiglusEngine archive.\r\n"); fclose(pck_f); return 1; }

	int ss_comp_size = 0;
	wchar_t **var1_name = NULL, **var2_name = NULL, **file_name_str = NULL;
	FileTable *file_table = NULL;
	//FileTable *file_table = new FileTable[scene_header.FileTableFilesTCount];
	//wchar_t **file_name_str = new wchar_t*[scene_header.FileTableNameTCount];
	//int *var1 = new int[scene_header.VarTableVarCount];
	//wchar_t **var1_name = new wchar_t*[scene_header.VarTableNamesCount];
	//int *var2 = new int[scene_header.FuncTableVarCount];
	//wchar_t **var2_name = new wchar_t*[scene_header.FuncTableNamesCount];
	VarStruct *var1 = NULL, *var2 = NULL;
	load_scene_head(pck_f, &scene_header, &ss_comp_size, var1_name, var1, var2_name, var2, file_name_str, file_table);

	wchar_t *buf = new wchar_t[32768];
	ZeroMemory(buf, 32768*2);
	wchar_t buf1[MAX_PATH];
	wchar_t int_str[MAX_PATH];
	wcscat_s(buf, 32768, L"Archive type: SiglusEngine Scene.pck\r\n");
	hum_size(int_str, file_size);
	wsprintf(buf1, L"  Archive size: %s (%u B)\r\n", int_str, file_size);
	wcscat_s(buf, 32768, buf1);
	wsprintf(buf1, L"  Files count:  %u\r\n", scene_header.FileTableNameTCount);
	wcscat_s(buf, 32768, buf1);
	hum_size(int_str, ss_comp_size);
	wsprintf(buf1, L"  Files size:   %s (%u B)\r\n", int_str, ss_comp_size);
	wcscat_s(buf, 32768, buf1);
	wsprintf(buf1, L"  Variables:    %u\r\n", scene_header.VarTableVarCount);
	wcscat_s(buf, 32768, buf1);
	wsprintf(buf1, L"  Functions:    %u\r\n", scene_header.FuncTableVarCount);
	wcscat_s(buf, 32768, buf1);
	wcscat_s(buf, 32768, L"  Encrypted:    ");
	if (scene_header.Encrypt1 == 0) wcscat_s(buf, 32768, L"No\r\n");
	else wcscat_s(buf, 32768, L"Yes\r\n");

	wprintf_api(0, L"%s", buf);

	if (mode == 7)
	{
		wsprintf(buf1, L"%s.txt", file_name);
		wprintf_api(0, L"\r\n  Write file list to %s\r\n", buf1);
		FILE *txt_f;
		err = _wfopen_s(&txt_f, buf1, L"wb");
		if (err == 0)
		{
			//fwrite(file_name, 2, wcslen(file_name), txt_f);
			//fwrite(L"\r\n  ", 2, 4, txt_f);
			//fwrite(buf, 2, wcslen(buf), txt_f);
			//buf[0] = L'\0';
			char *utf8_buf = new char[32768 * 2];
			ZeroMemory(utf8_buf, 32768*2);
/*
			buf[0] = L'\0';
			wcscat_s(buf, 32768, L"Archive type: SiglusEngine Scene.pck\r\n");
			hum_size(int_str, file_size);
			wsprintf(buf1, L"  Archive size: %s (%u B)\r\n", int_str, file_size);
			wcscat_s(buf, 32768, buf1);
			wsprintf(buf1, L"  Files count:  %u\r\n", scene_header.FileTableNameTCount);
			wcscat_s(buf, 32768, buf1);
			hum_size(int_str, ss_comp_size);
			wsprintf(buf1, L"  Files size:   %s (%u B)\r\n", int_str, ss_comp_size);
			wcscat_s(buf, 32768, buf1);
			wsprintf(buf1, L"  VarTable:    %u\r\n", scene_header.VarTableVarCount);
			wcscat_s(buf, 32768, buf1);
			wsprintf(buf1, L"  FuncTable:    %u\r\n", scene_header.FuncTableVarCount);
			wcscat_s(buf, 32768, buf1);
			wcscat_s(buf, 32768, L"  Encrypted:    ");
			if (scene_header.Encrypt1 == 0) wcscat_s(buf, 32768, L"No\r\n");
			else wcscat_s(buf, 32768, L"Yes\r\n");
*/
			WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
			fwrite(utf8_buf, 1, strlen(utf8_buf), txt_f);

			buf[0] = L'\0';
			wcscat_s(buf, 32768, L"\r\nGlobal Variables\r\n");
			for (int i = 0; i < scene_header.VarTableVarCount; i++)
			{
				//wsprintf(buf1, L"  0x%08x  0x%08x  %s\r\n", var1[i].var1, var1[i].var2, var1_name[i]);
				if (var1[i].var1 == 0xA) wsprintf(buf1, L"  int %s\r\n", var1_name[i]);
				else if (var1[i].var1 == 0xB) wsprintf(buf1, L"  int %s[%d]\r\n", var1_name[i], var1[i].var2);
				else if (var1[i].var1 == 0x14) wsprintf(buf1, L"  str %s\r\n", var1_name[i]);
				else if (var1[i].var1 == 0x15) wsprintf(buf1, L"  str %s[%d]\r\n", var1_name[i], var1[i].var2);
				wcscat_s(buf, 32768, buf1);
			}
			//wcscat_s(buf, 32768, L"VarTable end\r\n");
			wcscat_s(buf, 32768, L"end\r\n");
			//fwrite(buf, 2, wcslen(buf), txt_f);
			WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
			fwrite(utf8_buf, 1, strlen(utf8_buf), txt_f);

			buf[0] = L'\0';
			wcscat_s(buf, 32768, L"\r\nGlobal Functions\r\n  File        Offset        Name\r\n");
			for (int i = 0; i < scene_header.FuncTableVarCount; i++)
			{
				//wsprintf(buf1, L"  0x%08x  0x%08x  %s\r\n", var2[i].var1, var2[i].var2, var2_name[i]);
				wsprintf(buf1, L"  %s 0x%08x %s\r\n", file_name_str[var2[i].var1], var2[i].var2, var2_name[i]);
				wcscat_s(buf, 32768, buf1);
			}
			wcscat_s(buf, 32768, L"end\r\n");
			//fwrite(buf, 2, wcslen(buf), txt_f);
			WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
			fwrite(utf8_buf, 1, strlen(utf8_buf), txt_f);

			buf[0] = L'\0';
			wcscat_s(buf, 32768, L"\r\nFile Table start\r\n  Size                    Name\r\n");
			for (int i = 0; i < scene_header.FileTableFilesTCount; i++)
			{
				hum_size(int_str, file_table[i].size);
				//wsprintf(buf1, L"  %9s (%8u B)  %s\r\n", int_str, file_table[i].size, file_name_str[i]);
				if (verbose) wsprintf(buf1, L"  0x%08X %9s (%8u B)  %s\r\n", file_table[i].offset + scene_header.FileTableDataOffset, int_str, file_table[i].size, file_name_str[i]);
				else wsprintf(buf1, L"  %9s (%8u B)  %s\r\n", int_str, file_table[i].size, file_name_str[i]);
				wcscat_s(buf, 32768, buf1);
			}
			wcscat_s(buf, 32768, L"File Table end\r\n");
			//fwrite(buf, 2, wcslen(buf), txt_f);
			WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
			fwrite(utf8_buf, 1, strlen(utf8_buf), txt_f);

			delete[] utf8_buf;

			fclose(txt_f);
		}
		else wprintf_api(2, L"no write to file.\r\n");

	}
	delete[] buf;

/*
	delete[] var1;
	while (scene_header.VarTableNamesCount--) delete[] var1_name[scene_header.VarTableNamesCount];
	delete[] var1_name;
	delete[] var2;
	while (scene_header.FuncTableNamesCount--) delete[] var2_name[scene_header.FuncTableNamesCount];
	delete[] var2_name;
	delete[] file_table;
	while (scene_header.FileTableNameTCount--) delete[] file_name_str[scene_header.FileTableNameTCount];
	delete[] file_name_str;	
*/
	
	clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
	fclose(pck_f);

	return 1;
}


int extract_se_arc(wchar_t *file_name, wchar_t *dir_name, int game_id, bool duplicate_strings)
{
	errno_t err;

	//if ((GetFileAttributesW(file_name) & FILE_ATTRIBUTE_DIRECTORY) == 1) { wprintf_api(2, L"could not open file: %s.\r\n", file_name); return 1; }

	FILE* pck_f;
	err = open_bin_to_read(&pck_f, file_name);
	if (err <= 0) { wprintf_api(2, L"could not open file: %s.\r\n", file_name); return 1; }

	int file_size = err;

	ScenePCKhead scene_header;
	err = read_chek_scene_head(pck_f, file_size, &scene_header);
	if (err == 0) { wprintf_api(2, L"invalid SiglusEngine archive.\r\n"); fclose(pck_f); return 1; }

	wchar_t *buf = new wchar_t[32768];
	buf[0] = L'\0';
	wchar_t buf1[MAX_PATH];
	wchar_t int_str[MAX_PATH];
	wcscat_s(buf, 32767, L"Archive type: SiglusEngine Scene.pck\r\n");
	hum_size(int_str, file_size);
	wsprintf(buf1, L"  Archive size: %s (%u B)\r\n", int_str, file_size);
	wcscat_s(buf, 32767, buf1);
	wsprintf(buf1, L"  Files:        %u\r\n", scene_header.FileTableNameTCount);
	wcscat_s(buf, 32767, buf1);
	//hum_size(int_str, ss_comp_size);
	//wsprintf(buf1, L"  Files size:   %s (%u B)\r\n", int_str, ss_comp_size);
	//wcscat_s(buf, 32767, buf1);
	wsprintf(buf1, L"  Variables:    %u\r\n", scene_header.VarTableVarCount);
	wcscat_s(buf, 32767, buf1);
	wsprintf(buf1, L"  Functions:    %u\r\n", scene_header.FuncTableVarCount);
	wcscat_s(buf, 32767, buf1);
	wcscat_s(buf, 32767, L"  Encrypted:    ");
	if (scene_header.Encrypt1 == 0) wcscat_s(buf, 32767, L"No\r\n");
	else wcscat_s(buf, 32767, L"Yes\r\n");
	wprintf_api(0, L"%s", buf);
	delete[] buf;

	if (scene_header.Encrypt1 && !game_id)
	{
		wprintf_api(2, L"Скрипты зашифрованы, используйте ключ '-G' (--game) для выбора игры.\r\n  Список поддерживаемых игр можно получить запустив программу\r\n  с ключом '--gamelist'.\r\n");
		return 1;
	}

	int ss_comp_size = 0;
	wchar_t **var1_name = NULL, **var2_name = NULL, **file_name_str = NULL;
	FileTable *file_table = NULL;
	//FileTable *file_table = new FileTable[scene_header.FileTableFilesTCount];
	//wchar_t **file_name_str = new wchar_t*[scene_header.FileTableNameTCount];
	//int *var1 = new int[scene_header.VarTableVarCount];
	//wchar_t **var1_name = new wchar_t*[scene_header.VarTableNamesCount];
	//int *var2 = new int[scene_header.FuncTableVarCount];
	//wchar_t **var2_name = new wchar_t*[scene_header.FuncTableNamesCount];
	VarStruct *var1 = 0, *var2 = 0;
	load_scene_head(pck_f, &scene_header, &ss_comp_size, var1_name, var1, var2_name, var2, file_name_str, file_table);

	if (dir_name == NULL || dir_name[0] == L'\0')
	{
		int len = wcslen(file_name);
		wcsncpy_s(dir_name, len-3, file_name, -1);
	}

	unsigned int unpacked_files = 0;
	wprintf_api(0, L"\r\n  Extracting...\r\n");

	err = CreateDirectoryW(dir_name, NULL);
	if (err != 0 || (err == 0 && GetLastError() == ERROR_ALREADY_EXISTS))
	{
		err = 0;
//#if 0
		for (int i = 0; i < scene_header.FileTableFilesTCount; i++)
		{
			err = 0;
			if (verbose) wprintf_api(0, L"Extracting: %s\r\n", file_name_str[i]);

			int strlen = 0;
			wchar_t out_file_name_raw[MAX_PATH + 1];
			wchar_t out_file_name_xor1[MAX_PATH + 1];
			wchar_t out_file_name_xor2[MAX_PATH + 1];
			wchar_t out_file_name[MAX_PATH + 1];
			strlen = swprintf(out_file_name_raw, MAX_PATH, L"%s\\%s-raw.raw", dir_name, file_name_str[i]);
			if (strlen == -1) out_file_name_raw[0] = L'\0';
			strlen = swprintf(out_file_name_xor1, MAX_PATH, L"%s\\%s-xor1.raw", dir_name, file_name_str[i]);
			if (strlen == -1) out_file_name_xor1[0] = L'\0';
			strlen = swprintf(out_file_name_xor2, MAX_PATH, L"%s\\%s-xor2.raw", dir_name, file_name_str[i]);
			if (strlen == -1) out_file_name_xor2[0] = L'\0';
			strlen = swprintf(out_file_name, MAX_PATH, L"%s\\%s.slbc", dir_name, file_name_str[i]);
			if (strlen == -1) { wprintf_api(2, L"path is very long\r\n"); err = 1; continue; }
			//wprintf(L"%s\n", out_file_name);
			//{ //wprintf_api(2, L"Cannot create file: %s\n", file_name_str[i]); return 0; }
			unsigned char *data = new unsigned char[file_table[i].size];
			if (!err) if (fseek(pck_f, scene_header.FileTableDataOffset + file_table[i].offset, SEEK_SET) != 0) err = 2;
			if (!err) if (fread(data, 1, file_table[i].size, pck_f) != file_table[i].size) err = 2;
			if (err)
			{
				wprintf_api(2, L"не удалось прочитать %s. Архив %s повреждён.\r\n", file_name_str[i], file_name);
				delete[] data;
				err = 2;
				break;
			}
			
			if (verbose)
			{
				if (out_file_name_raw[0] != L'\0')
				{
					FILE *out_f;
					if (_wfopen_s(&out_f, out_file_name_raw, L"wb") == 0)
					{
						fwrite(data, 1, file_table[i].size, out_f);
						fclose(out_f);
					}
				}
			}

			//if (scene_header.Encrypt2) // Здесь всё гораздо сложнее(
			//{
				data_xor(data, file_table[i].size, sc_xor_mask, 256);

				if (verbose)
				{
					if (out_file_name_xor1[0] != L'\0')
					{
						FILE *out_f;
						if (_wfopen_s(&out_f, out_file_name_xor1, L"wb") == 0)
						{
							fwrite(data, 1, file_table[i].size, out_f);
							fclose(out_f);
						}
					}
				}

				if(scene_header.Encrypt1) 
				{
					data_xor(data, file_table[i].size, XorKeys[game_id], 16);

					if (verbose)
					{
						if (out_file_name_xor2[0] != L'\0')
						{
							FILE *out_f;
							if (_wfopen_s(&out_f, out_file_name_xor2, L"wb") == 0)
							{
								fwrite(data, 1, file_table[i].size, out_f);
								fclose(out_f);
							}
						}
					}
				}
			//}

			unsigned int compressed_size = *(unsigned int*) data;
			unsigned int uncompressed_size = *(unsigned int*) (data+4);

			if (compressed_size == file_table[i].size)
			{

					unsigned char *decompressed_data = new unsigned char[uncompressed_size];
					unsigned char *compressed_data = data + 8;
					char decomperr = va_decompress_g00_1(compressed_data, compressed_size - 8, decompressed_data, uncompressed_size);
					if (decomperr)
					{
						FILE *out_f;
						err = _wfopen_s(&out_f, out_file_name, L"wb");
						if (err == 0)
						{
							fwrite(decompressed_data, 1, uncompressed_size, out_f);
							fclose(out_f);
							unpacked_files++;
						}
						else { wprintf_api(2, L"could not write file: %s\r\n", file_name_str[i]); err = 3; }

						dumptext(decompressed_data, uncompressed_size, dir_name, file_name_str[i], duplicate_strings);

					}
					else { wprintf_api(2, L"decompress error\r\n"); err = 4; }
					delete[] decompressed_data;
			}
			else { wprintf_api(2, L"decrypt error. Incorrect compressed/decmpressed data size.\r\n"); err = 5; }

			delete[] data;
			//Sleep(1000);
			//if (err) break;
		}
		
		wprintf_api(0, L"Unpacked files: %u\r\n", unpacked_files);
//#endif
		wprintf_api(0, L"\r\n  Write project file...\r\n");
		//if (unpacked_files > 0)
		//{
			wchar_t info_file_name[MAX_PATH + 1];
			int slen = swprintf(info_file_name, MAX_PATH, L"%s\\info.xml", dir_name);
			if (slen != -1)
			{
				FILE *info_f;
				//err = _wfopen_s(&info_f, info_file_name, L"wb");
				err = _wfopen_s(&info_f, info_file_name, L"wt");
				if (err == 0)
				{

					XMLDoc doc;
					XMLNode* node;
					XMLDoc_init(&doc);

					wchar_t bufw[MAX_PATH + 1];
					char buf8[MAX_PATH + 1];

					node = XMLNode_alloc();
					XMLNode_set_tag(node, "xml version=\"1.0\" encoding=\"UTF-8\"");
					XMLNode_set_type(node, TAG_INSTR);
					XMLDoc_add_node(&doc, node);

					node = XMLNode_alloc();
					XMLNode_set_tag(node, " vasa SiglusEngien project file ");
					XMLNode_set_type(node, TAG_COMMENT);
					XMLDoc_add_node(&doc, node);

					XMLNode* rootnode = XMLNode_alloc();
					XMLNode_set_tag(rootnode, "VisualArtsScriptArchiverProject");
					XMLNode_set_type(rootnode, TAG_FATHER);

					XMLNode_set_attribute(rootnode, "version", "1");
					XMLNode_set_attribute(rootnode, "type", "pck");

					WideCharToMultiByte(CP_UTF8, NULL, GameIDs[game_id], -1, buf8, MAX_PATH, NULL, NULL);
					XMLNode_set_attribute(rootnode, "game", buf8);

					swprintf(bufw, MAX_PATH, L"0x%02X", scene_header.Encrypt1);
					WideCharToMultiByte(CP_UTF8, NULL, bufw, -1, buf8, MAX_PATH, NULL, NULL);
					XMLNode_set_attribute(rootnode, "encrypt1", buf8);

					swprintf(bufw, MAX_PATH, L"0x%02X", scene_header.Encrypt2);
					WideCharToMultiByte(CP_UTF8, NULL, bufw, -1, buf8, MAX_PATH, NULL, NULL);
					XMLNode_set_attribute(rootnode, "encrypt2", buf8);

					XMLDoc_add_node(&doc, rootnode);
					
					XMLNode* fsnode = XMLNode_alloc();
					XMLNode_set_tag(fsnode, "files");
					XMLNode_add_child(rootnode, fsnode);

					for (int i = 0; i < scene_header.FileTableFilesTCount; i++)
					{
						WideCharToMultiByte(CP_UTF8, NULL, file_name_str[i], -1, buf8, MAX_PATH, NULL, NULL);
						node = XMLNode_alloc();
						XMLNode_set_tag(node, "file");
						XMLNode_set_attribute(node, "name", buf8);
						XMLNode_add_child(fsnode, node);
					}

					XMLNode* vnode = XMLNode_alloc();
					XMLNode_set_tag(vnode, "variables");
					XMLNode_add_child(rootnode, vnode);

					for (int i = 0; i < scene_header.VarTableVarCount; i++)
					{
						//wsprintf(buf1, L"0x%08x\t0x%08x\t%s\r\n", var1[i].var1, var1[i].var2, var1_name[i]);

						WideCharToMultiByte(CP_UTF8, NULL, var1_name[i], -1, buf8, MAX_PATH, NULL, NULL);
						node = XMLNode_alloc();
						XMLNode_set_tag(node, "variable");
						XMLNode_set_attribute(node, "name", buf8);

						if (var1[i].var1 == 0xA || var1[i].var1 == 0xB)	XMLNode_set_attribute(node, "type", "int");
						else if (var1[i].var1 == 0x14 || var1[i].var1 == 0x15) XMLNode_set_attribute(node, "type", "str");

						//sprintf(buf8, "%d", var1[i].var2);
						swprintf(bufw, MAX_PATH, L"%d", var1[i].var2);
						bufw[MAX_PATH - 1] = L'\0';
						WideCharToMultiByte(CP_UTF8, NULL, bufw, -1, buf8, MAX_PATH, NULL, NULL);
						if (var1[i].var1 == 0xB || var1[i].var1 == 0x15) XMLNode_set_attribute(node, "size", buf8);

						XMLNode_add_child(vnode, node);
					}

					XMLNode* fnnode = XMLNode_alloc();
					XMLNode_set_tag(fnnode, "functions");
					XMLNode_add_child(rootnode, fnnode);

					for (int i = 0; i < scene_header.FuncTableVarCount; i++)
					{
						//wsprintf(buf1, L"%s\t0x%08x\t%s\r\n", file_name_str[var2[i].var1], var2[i].var2, var2_name[i]);

						WideCharToMultiByte(CP_UTF8, NULL, var2_name[i], -1, buf8, MAX_PATH, NULL, NULL);
						node = XMLNode_alloc();
						XMLNode_set_tag(node, "function");
						XMLNode_set_attribute(node, "name", buf8);

						WideCharToMultiByte(CP_UTF8, NULL, file_name_str[var2[i].var1], -1, buf8, MAX_PATH, NULL, NULL);
						XMLNode_set_attribute(node, "file", buf8);

						//sprintf(buf8, "0x%08x", var2[i].var2);
						swprintf(bufw, MAX_PATH, L"0x%08x", var2[i].var2);
						bufw[MAX_PATH - 1] = L'\0';
						WideCharToMultiByte(CP_UTF8, NULL, bufw, -1, buf8, MAX_PATH, NULL, NULL);
						XMLNode_set_attribute(node, "offset", buf8);

						XMLNode_add_child(fnnode, node);
					}

					XMLDoc_print(&doc, info_f, "\n", "    ", false, 0, 4);

					XMLDoc_free(&doc);

					fclose(info_f);
					wprintf_api(0, L"Save archive info in %s\r\n", info_file_name);
					#if 0
					wchar_t *buf = new wchar_t[32768];
					ZeroMemory(buf, 32768*2);
					char *utf8_buf = new char[32768 * 2];
					ZeroMemory(utf8_buf, 32768*2);
					wchar_t buf1[MAX_PATH + 1];

					wcscpy_s(buf, 32767, L"// Project info file. cp UTF-8\r\n\r\n");
					wcscat_s(buf, 32767, L"InfoStart\r\n");
					wcscat_s(buf, 32767, L"type=pck\r\n");
					swprintf(buf1, MAX_PATH, L"game=%s\r\n", GameIDs[game_id]);
					wcscat_s(buf, 32767, buf1);
					swprintf(buf1, MAX_PATH, L"encrypt1=0x%02X\r\nencrypt2=0x%02X\r\n", scene_header.Encrypt1, scene_header.Encrypt2);
					wcscat_s(buf, 32767, buf1);
					//swprintf(buf1, 32767, L"encrypt2=0x%02X\r\n", scene_header.Encrypt2);
					wcscat_s(buf, 32767, L"InfoEnd\r\n\r\n");

					WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
					fwrite(utf8_buf, 1, strlen(utf8_buf), info_f);

					wcscpy_s(buf, 32767, L"VarStart\r\n");
					//wcscpy_s(buf, 32767, L"Var1Start\r\n");
					swprintf(buf1, MAX_PATH, L"size=%d\r\n", scene_header.VarTableVarCount);
					wcscat_s(buf, 32767, buf1);
					for (int i = 0; i < scene_header.VarTableVarCount; i++)
					{
						//wsprintf(buf1, L"0x%08x\t0x%08x\t%s\r\n", var1[i].var1, var1[i].var2, var1_name[i]);
						if (var1[i].var1 == 0xA) wsprintf(buf1, L"int\t%s\r\n", var1_name[i]);
						else if (var1[i].var1 == 0xB) wsprintf(buf1, L"int\t%s[%d]\r\n", var1_name[i], var1[i].var2);
						else if (var1[i].var1 == 0x14) wsprintf(buf1, L"str\t%s\r\n", var1_name[i]);
						else if (var1[i].var1 == 0x15) wsprintf(buf1, L"str\t%s[%d]\r\n", var1_name[i], var1[i].var2);
						wcscat_s(buf, 32767, buf1);
					}
					wcscat_s(buf, 32767, L"VarEnd\r\n\r\n");

					WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
					fwrite(utf8_buf, 1, strlen(utf8_buf), info_f);

					wcscpy_s(buf, 32767, L"FuncStart\r\n");
					swprintf(buf1, MAX_PATH, L"size=%d\r\n", scene_header.FuncTableVarCount);
					wcscat_s(buf, 32767, buf1);
					for (int i = 0; i < scene_header.FuncTableVarCount; i++)
					{
						//wsprintf(buf1, L"0x%08x\t0x%08x\t%s\r\n", var2[i].var1, var2[i].var2, var2_name[i]);
						wsprintf(buf1, L"%s\t0x%08x\t%s\r\n", file_name_str[var2[i].var1], var2[i].var2, var2_name[i]);
						wcscat_s(buf, 32767, buf1);
					}
					wcscat_s(buf, 32767, L"FuncEnd\r\n\r\n");

					WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
					fwrite(utf8_buf, 1, strlen(utf8_buf), info_f);

					wcscpy_s(buf, 32767, L"FilesStart\r\n");
					swprintf(buf1, MAX_PATH, L"size=%d\r\n", scene_header.FileTableFilesTCount);
					wcscat_s(buf, 32767, buf1);
					for (int i = 0; i < scene_header.FileTableFilesTCount; i++)
					{
						wcscat_s(buf, 32767, file_name_str[i]);
						wcscat_s(buf, 32767, L"\r\n");
					}
					wcscat_s(buf, 32767, L"FilesEnd\r\n");

					WideCharToMultiByte(CP_UTF8, NULL, buf, -1, utf8_buf, 32768 * 2, NULL, NULL);
					fwrite(utf8_buf, 1, strlen(utf8_buf), info_f);

					fclose(info_f);
					delete[] utf8_buf;
					delete[] buf;
					wprintf_api(0, L"Save archive info in %s\r\n", info_file_name);
					#endif
				}
				else wprintf_api(2, L"could not create project file: %s.\r\n", info_file_name);
			}
			else wprintf_api(2, L"path is very long.\r\n");
		//}
	}
	else wprintf_api(2, L"could not create directory: %s.\r\n", dir_name);

	clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
	fclose(pck_f);

	return 1;
}


int create_se_arc(wchar_t *file_name, wchar_t *dir_name, int compress_lv, int game_id, bool encrypt_scene)
{
	if (file_name == NULL || dir_name == NULL) return 1;
	wprintf_api(0, L"Open directory: %s\r\n", dir_name);
	if (dir_name[0] == L'\0') { wprintf_api(2, L"cannot find directory.\r\n"); return 1; }
	DWORD dirstate = GetFileAttributesW(dir_name);
	if (dirstate == INVALID_FILE_ATTRIBUTES || (dirstate & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		wprintf_api(2, L"cannot find directory '%s'.\r\n", dir_name);
		return 1;
	}

	errno_t err;
	int infoftype = 0;

	ScenePCKhead scene_header;
	memset(&scene_header, 0, sizeof(ScenePCKhead));
	scene_header.HeadSize = sizeof(ScenePCKhead);

	wchar_t **var1_name = NULL, **var2_name = NULL, **file_name_str = NULL;
	FileTable *file_table = NULL;
	VarStruct *var1 = NULL, *var2 = NULL;

	err = 0;
	int section = 0;
	int elements = 0;
	//int celement = 0;
	scene_header.FileTableDataCount = 0;
	scene_header.FileTableFilesTCount = 0;
	scene_header.FileTableNameTCount = 0;
	scene_header.FileTableNamesCount = 0;
	int FileTableNamesLen = 0;
	scene_header.VarTableVarCount = 0;
	scene_header.VarTableNameTCount = 0;
	scene_header.VarTableNamesCount = 0;
	int VarTableNamesLen = 0;
	scene_header.FuncTableVarCount = 0;
	scene_header.FuncTableNameTCount = 0;
	scene_header.FuncTableNamesCount = 0;
	int FuncTableNamesLen = 0;
	int workssection = 0;
	int Encrypt1Flag = 0;
	int Encrypt2Flag = 0x366;

	wchar_t info_file_name[MAX_PATH + 1];

	wprintf_api(0, L"Read project file: %s\\info.xml\r\n", dir_name);
	int slen = swprintf(info_file_name, MAX_PATH, L"%s\\info.xml", dir_name);
	if (slen == -1) { wprintf_api(2, L"path is very long.\r\n"); return 1; }
	
	FILE *info_f;
	err = open_bin_to_read(&info_f, info_file_name);
	if (err <= 0)
	{
		//wprintf_api(1, L"cannot open project file: %s.\r\n", info_file_name);

		wprintf_api(0, L"Read project file: %s\\info.txt\r\n", dir_name);
		swprintf(info_file_name, MAX_PATH, L"%s\\info.txt", dir_name);
		err = open_bin_to_read(&info_f, info_file_name);
		if (err <= 0) { wprintf_api(2, L"cannot open project file: %s.\r\n", info_file_name); return 1; }
		else infoftype = 1;

	}
	else infoftype = 2;

	// check BOM
	int utfbom;
	fread(&utfbom, 4, 1, info_f);
	if (utfbom == 0x0000FEFF || utfbom == 0xFFFE0000 || (utfbom & 0x0000FFFF) == 0xFEFF || (utfbom & 0x0000FFFF) == 0xFFFE) 
	{
		wprintf_api(2, L"incorrect codepage. Support UTF-8 only.\r\n");
		return 1;
	}
	// skeep UTF-8 BOM
	if ((utfbom & 0x00FFFFFF) == 0xBFBBEF) { err -= 3; fseek(info_f, 3, SEEK_SET); }
	else fseek(info_f, 0, SEEK_SET);

	char *utf8_buf = new char[err + 1];
	memset(utf8_buf, 0, err + 1);
	//wchar_t *buf = new wchar_t[err + 1];
	//ZeroMemory(buf, err * 2 + 1);

	err = fread(utf8_buf, 1, err, info_f);
	fclose(info_f);

	if (infoftype == 2)
	{
		
	XMLDoc doc;
	XMLNode* node;
	XMLDoc_init(&doc);
	const char *attrval = NULL;

	XMLDoc_parse_buffer_DOM_text_as_nodes(utf8_buf, "info.xml", &doc, 0);

	//delete[] utf8_buf;

	if (doc.n_nodes <= 0)
	{
		wprintf_api(2, L"XML: incorrect xml in project file.\r\n");
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	node = doc.nodes[0];
	if (node->active && node->tag_type == TAG_INSTR)
	{
		char *encattr = strstr(node->tag, "encoding");
		if (encattr)
		{
			if(strncmp(encattr+10, "UTF-8", 5) != 0)
			{
				wprintf_api(2, L"incorrect codepage. Support UTF-8 only.\r\n");
				XMLDoc_free(&doc);
				delete[] utf8_buf;
				//delete[] buf;
				return 1;
			}
		}
	}

	//printf("%d %d\n", doc.n_nodes, doc.i_root);
	XMLNode* rootnode = XMLDoc_root(&doc);

	if (stricmp(rootnode->tag, "VisualArtsScriptArchiverProject") != 0)
	{
		wprintf_api(2, L"XML: cannot find 'VisualArtsScriptArchiverProject' tag.");
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	//int rootnodeatrcnt = XMLNode_get_attribute_count(rootnode);

	if (attrval) free((void*)attrval);
	attrval = NULL;
	if (XMLNode_get_attribute_with_default(rootnode, "type", &attrval, NULL) && attrval != NULL)
	{
		if(strcmp(attrval, "pck") != 0)
		{
			wprintf_api(2, L"XML: unknown archive type.\r\n");
			//wprintf_api(2, L"XML: cannot find encrypt1 attribute.\r\n");
			if (attrval) free((void*)attrval);
			attrval = NULL;
			XMLDoc_free(&doc);
			delete[] utf8_buf;
			//delete[] buf;
			return 1;
		}
	}
	else
	{
		wprintf_api(2, L"XML: cannot find 'type' attribute.\r\n");
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	if (attrval) free((void*)attrval);
	attrval = NULL;
	if (XMLNode_get_attribute_with_default(rootnode, "encrypt1", &attrval, NULL) && attrval != NULL)
	{
		char *nextptr;
		int ival = 0;
		ival = strtol(attrval, &nextptr, 16);
		if (nextptr == attrval || errno == ERANGE)
		{
			wprintf_api(1, L"XML: incorrect value\r\n");
			//return -1;
		}
		else
			Encrypt1Flag = ival;
	}
	else
	{
		wprintf_api(2, L"XML: cannot find 'encrypt1' attribute.\r\n");
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	if (attrval) free((void*)attrval);
	attrval = NULL;
	if (XMLNode_get_attribute_with_default(rootnode, "encrypt2", &attrval, NULL) && attrval != NULL)
	{
		char *nextptr;
		int ival = 0;
		ival = strtol(attrval, &nextptr, 16);
		if (nextptr == attrval || errno == ERANGE)
		{
			wprintf_api(1, L"XML: incorrect value\r\n");
			//return -1;
		}
		else
			Encrypt2Flag = ival;
	}
	else
	{
		wprintf_api(2, L"XML: cannot find 'encrypt2' attribute.\r\n");
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	XMLNode* fsnode = XMLNode_search_child(rootnode, "files", 0);
	elements = XMLNode_get_children_count(fsnode);
	//printf("%d\n", elements);
	if (elements <= 0)
	{
		wprintf_api(2, L"XML: cannot find 'files' tag.\r\n");
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}

	file_name_str = new wchar_t*[elements];
	memset(file_name_str, 0, elements * sizeof(wchar_t*));

	err = 0;
	node = fsnode;
	for (int i = 0; i < elements; i++)
	{
		node = XMLNode_next(node);
		if (strcmp(node->tag, "file") != 0) continue;
		//printf("%s\n", node->tag);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "name", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'name' attribute.\r\n"); err = 1; break; }
		
		wchar_t *ufn = iconv_string_8to16(attrval, -1);
		if (!ufn) { wprintf_api(2, L"XML: iconv error.\r\n"); err = 2; break; }
		int l = wcslen(ufn);
		file_name_str[scene_header.FileTableNameTCount] = new wchar_t[l + 1];
		wcscpy(file_name_str[scene_header.FileTableNameTCount], ufn);
		FileTableNamesLen += l;
		//wprintf(L"%s\n", file_name_str[scene_header.FileTableNameTCount]);
		scene_header.FileTableDataCount++;
		scene_header.FileTableFilesTCount++;
		scene_header.FileTableNameTCount++;
		scene_header.FileTableNamesCount++;
		free(ufn);
	}

	if (err != 0)
	{
		wprintf_api(2, L"XML: cannot parse 'files' section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}


	XMLNode* vnode = XMLNode_search_child(rootnode, "variables", 0);
	elements = XMLNode_get_children_count(vnode);
	//printf("%d\n", elements);
	/*if (elements <= 0)
	{
		wprintf_api(2, L"XML: cannot find 'variables' node.\r\n");
		XMLDoc_free(&doc);
		//delete[] utf8_buf;
		delete[] buf;
		return 1;
	}*/
	
	if (elements > 0)
	{
	var1 = new VarStruct[elements];
	var1_name = new wchar_t*[elements];
	memset(var1_name, 0, elements * sizeof(wchar_t*));

	node = vnode;
	for (int i = 0; i < elements; i++)
	{
		node = XMLNode_next(node);
		if (strcmp(node->tag, "variable") != 0) continue;
		//printf("%s\n", node->tag);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "name", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'name' attribute.\r\n"); err = 1; break; }

		wchar_t *uvn = iconv_string_8to16(attrval, -1);
		if (!uvn)
		{
			wprintf_api(2, L"XML: iconv error.\r\n");
			err = 2;
			break;
		}
		int l = wcslen(uvn);
		var1_name[scene_header.VarTableNameTCount] = new wchar_t[l + 1];
		wcscpy(var1_name[scene_header.VarTableNameTCount], uvn);
		VarTableNamesLen += l;

		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "type", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'name' attribute.\r\n"); err = 1; break; }

		if (strcmp(attrval, "int") == 0)
		{
			var1[scene_header.VarTableVarCount].var1 = 0x0A;
		}
		else if (strcmp(attrval, "str") == 0)
		{
			var1[scene_header.VarTableVarCount].var1 = 0x14;
		}
		else
		{
			wprintf_api(2, L"XML: unknown type.\r\n");
			err = 2;
			break;
		}

		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (XMLNode_get_attribute_with_default(node, "size", &attrval, NULL) && attrval != NULL)
		{
			char *nextptr;
			int ival = 0;
			ival = strtol(attrval, &nextptr, 10);
			if (nextptr == attrval || errno == ERANGE)
			{
				wprintf_api(2, L"XML: incorrect value\r\n");
				err = 2;
				break;
			}
			var1[scene_header.VarTableVarCount].var1++;
			var1[scene_header.VarTableVarCount].var2 = ival;
		}
		else
		{
			var1[scene_header.VarTableVarCount].var2 = 0;
		}

		scene_header.VarTableVarCount++;
		scene_header.VarTableNameTCount++;
		scene_header.VarTableNamesCount++;
		free(uvn);
	}

	}
	
	if (err != 0)
	{
		wprintf_api(2, L"XML: cannot parse 'variables' section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}


	XMLNode* fnnode = XMLNode_search_child(rootnode, "functions", 0);
	elements = XMLNode_get_children_count(fnnode);
	//printf("%d\n", elements);
	/*if (elements <= 0)
	{
		wprintf_api(2, L"XML: cannot find 'variables' node.\r\n");
		XMLDoc_free(&doc);
		//delete[] utf8_buf;
		delete[] buf;
		return 1;
	}*/
	
	if (elements > 0)
	{
	var2 = new VarStruct[elements];
	var2_name = new wchar_t*[elements];
	memset(var2_name, 0, elements * sizeof(wchar_t*));

	node = fnnode;
	for (int i = 0; i < elements; i++)
	{
		node = XMLNode_next(node);
		if (strcmp(node->tag, "function") != 0) continue;
		//printf("%s\n", node->tag);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "name", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'name' attribute.\r\n"); err = 1; break; }

		wchar_t *uvn = iconv_string_8to16(attrval, -1);
		if (!uvn)
		{
			wprintf_api(2, L"XML: iconv error.\r\n");
			err = 2;
			break;
		}
		int l = wcslen(uvn);
		var2_name[scene_header.FuncTableNameTCount] = new wchar_t[l + 1];
		wcscpy(var2_name[scene_header.FuncTableNameTCount], uvn);
		FuncTableNamesLen += l;
		free(uvn);

		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "file", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'file' attribute.\r\n"); err = 1; break; }

		uvn = iconv_string_8to16(attrval, -1);
		if (!uvn)
		{
			wprintf_api(2, L"XML: iconv error.\r\n");
			err = 2;
			break;
		}
		var2[scene_header.FuncTableVarCount].var1 = 100000;
		for (int i = 0; i < scene_header.FileTableNameTCount; i++)
		{
			if (wcscmp(file_name_str[i], uvn) == 0) var2[scene_header.FuncTableVarCount].var1 = i;
		}
		if (var2[scene_header.FuncTableVarCount].var1 >= 100000)
		{
			wprintf_api(2, L"XML: cannot find.\r\n");
			err = 3;
			break;
		}

		if (attrval) free((void*)attrval);
		attrval = NULL;
		if (!XMLNode_get_attribute_with_default(node, "offset", &attrval, NULL) || attrval == NULL) { wprintf_api(2, L"XML: cannot find 'offset' attribute.\r\n"); err = 1; break; }
		char *nextptr;
		int ival = 0;
		ival = strtol(attrval, &nextptr, 16);
		if (nextptr == attrval || errno == ERANGE)
		{
			wprintf_api(2, L"XML: incorrect value\r\n");
			err = 2;
			break;
		}
		var2[scene_header.FuncTableVarCount].var2 = ival;


		scene_header.FuncTableVarCount++;
		scene_header.FuncTableNameTCount++;
		scene_header.FuncTableNamesCount++;
		free(uvn);
	}

	}
	
	if (err != 0)
	{
		wprintf_api(2, L"XML: cannot parse 'functions' section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		if (attrval) free((void*)attrval);
		attrval = NULL;
		XMLDoc_free(&doc);
		delete[] utf8_buf;
		//delete[] buf;
		return 1;
	}
	
	//XMLNode_print(node, stdout, NULL, NULL, false, 0, 0);

	//XMLDoc_parse_file("info.xml", &xmldoc);

	if (attrval) free((void*)attrval);
	attrval = NULL;
	XMLDoc_free(&doc);
	delete[] utf8_buf;
	//delete[] buf;

	//return 1;

	}
	else
	{
		wchar_t *buf = new wchar_t[err + 1];

	SetLastError(ERROR_SUCCESS);
	err = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_buf, err + 1, buf, err + 1);
	if (err == 0 || GetLastError() == ERROR_NO_UNICODE_TRANSLATION) //err = 1;
	{
		wprintf_api(2, L"incorrect text encoding in project file.\r\n");
		delete[] utf8_buf;
		delete[] buf;
		return 1;
	}

	err = 0;

	delete[] utf8_buf;

	wchar_t buf1[MAX_PATH + 1];

	wchar_t *ptr = buf;
	while(get_string(buf1, MAX_PATH, ptr))
	{
		wchar_t *finstr = buf1;
		while (is_space_w(*finstr)) finstr++;
		int l = wcslen(finstr);
		while (is_space_w(finstr[l - 1])) finstr[--l] = 0;
		if (*finstr == L'\0') continue;
		if (finstr[0] == L'/' && finstr[1] == L'/') continue;
		if (wcscmp(finstr, L"InfoEnd") == 0) { section = 0; elements = 0; continue; }
		else if (wcscmp(finstr, L"VarEnd") == 0 || wcscmp(finstr, L"Var1End") == 0)
		{
			if (elements != scene_header.VarTableNameTCount) wprintf_api(1, L"incorrect var table size.\r\n");
			//scene_header.VarTableNameTCount = celement;
			section = 0; elements = 0; continue;
		}
		else if (wcscmp(finstr, L"FuncEnd") == 0 || wcscmp(finstr, L"Var2End") == 0)
		{
			//if (elements != scene_header.FuncTableNameTCount) wprintf_api(1, L"incorrect var2 table size.\r\n");
			if (elements != scene_header.FuncTableNameTCount) wprintf_api(1, L"incorrect func table size.\r\n");
			//scene_header.FuncTableNameTCount = celement;
			section = 0; elements = 0; continue;
		}
		else if (wcscmp(finstr, L"FilesEnd") == 0)
		{
			if (elements != scene_header.FileTableNameTCount) wprintf_api(1, L"incorrect file table size.\r\n");
			//scene_header.FileTableNameTCount = celement;
			section = 0; elements = 0; continue;
		}
		else if (wcscmp(finstr, L"InfoStart") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted InfoStart.\r\n");
			workssection = workssection | 0x01;
			elements = 0; section = 1; continue;
		}
		else if (wcscmp(finstr, L"Var1Start") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted Var1Start.\r\n");
			workssection = workssection | 0x02;
			elements = 0; section = 2; continue;
		}
		else if (wcscmp(finstr, L"VarStart") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted VarStart.\r\n");
			workssection = workssection | 0x02;
			elements = 0; section = 2; continue;
		}
		else if (wcscmp(finstr, L"Var2Start") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted Var2Start.\r\n");
			workssection = workssection | 0x04;
			elements = 0; section = 3; continue;
		}
		else if (wcscmp(finstr, L"FuncStart") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted FuncStart.\r\n");
			workssection = workssection | 0x04;
			elements = 0; section = 3; continue;
		}
		else if (wcscmp(finstr, L"FilesStart") == 0)
		{
			if (section != 0) wprintf_api(1, L"expacted FilesStart.\r\n");
			workssection = workssection | 0x08;
			elements = 0; section = 4; continue;
		}

		if (section == 1)
		{
			wchar_t *str_val = 0;
			int int_val = 0;
			str_val = get_str_val(L"type", finstr);
			if (str_val) if (_wcsicmp(str_val, L"PCK") != 0) { wprintf_api(2, L"unknown archive type.\r\n"); err = 1; break; }
			str_val = get_str_val(L"game", finstr);
			if (str_val) for (unsigned int i = 0; i < GameIDs.size(); i++) if (_wcsicmp(str_val, GameIDs[i]) == 0) game_id = i;

			int_val = get_int_val(L"encrypt1", finstr, 16);
			int_val = get_int_val(L"encrypt2", finstr, 16);
			if (int_val) Encrypt2Flag = int_val;
		}
		else if (section == 2)
		{
			if (elements == 0) 
			{
				elements = get_int_val(L"size", finstr, 10);
				if (elements == 0) elements = 1000;
				var1 = new VarStruct[elements];
				var1_name = new wchar_t*[elements];
				ZeroMemory(var1_name, elements * sizeof(wchar_t*));
				//celement = 0;
				continue;
			}
			if (scene_header.VarTableVarCount >= elements) { wprintf_api(2, L"var1 table overload.\r\n"); err = 2; break; }
			while (is_space_w(*finstr)) finstr++;
			wchar_t *var2_ptr;
			var1[scene_header.VarTableVarCount].var1 = wcstol(finstr, &var2_ptr, 16);
			if (errno == 34) wprintf_api(1, L"incorrect value in %s.\r\n", finstr);
			while (is_space_w(*var2_ptr)) var2_ptr++;
			wchar_t *name_ptr;
			var1[scene_header.VarTableVarCount].var2 = wcstol(var2_ptr, &name_ptr, 16);
			if (errno == 34) wprintf_api(1, L"incorrect value in %s.\r\n", finstr);
			while (is_space_w(*name_ptr)) name_ptr++;
			l = wcslen(name_ptr);
			var1_name[scene_header.VarTableNameTCount] = new wchar_t[l + 1];
			wcscpy(var1_name[scene_header.VarTableNameTCount], name_ptr);
			VarTableNamesLen += l;
			scene_header.VarTableVarCount++;
			scene_header.VarTableNameTCount++;
			scene_header.VarTableNamesCount++;
		}
		else if (section == 3)
		{
			if (elements == 0) 
			{
				elements = get_int_val(L"size", finstr, 10);
				if (elements == 0) elements = 1000;
				var2 = new VarStruct[elements];
				var2_name = new wchar_t*[elements];
				ZeroMemory(var2_name, elements * sizeof(wchar_t*));
				//celement = 0;
				continue;
			}
			if (scene_header.FuncTableVarCount >= elements) { wprintf_api(2, L"func table overload.\r\n"); err = 3; break; }
			while (is_space_w(*finstr)) finstr++;
			wchar_t *var2_ptr;
			var2[scene_header.FuncTableVarCount].var1 = wcstol(finstr, &var2_ptr, 16);
			if (errno == 34) wprintf_api(1, L"incorrect value in %s.\r\n", finstr);
			while (is_space_w(*var2_ptr)) var2_ptr++;
			wchar_t *name_ptr;
			var2[scene_header.FuncTableVarCount].var2 = wcstol(var2_ptr, &name_ptr, 16);
			if (errno == 34) wprintf_api(1, L"incorrect value in %s.\r\n", finstr);
			while (is_space_w(*name_ptr)) name_ptr++;
			l = wcslen(name_ptr);
			var2_name[scene_header.FuncTableNameTCount] = new wchar_t[l + 1];
			wcscpy(var2_name[scene_header.FuncTableNameTCount], name_ptr);
			FuncTableNamesLen += l;
			scene_header.FuncTableVarCount++;
			scene_header.FuncTableNameTCount++;
			scene_header.FuncTableNamesCount++;
		}
		else if (section == 4)
		{
			if (elements == 0) 
			{
				elements = get_int_val(L"size", finstr, 10);
				if (elements == 0) elements = 1000;
				file_name_str = new wchar_t*[elements];
				ZeroMemory(file_name_str, elements * sizeof(wchar_t*));
				//celement = 0;
				continue;
			}
			if (scene_header.FileTableNameTCount >= elements) { wprintf_api(2, L"string table overload.\r\n"); err = 4; break; }
			l = wcslen(finstr);
			file_name_str[scene_header.FileTableNameTCount] = new wchar_t[l + 1];
			wcscpy(file_name_str[scene_header.FileTableNameTCount], finstr);
			FileTableNamesLen += l;
			scene_header.FileTableDataCount++;
			scene_header.FileTableFilesTCount++;
			scene_header.FileTableNameTCount++;
			scene_header.FileTableNamesCount++;
		}
	}

	delete[] buf;

	if (err != 0)
	{
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
	}

	if ((workssection & 0x01) == 0)
	{
		wprintf_api(2, L"cannot find or invalid Info section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
	}
	/*
	if ((workssection & 0x02) == 0)
	{
		wprintf_api(2, L"cannot find or invalid Variables section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
	}
	if ((workssection & 0x04) == 0)
	{
		wprintf_api(2, L"cannot find or invalid Functions section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
	}
	*/
	if ((workssection & 0x08) == 0)
	{
		wprintf_api(2, L"cannot find or invalid Files section.\r\n");
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
		/*
		std::vector<std::wstring> file_list;
		scene_header.FileTableNamesCount = get_files_in_dir(dir_name, L"*.slbc", &file_list);
		if (scene_header.FileTableNamesCount == 0)
		{
			clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
			return 1;
		}

		file_name_str = new wchar_t*[scene_header.FileTableNamesCount];
		ZeroMemory(file_name_str, scene_header.FileTableNamesCount * sizeof(wchar_t*));

		for (unsigned int i = 0; i < file_list.size(); i++)
		{
		int l = file_list[i].length() - 4;
		file_name_str[scene_header.FileTableNameTCount] = new wchar_t[l + 1];
		wcsncpy_s(file_name_str[scene_header.FileTableNameTCount], l, file_list[i].c_str(), -1);
		FileTableNamesLen += l;
		scene_header.FileTableDataCount++;
		scene_header.FileTableFilesTCount++;
		scene_header.FileTableNameTCount++;
		}*/
	}

	}

	wprintf_api(0, L"Variables: %d\r\n  Functions: %d\r\n  Files:     %d\r\n", scene_header.VarTableVarCount, scene_header.FuncTableVarCount, scene_header.FileTableDataCount);
	wprintf_api(0, L"Encrypt: ");
	if (encrypt_scene && game_id) wprintf_api(0, L"Yes\r\n");
	else wprintf_api(0, L"No\r\n");

	wprintf_api(0, L"Create:    %s\r\n", file_name);

	FILE *pck_f;
	err = _wfopen_s(&pck_f, file_name, L"wb");
	if (err != 0)
	{
		wprintf_api(2, L"cannot create '%s'.\r\n", file_name);
		clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
		return 1;
	}
	//{
		err = 0;

		// Write temp Header
		fseek(pck_f, 0, SEEK_SET);
		fwrite(&scene_header, 1, scene_header.HeadSize, pck_f);

		// Write Var1
		if (scene_header.VarTableVarCount != scene_header.VarTableNameTCount || scene_header.VarTableVarCount != scene_header.VarTableNamesCount) err = 2;			
		//if (err > 0) { wprintf_api(2, L"cannot create arhive. errcode: %d\n", err); }
		scene_header.VarTableVarOffset = scene_header.HeadSize;
		//wprintf_api(0, L"%d %d\n", scene_header.VarTableVarOffset, ftell(pck_f));
		if (err == 0) err = write_vars(pck_f, scene_header.VarTableVarOffset, &scene_header.VarTableVarOffset, &scene_header.VarTableNameTOffset, &scene_header.VarTableNamesOffset, scene_header.VarTableVarCount, var1, var1_name);
		if (err > 0) { wprintf_api(2, L"cannot create archive (variables). errcode: %d.\r\n", err); }

		if (err != 0)
		{
			clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
			fclose(pck_f);
			return 1;
		}
			
		// Write Var2
		if (scene_header.FuncTableVarCount != scene_header.FuncTableNameTCount || scene_header.FuncTableVarCount != scene_header.FuncTableNamesCount) err = 3;			
		//if (err > 0) { wprintf_api(2, L"cannot create arhive. errcode: %d\n", err); }
		scene_header.FuncTableVarOffset = scene_header.VarTableVarOffset + scene_header.VarTableVarCount * sizeof(VarStruct) + scene_header.VarTableNameTCount * sizeof(FileTable) + VarTableNamesLen * 2;
		//wprintf_api(0, L"%d %d\n", scene_header.FuncTableVarOffset, ftell(pck_f));
		if (err == 0) err = write_vars(pck_f, scene_header.FuncTableVarOffset, &scene_header.FuncTableVarOffset, &scene_header.FuncTableNameTOffset, &scene_header.FuncTableNamesOffset, scene_header.FuncTableVarCount, var2, var2_name);
		if (err > 0) { wprintf_api(2, L"cannot create archive (functions). errcode: %d.\r\n", err); }

		if (err != 0)
		{
			clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
			fclose(pck_f);
			return 1;
		}

		// Write FileNames
		if (scene_header.FileTableDataCount != scene_header.FileTableFilesTCount || scene_header.FileTableDataCount != scene_header.FileTableNameTCount || scene_header.FileTableDataCount != scene_header.FileTableNamesCount) err = 4;
		//if (err > 0) { wprintf_api(2, L"cannot create arhive. errcode: %d\n", err); }
		scene_header.FileTableNameTOffset = scene_header.FuncTableVarOffset + scene_header.FuncTableVarCount * sizeof(VarStruct) + scene_header.FuncTableNameTCount * sizeof(FileTable) + FuncTableNamesLen * 2;
		if (err == 0) err = write_names(pck_f, scene_header.FileTableNameTOffset, &scene_header.FileTableNameTOffset, &scene_header.FileTableNamesOffset, scene_header.FileTableNamesCount, file_name_str);
		if (err > 0) { wprintf_api(2, L"cannot create archive (file table). errcode: %d.\r\n", err); }

		if (err != 0)
		{
			clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
			fclose(pck_f);
			return 1;
		}

		scene_header.FileTableFilesTOffset = ftell(pck_f);///
		file_table = new FileTable[scene_header.FileTableFilesTCount];
		scene_header.FileTableDataOffset = scene_header.FileTableFilesTOffset + scene_header.FileTableFilesTCount * sizeof(FileTable);

		fseek(pck_f, scene_header.FileTableDataOffset, SEEK_SET);

		wprintf_api(0, L"\r\n  Reading, compressing, encrypting and writing...\r\n");

		for (int i = 0; i < scene_header.FileTableDataCount; i++)
		{
			err = 0;

			if (verbose) wprintf_api(0, L"\r\n  Read file: %s\r\n", file_name_str[i]);
			wchar_t scr_file_name[MAX_PATH + 1];
			int slen = swprintf(scr_file_name, MAX_PATH, L"%s\\%s.slbc", dir_name, file_name_str[i]);
			if (slen == -1) { wprintf_api(2, L"path is very long.\r\n"); err = 5; break; }

			FILE *scr_f;
			err = open_bin_to_read(&scr_f, scr_file_name);
			if (err <= 0) { wprintf_api(2, L"cannot open file '%s'.\r\n", file_name_str[i]); err = 5; break; }
			int scr_f_size = err;
			err = 0;
			if (scr_f_size > 10000000) { wprintf_api(2, L"bytecode file is veri big. Max file size 10 MB.\r\n"); fclose(scr_f); err = 5; break; }

			unsigned char *scr_data = new unsigned char[scr_f_size];
			fread(scr_data, 1, scr_f_size, scr_f);

			if (*((int*) scr_data) != 132) { wprintf_api(2, L"incorrect bytecode file '%s'.\r\n", file_name_str[i]); delete[] scr_data; fclose(scr_f); err = 5; break; }

			loadtext(scr_data, &scr_f_size, dir_name, file_name_str[i]);
				
			int scr_out_size = scr_f_size * 2;
			unsigned char *scr_out = new unsigned char[scr_out_size];
			//unsigned char *scr_out = new unsigned char[10000000 + 8];
			FileTable *scr_info = (FileTable*) scr_out;
			unsigned char *compressed_data = scr_out + 8;

			int complv = compress_lv;
			//if (complv != 0)
			//{
			if (complv == 6)
			{
				if (scr_f_size < 200000) complv = 4;
				else if (scr_f_size >= 200000 && scr_f_size < 500000) complv = 3;
				else if (scr_f_size >= 500000 && scr_f_size < 1000000) complv = 2;
				else if (scr_f_size >= 1000000) complv = 1;
			}			

			if (verbose) wprintf_api(0, L"Compresing... Lv: %d\r\n", complv);

			scr_out_size = va_compress_g00_1(scr_data, scr_f_size, compressed_data, windowsize[complv]) + 8;
			scr_info->offset = scr_out_size;
			scr_info->size = scr_f_size;
			/*}
			else
			{
				MoveMemory(scr_out, scr_data, scr_f_size);
				scr_our_size = scr_f_size;
				//scr_info->offset = scr_our_size;
				//scr_info->size = scr_f_size;
			}*/

			if (verbose) wprintf_api(0, L"Encrypting...\r\n");
			data_xor(scr_out, scr_out_size, sc_xor_mask, 256);
			if (encrypt_scene && game_id) data_xor(scr_out, scr_out_size, XorKeys[game_id], 16);

			file_table[i].size = scr_out_size;
			file_table[i].offset = ftell(pck_f) - scene_header.FileTableDataOffset;

			if (verbose) wprintf_api(0, L"Writing...\r\n");
			if (fwrite(scr_out, 1, scr_out_size, pck_f) != scr_out_size) { wprintf_api(2, L"cannot write file.\r\n"); err = 5; }

			delete[] scr_data;
			delete[] scr_out;
			fclose(scr_f);
			if (err != 0) break;
		}

		if (err != 0)
		{
			clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);
			fclose(pck_f);
			return 1;
		}

		// Write FileTable
		fseek(pck_f, scene_header.FileTableFilesTOffset, SEEK_SET);
		fwrite(file_table, sizeof(FileTable), scene_header.FileTableFilesTCount, pck_f);

		//scene_header.Encrypt2 = windowsize[compress_lv];
		if (encrypt_scene && game_id) scene_header.Encrypt1 = 1;
		else scene_header.Encrypt1 = 0;
		scene_header.Encrypt2 = Encrypt2Flag;

		// Write Header
		fseek(pck_f, 0, SEEK_SET);
		fwrite(&scene_header, 1, scene_header.HeadSize, pck_f);

	clear_pck_info(scene_header, var1, var2, var1_name, var2_name, file_table, file_name_str);

	fclose(pck_f);
		//}
	//}
	return 1;
}


void clear_pck_info(ScenePCKhead scene_header, VarStruct *var1, VarStruct *var2, wchar_t **var1_name, wchar_t **var2_name, FileTable *file_table, wchar_t **file_name_str)
{
	if (var1) { delete[] var1; /*printf("del\n");*/ }
	while (scene_header.VarTableNamesCount--) if (var1_name[scene_header.VarTableNamesCount]) delete[] var1_name[scene_header.VarTableNamesCount];
	if (var1_name) { delete[] var1_name; /*printf("del\n");*/ }

	if (var2) { delete[] var2; /*printf("del\n");*/ }
	while (scene_header.FuncTableNamesCount--) if (var2_name[scene_header.FuncTableNamesCount]) delete[] var2_name[scene_header.FuncTableNamesCount];
	if (var2_name) { delete[] var2_name; /*printf("del\n");*/ }

	if (file_table) { delete[] file_table; /*printf("del\n");*/ }
	while (scene_header.FileTableNameTCount--) if (file_name_str[scene_header.FileTableNameTCount]) delete[] file_name_str[scene_header.FileTableNameTCount];
	if (file_name_str) { delete[] file_name_str; /*printf("del\n");*/ }
}

int write_names(FILE *pck_f, int startoffset, int *NameTOffset, int *NamesOffset, int NamesCount, wchar_t **name_str)
{
	errno_t err = 0;

	*NameTOffset = ftell(pck_f);
	if (*NameTOffset != startoffset) { /*wprintf_api(2, L"fatal error: var1 names table\n");*/ err = 3; }
	FileTable *name_table = new FileTable[NamesCount];
	int lastoffest = 0;
	for (int i = 0; i < NamesCount; i++)
	{
		name_table[i].offset = lastoffest;
		name_table[i].size = wcslen(name_str[i]);
		lastoffest += name_table[i].size;
	}
	//fseek(pck_f, scene_header.VarTableNameTOffset, SEEK_SET);
	if (err == 0) if (fwrite(name_table, sizeof(FileTable), NamesCount, pck_f) != NamesCount) err = 4;

	*NamesOffset = ftell(pck_f);
	if (*NamesOffset != *NameTOffset + NamesCount * sizeof(FileTable)) { /*wprintf_api(2, L"fatal error: var1 names\n");*/ err = 5; }
	for (int i = 0; i < NamesCount; i++)
	{
		if (*NamesOffset + name_table[i].offset * 2 != ftell(pck_f)) { /*wprintf_api(2, L"fatal error: var1 names\n");*/ err = 6; break; }
		//fseek(pck_f, scene_header.VarTableNamesOffset + var1_name_table[i].offset * 2, SEEK_SET);
		if (err == 0) if (fwrite(name_str[i], 2, name_table[i].size, pck_f) != name_table[i].size) { err = 7; break; }
	}
	delete[] name_table;

	return err;
}

int write_vars(FILE *pck_f, int startsection, int *VarTableVarOffset, int *VarTableNameTOffset, int *VarTableNamesOffset, int VarTableVarCount, VarStruct *var, wchar_t **var_name)
{
	errno_t err = 0;

	*VarTableVarOffset = ftell(pck_f);
	if (*VarTableVarOffset != startsection) return 1;
	//fseek(pck_f, scene_header.VarTableVarOffset, SEEK_SET);
	if (fwrite(var, sizeof(VarStruct), VarTableVarCount, pck_f) != VarTableVarCount) return 2;
	
	*VarTableNameTOffset = *VarTableVarOffset + VarTableVarCount * sizeof(VarStruct);

	err = write_names(pck_f, *VarTableNameTOffset, VarTableNameTOffset, VarTableNamesOffset, VarTableVarCount, var_name);

	return err;
}