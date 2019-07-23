/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

#include "common.h"

/*
Стандартный порядок секций
1) stringtable
2) stringdata
3) bytecode
4) labels
5) entrypoints
6) functions
7) local_vars
8) local_varsnt
9) local_varsn
10) local_func
11) local_funcnt
12) local_funcn
13) call_varsnt
14) call_vatsn
15) Unknown15
16) savepoints
*/

#pragma pack(push, 1)
struct SceneSSHead
{
	int		headersize;		// 0x00	 // normal header size 132 bytes
	FileTable bytecode;		// 0x04
	FileTable stringtable;	// 0x0c
	FileTable stringdata;	// 0x14
	FileTable labels;		// 0x1c
	FileTable entrypoints;	// 0x24
	FileTable functions;	// 0x2c // functions (all, local&global) (offset only)
	FileTable local_vars;	// 0x34 // local variables (variable type table)
	FileTable local_varsnt;	// 0x3c // string index, names for local variables in local_vars
	FileTable local_varsn;	// 0x44 // string data, names for local variables in local_vars
	FileTable local_func;	// 0x4c // local functions (id and offset)
	FileTable local_funcnt;	// 0x54 // string index, names for local functions in local_func
	FileTable local_funcn;	// 0x5c // string data, names for local functions in local_func
	FileTable call_varsnt;	// 0x64 // string index, names for call variables
	FileTable call_varsn;	// 0x6c // string data, names for call variables
	FileTable Unknown15;	// 0x74
	FileTable savepoints;	// 0x7c // savepoints
};

/*
struct SSHead
{
	int headersize;			// 0x00	 // normal header size 132 bytes
	//int BytecodeOffset;
	//int BytecodeSize;
	FileTable bytecode;		// 0x04
	//int StringTOffset;
	//int StringTCount;
	FileTable stringtable;	// 0x0c
	//int StringsOffset;
	//int StringsCount;
	FileTable stringdata;	// 0x14
	//int Unknown4Offset;
	//int Unknown4Size;
	FileTable labels;		// 0x1c
	//int Unknown5Offset;
	//int Unknown5Size;
	FileTable entrypoints;	// 0x24
	//int Unknown6Offset;
	//int Unknown6Size;
	FileTable Unknown6;		// 0x2c // some kind of label. possibly functions or variables of some kind?
	//int Unknown7Offset;
	//int Unknown7Size;
	FileTable Unknown7;		// 0x34 // variable type table?
	//int Unknown8Offset;
	//int Unknown8Size;
	FileTable Unknown8;		// 0x3c // string index
	//int Unknown9Offset;
	//int Unknown9Size;
	FileTable Unknown9;		// 0x44 // string table
	//int Unknown10Offset;
	//int Unknown10Size;
	FileTable Unknown10;	// 0x4c // corresponds with the stuff in Unknown6. only offsets
	//int Unknown11Offset;
	//int Unknown11Size;
	FileTable Unknown11;	// 0x54 // string index
	//int Unknown12Offset;
	//int Unknown12Size;
	FileTable Unknown12;	// 0x5c // string table
	//int Unknown13Offset;
	//int Unknown13Size;
	FileTable Unknown13;	// 0x64 // string index
	//int Unknown14Offset;
	//int Unknown14Size;
	FileTable Unknown14;	// 0x6c // string table
	//int Unknown15Offset;
	//int Unknown15Size;
	FileTable Unknown15;	// 0x74
	//int Unknown16Offset;
	//int Unknown16Size;
	FileTable Unknown16;	// 0x7c // savepoints
};
*/
#pragma pack(pop)
/*
int readstrings(FileTable *stringtable, wchar_t *stringdata, int stringcount, wchar_t **&outstringarr)
{
	//if (stringtable.size != stringdata.size) return 1;
	//if (stringtable.size == 0) return 0;
	if (stringcount == 0) return 0;

	//FileTable *string_table = stringtable.offset;
	//memset(string_table, 0, stringtable.size * sizeof(FileTable));

	outstringarr = new wchar_t*[stringcount];
	ZeroMemory(outstringarr, stringcount * sizeof(wchar_t*));

	int i = 0;
	for (i = 0; i < stringcount; i++)
	{
		outstringarr[i] = new wchar_t[stringtable[i].size + 1];
		wchar_t *instr = stringdata + stringtable[i].offset;
		wchar_t *outstr = outstringarr[i];
		for (int j = 0; j < stringtable[i].size; j++) *outstr++ = *instr++;
		*outstr = NULL;
	}

	//delete[] string_table;
	return i;
}
*/
int unpackstrings(FileTable *stringtable, wchar_t *stringdata, int *stringdatalen, int stringcount, wchar_t ***stringarr, int encrypt)
{
	if (stringtable == NULL || stringdata == NULL || stringcount == 0 || stringarr == NULL) return -1;

	int strdatalen = 0;
	wchar_t **outstringarr = NULL;

	if (*stringarr == NULL) outstringarr = new wchar_t*[stringcount];
	else outstringarr = *stringarr;
	//wchar_t **outstringarr = (wchar_t**) malloc(stringcount * sizeof(wchar_t*));
	memset(outstringarr, 0, stringcount * sizeof(wchar_t*));

	//wchar_t *stringdataw = (wchar_t*) stringdata;

	int i = 0;
	for (i = 0; i < stringcount; i++)
	{
		if (stringtable[i].offset < 0) break;
		if (stringdatalen && *stringdatalen > 0) if (stringdata + stringtable[i].offset + stringtable[i].size > stringdata + *stringdatalen) break;
		outstringarr[i] = new wchar_t[stringtable[i].size + 1];
		//*outstringarr[i] = (wchar_t*) malloc((stringtable[i].size + 1) * sizeof(wchar_t));
		wchar_t *instr = stringdata + stringtable[i].offset;
		wchar_t *outstr = outstringarr[i];
		if (encrypt) for (int j = 0; j < stringtable[i].size; j++) *outstr++ = *instr++ ^ (i * 0x7087);
		else for (int j = 0; j < stringtable[i].size; j++) *outstr++ = *instr++;
		*outstr = NULL;
		strdatalen += stringtable[i].size;
	}

	*stringarr = outstringarr;
	if (stringdatalen) *stringdatalen = strdatalen;
	return i;
}

int packstrings(FileTable **stringtable, wchar_t **stringdata, int *stringdatalen, int stringcount, wchar_t **stringarr, int encrypt)
{
	if (stringtable == NULL || stringdata == NULL || stringcount == 0 || stringarr == NULL) return -1;
	
	int i = 0;
	int strdatalen = 0;
	FileTable *outstringtable = NULL;
	wchar_t *outstringdata = NULL;

	if (*stringtable == NULL) outstringtable = new FileTable[stringcount];
	else outstringtable = *stringtable;
	//FileTable *outstringtable = (FileTable*) malloc(stringcount * sizeof(FileTable));
	memset(outstringtable, 0, stringcount * sizeof(FileTable));

	if (stringdatalen && *stringdatalen > 0)
	{
		strdatalen = *stringdatalen;
		if (*stringdata == NULL) outstringdata = new wchar_t[strdatalen];
		else outstringdata = *stringdata;
	}
	else
	{
		for (i = 0; i < stringcount; i++)
		{
			//outstringtable[i].offset = 0;
			//outstringtable[i].size = wcslen(stringarr[i]);
			strdatalen += wcslen(stringarr[i]);
		}
		outstringdata = new wchar_t[strdatalen];
	}

	//wchar_t *outstringdata = (wchar_t*) malloc(stringcount * sizeof(wchar_t));
	memset(outstringdata, 0, strdatalen * sizeof(wchar_t));

	wchar_t *outstr = outstringdata;

	for (i = 0; i < stringcount; i++)
	{
		wchar_t *instr = stringarr[i];
		outstringtable[i].size = wcslen(instr);
		if (outstr + outstringtable[i].size > outstringdata + strdatalen) break;
		outstringtable[i].offset = (outstr - outstringdata); // / sizeof(wchar_t);
		if (encrypt) for (int j = 0; j < outstringtable[i].size; j++) *outstr++ = *instr++ ^ (i * 0x7087);
		else for (int j = 0; j < outstringtable[i].size; j++) *outstr++ = *instr++;
	}

	*stringtable = outstringtable;
	*stringdata = outstringdata;
	if (stringdatalen) *stringdatalen = strdatalen;
	return i;
}

void freestrings(int stringcount, wchar_t **stringarr)
{
	if (stringcount == 0 || stringarr == NULL) return;
	while (stringcount--) if (stringarr[stringcount] != NULL) delete[] stringarr[stringcount];
	//while (stringcount--) if (stringarr[stringcount] != NULL) free(stringarr[stringcount]);
	delete[] stringarr;
	//free(stringarr);
}

wchar_t* convertescapetotag(wchar_t *instr)
{
	if (instr == NULL) return NULL;
	int slen = wcslen(instr);

	wchar_t *outstr = new wchar_t[(slen + 1) * 2];

	wchar_t *str1ptr = instr;
	wchar_t *str2ptr = outstr;

	if (*str1ptr == L' ') *str2ptr++ = L'\\';
	while (*str1ptr)
	{
		if (*str1ptr == L'\r') { *str2ptr++ = L'\\'; *str2ptr++ = L'r'; str1ptr++; }
		else if (*str1ptr == L'\n') { *str2ptr++ = L'\\'; *str2ptr++ = L'n'; str1ptr++; }
		else if (*str1ptr == L'\\') { *str2ptr++ = L'\\'; *str2ptr++ = L'\\'; str1ptr++; }
		//else if (*str1ptr == L'\t') { *str2ptr++ = L'\\'; *str2ptr++ = L't'; str1ptr++; }
		else *str2ptr++ = *str1ptr++;
	}
	if (*(str2ptr - 1) == L' ') *str2ptr++ = L'\\';
	*str2ptr = L'\0';

	return outstr;
}

wchar_t* converttagtoescape(wchar_t *instr)
{
	if (instr == NULL) return NULL;
	int slen = wcslen(instr);

	wchar_t *outstr = new wchar_t[slen + 1];

	wchar_t *str1ptr = instr;
	wchar_t *str2ptr = outstr;

	while (*str1ptr)
	{
		if (*str1ptr == L'\\')
		{
			str1ptr++;
			if (*str1ptr == L'\0') break;
			if (*str1ptr == L'r') { *str2ptr++ = L'\r'; str1ptr++; }
			else if (*str1ptr == L'n') { *str2ptr++ = L'\n'; str1ptr++; }
			else *str2ptr++ = *str1ptr++;
		}
		else *str2ptr++ = *str1ptr++;
	}
	*str2ptr = L'\0';

	return outstr;
}

int dumptext(unsigned char *scriptdata, int scriptsize, wchar_t *dir_name, wchar_t *b_file_name, bool dubl)
{
	if (scriptsize <= 0x84)  { if (verbose) wprintf_api(2, L"incorrect bytecode file.\r\n"); return 1; }

	//SSHead *SceneHeader = (SSHead*) scriptdata;
	SceneSSHead SceneHeader;
	memcpy(&SceneHeader, scriptdata, sizeof(SceneSSHead));
	if (SceneHeader.headersize != 0x84) { if (verbose) wprintf_api(2, L"incorrect bytecode file.\r\n"); return 1; }

	if (SceneHeader.stringtable.size == 0) return 0;

	FileTable *string_table = (FileTable*) (scriptdata + SceneHeader.stringtable.offset);
	wchar_t *string_data = (wchar_t*) (scriptdata + SceneHeader.stringdata.offset);

	//int str_encrypt = 0;
	//if (string_table[0].offset != 0) str_encrypt = 1;

	wchar_t file_name[MAX_PATH];
	int slen = swprintf(file_name, MAX_PATH, L"%s\\%s-str.txt", dir_name, b_file_name);
	if (slen == -1) { if (verbose) wprintf_api(2, L"path is very long\r\n"); return 1; }

	errno_t err;
	FILE *txt_f;
	err = _wfopen_s(&txt_f, file_name, L"wb");
	if (err != 0) { if (verbose) wprintf_api(2, L"could not write text file: %s\r\n", file_name); return 1; }

	//fwrite("\xFF\xFE", 1, 2, txt_f);

	wchar_t **textstrings = NULL;
	int string_data_len = scriptsize / sizeof(wchar_t);

	if (unpackstrings(string_table, string_data, &string_data_len, SceneHeader.stringtable.size, &textstrings, 1) != SceneHeader.stringtable.size)
	{
		freestrings(SceneHeader.stringtable.size, textstrings);
		fclose(txt_f);
		wprintf_api(2, L"could not unpack strings in file: %s\r\n", b_file_name);
		return 1;
	}

	for (int i = 0; i < SceneHeader.stringtable.size; i++)
	{
		/*
		wchar_t *str1 = new wchar_t[string_table[i].size + 1];
		memset(str1, 0, (string_table[i].size + 1) * 2);
		memcpy(str1, scriptdata + SceneHeader.stringdata.offset + string_table[i].offset * 2, string_table[i].size * 2);

		//if (str_encrypt) 
		for (int j = 0; j < string_table[i].size; j++) str1[j] = str1[j] ^ (i * 0x7087);

		wchar_t *str2 = new wchar_t[(wcslen(str1) + 1) * 2];
		wchar_t *str1ptr = str1;
		wchar_t *str2ptr = str2;

		if (*str1ptr == L' ') *str2ptr++ = L'\\';
		while (*str1ptr)
		{
			if (*str1ptr == L'\r') { *str2ptr++ = L'\\'; *str2ptr++ = L'r'; str1ptr++; }
			else if (*str1ptr == L'\n') { *str2ptr++ = L'\\'; *str2ptr++ = L'n'; str1ptr++; }
			else if (*str1ptr == L'\\') { *str2ptr++ = L'\\'; *str2ptr++ = L'\\'; str1ptr++; }
			//else if (*str1ptr == L'\t') { *str2ptr++ = L'\\'; *str2ptr++ = L't'; str1ptr++; }
			else *str2ptr++ = *str1ptr++;
		}
		if (*(str2ptr - 1) == L' ') *str2ptr++ = L'\\';
		*str2ptr = L'\0';
		*/

		wchar_t *str2 = convertescapetotag(textstrings[i]);

		size_t out_str_len = (wcslen(str2) + 1) * 2 + 100;
		wchar_t *out_str = new wchar_t[out_str_len];
		//wsprintf(out_str, L"// <%04d> %s\r\n<%04d> %s\r\n\r\n", i, str2, i, str2); // The maximum size of the buffer is 1024 chars.
		if (dubl) swprintf(out_str, out_str_len, L"// <%04d> %s\r\n<%04d> %s\r\n\r\n", i, str2, i, str2);
		else swprintf(out_str, out_str_len, L"<%04d> %s\r\n", i, str2);

		char *utf8str = new char[(wcslen(out_str) + 1) * 4];
		WideCharToMultiByte(CP_UTF8, NULL, out_str, -1, utf8str, (wcslen(out_str) + 1) * 4, NULL, NULL);
		fwrite(utf8str, sizeof(char), strlen(utf8str), txt_f);

		//fwrite(out_str, sizeof(wchar_t), wcslen(out_str), txt_f);


		delete[] utf8str;
		delete[] out_str;
		delete[] str2;
		//delete[] str1;
	}

	freestrings(SceneHeader.stringtable.size, textstrings);

/*
	if (SceneHeader.Unknown8.size > 0)
	{
		if (SceneHeader.Unknown8.size != SceneHeader.Unknown9.size) { wprintf_api(2, L"Unknown8 != Unknown9r\n"); return 1; };
		wchar_t **strings = 0;
		FileTable *string_table = (FileTable*) (scriptdata + SceneHeader.Unknown8.offset);
		int reads = readstrings(string_table, (wchar_t*) (scriptdata + SceneHeader.Unknown9.offset), SceneHeader.Unknown8.size, strings);
		
		fwrite("\r\n", 1, 2, txt_f);

		for(int i = 0; i < SceneHeader.Unknown8.size; i++)
		{
			wchar_t *unistr = new wchar_t[wcslen(strings[i]) + 20 + 1];
			wsprintf(unistr, L"[%04d] %s\r\n", i, strings[i]);
			char *utf8str = new char[(wcslen(unistr) + 1) * 3];
			utf8str[0] = 0;
			WideCharToMultiByte(CP_UTF8, NULL, unistr, -1, utf8str, (wcslen(unistr) + 1) * 3, NULL, NULL);
			fwrite(utf8str, sizeof(char), strlen(utf8str), txt_f);
			//fwrite(unistr, sizeof(wchar_t), wcslen(unistr), txt_f);
			delete[] utf8str;
			delete[] unistr;
			delete[] strings[i];
		}

		delete[] strings;
	}

	if (SceneHeader.Unknown11.size > 0)
	{
		if (SceneHeader.Unknown11.size != SceneHeader.Unknown12.size) { wprintf_api(2, L"Unknown11 != Unknown12r\n"); return 1; };
		wchar_t **strings = 0;
		//FileTable *string_table = (FileTable*) (scriptdata + SceneHeader.Unknown11.offset);
		int reads = readstrings((FileTable*) (scriptdata + SceneHeader.Unknown11.offset), (wchar_t*) (scriptdata + SceneHeader.Unknown12.offset), SceneHeader.Unknown11.size, strings);
		
		fwrite("\r\n", 1, 2, txt_f);

		for(int i = 0; i < SceneHeader.Unknown11.size; i++)
		{
			wchar_t *unistr = new wchar_t[wcslen(strings[i]) + 10 + 1];
			wsprintf(unistr, L"{%04d} %s\r\n", i, strings[i]);
			char *utf8str = new char[(wcslen(unistr) + 1) * 3];
			WideCharToMultiByte(CP_UTF8, NULL, unistr, -1, utf8str, (wcslen(unistr) + 1) * 3, NULL, NULL);
			fwrite(utf8str, sizeof(char), strlen(utf8str), txt_f);
			//fwrite(unistr, sizeof(wchar_t), wcslen(unistr), txt_f);
			delete[] utf8str;
			delete[] unistr;
			delete[] strings[i];
		}

		delete[] strings;
	}

	if (SceneHeader.Unknown13.size > 0)
	{
		if (SceneHeader.Unknown13.size != SceneHeader.Unknown14.size) { wprintf_api(2, L"Unknown13 != Unknown14r\n"); return 1; };
		wchar_t **strings = 0;
		//FileTable *string_table = (FileTable*) (scriptdata + SceneHeader.Unknown11.offset);
		int reads = readstrings((FileTable*) (scriptdata + SceneHeader.Unknown13.offset), (wchar_t*) (scriptdata + SceneHeader.Unknown14.offset), SceneHeader.Unknown13.size, strings);
		
		fwrite("\r\n", 1, 2, txt_f);

		for(int i = 0; i < SceneHeader.Unknown13.size; i++)
		{
			wchar_t *unistr = new wchar_t[wcslen(strings[i]) + 10 + 1];
			wsprintf(unistr, L"(%04d) %s\r\n", i, strings[i]);
			char *utf8str = new char[(wcslen(unistr) + 1) * 3];
			WideCharToMultiByte(CP_UTF8, NULL, unistr, -1, utf8str, (wcslen(unistr) + 1) * 3, NULL, NULL);
			fwrite(utf8str, sizeof(char), strlen(utf8str), txt_f);
			//fwrite(unistr, sizeof(wchar_t), wcslen(unistr), txt_f);
			delete[] utf8str;
			delete[] unistr;
			delete[] strings[i];
		}

		delete[] strings;
	}

	fprintf(txt_f, "\r\n");

	if (SceneHeader.labels.size > 0)
	{
		fprintf(txt_f, "labels: 0x%02X, %d\r\n", SceneHeader.labels.offset, SceneHeader.labels.size);
	}

	if (SceneHeader.entrypoints.size > 0)
	{
		fprintf(txt_f, "entrypoints: 0x%02X, %d\r\n", SceneHeader.entrypoints.offset, SceneHeader.entrypoints.size);
	}

	if (SceneHeader.Unknown6.size > 0)
	{
		fprintf(txt_f, "Unknown6: 0x%02X, %d\r\n", SceneHeader.Unknown6.offset, SceneHeader.Unknown6.size);
	}

	if (SceneHeader.Unknown7.size > 0)
	{
		fprintf(txt_f, "Unknown7: 0x%02X, %d\r\n", SceneHeader.Unknown7.offset, SceneHeader.Unknown7.size);
	}

	if (SceneHeader.Unknown10.size > 0)
	{
		fprintf(txt_f, "Unknown10: 0x%02X, %d\r\n", SceneHeader.Unknown10.offset, SceneHeader.Unknown10.size);
	}

	if (SceneHeader.Unknown15.size > 0)
	{
		fprintf(txt_f, "Unknown15: 0x%02X, %d\r\n", SceneHeader.Unknown15.offset, SceneHeader.Unknown15.size);
	}

	if (SceneHeader.Unknown16.size > 0)
	{
		fprintf(txt_f, "Unknown16: 0x%02X, %d\r\n", SceneHeader.Unknown16.offset, SceneHeader.Unknown16.size);
	}
*/
	fclose(txt_f);
	
	if (verbose) wprintf_api(0, L"Dumped strings: %d\r\n", SceneHeader.stringtable.size);

	return 0;
}


int loadtext(unsigned char *&scriptdata, int *scriptsize, wchar_t *dir_name, wchar_t *b_file_name)
{
	if (*scriptsize <= 0x84)  { if (verbose) wprintf_api(1, L"incorrect bytecode file.\r\n"); return 1; }

	SceneSSHead SceneHeader;
	memcpy(&SceneHeader, scriptdata, sizeof(SceneSSHead));
	if (SceneHeader.headersize != 0x84) { if (verbose) wprintf_api(1, L"incorrect bytecode file.\r\n"); return 1; }

	//if (SceneHeader.stringtable.offset > SceneHeader.stringdata.offset || SceneHeader.stringdata.offset > SceneHeader.bytecode.offset) { wprintf_api(2, L"нестандартный порядок секций. Обновление текста не выполнено\r\n  в %s.\r\n", b_file_name); return 1; }

	if (SceneHeader.stringtable.size == 0) return 0;
	if (SceneHeader.stringtable.size != SceneHeader.stringdata.size) { if (verbose) wprintf_api(1, L"incorrect string table.\r\n"); return 1; }

	wchar_t file_name[MAX_PATH];
	int slen = swprintf(file_name, MAX_PATH, L"%s\\%s-str.txt", dir_name, b_file_name);
	if (slen == -1) { if (verbose) wprintf_api(1, L"path is very long\r\n"); return 1; }

	errno_t err;
	FILE *txt_f;
	err = open_bin_to_read(&txt_f, file_name);
	if (err <= 0) { if (verbose) wprintf_api(1, L"could not read text file: %s.\r\n", file_name); return 1; }

	char *utf8buf = new char[err + 1];
	memset(utf8buf, 0, err + 1);
	wchar_t *buf = new wchar_t[err + 1];
	//memset(buf, 0, err * 2 + 1);

	// check BOM
	int utfbom;
	fread(&utfbom, 4, 1, txt_f);
	if (utfbom == 0x0000FEFF || utfbom == 0xFFFE0000 || (utfbom & 0x0000FFFF) == 0xFEFF || (utfbom & 0x0000FFFF) == 0xFFFE) 
	{
		wprintf_api(2, L"incorrect codepage. Support UTF-8 only.\r\n");
		return 1;
	}
	// skeep UTF-8 BOM
	if ((utfbom & 0x00FFFFFF) == 0xBFBBEF) { err -= 3; fseek(txt_f, 3, SEEK_SET); }
	else fseek(txt_f, 0, SEEK_SET);

	err = fread(utf8buf, 1, err, txt_f);
	fclose(txt_f);

	SetLastError(ERROR_SUCCESS);
	err = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8buf, err + 1, buf, err + 1);
	if (err == 0 || GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
	{
		if (verbose) wprintf_api(1, L"incorrect text encoding in text file: %s.\r\n", b_file_name);
		delete[] utf8buf;
		delete[] buf;
		return 1;
	}
	delete[] utf8buf;

	wchar_t *bufptr = buf;

	wchar_t **new_strings = new wchar_t*[SceneHeader.stringtable.size];
	memset(new_strings, 0, SceneHeader.stringtable.size * sizeof(wchar_t*));

	wchar_t *strtbuf = new wchar_t[32768];
	
	int read_str_in_file = 0;

	while (get_string(strtbuf, 32768, bufptr))
	{
		read_str_in_file++;

		wchar_t *bufstr = wcsslccut(strtbuf);
		bufstr = wcstrim(bufstr);

		if (*bufstr == L'\0') continue;
		//if (bufstr[0] == L'/' && bufstr[1] == L'/') continue;

		while (*bufstr != L'\0' && *bufstr != L'<') bufstr++;
		if (*bufstr == L'\0') continue;
		if (*bufstr == L'<') bufstr++;

		errno = 0;
		int str_key = wcstol(bufstr, &bufstr, 10);
		if (errno == 34) { wprintf_api(1, L"incorrect key in %s line %d\r\n", b_file_name, read_str_in_file); continue; }
		if (*bufstr == L'>') bufstr++;
		else { wprintf_api(1, L"incorrect key in %s line %d\r\n", b_file_name, read_str_in_file); continue; }
		
		if (str_key < 0 || str_key >= SceneHeader.stringtable.size) { wprintf_api(1, L"incorrect key <%04d> in %s line %d? out of rage\r\n", str_key, b_file_name, read_str_in_file); continue; }
		if (new_strings[str_key] != 0) { wprintf_api(1, L"duplicate key <%04d> in %s line %d\r\n", str_key, b_file_name, read_str_in_file); continue; }

		bufstr = wcstrim(bufstr);
		new_strings[str_key] = converttagtoescape(bufstr);
		/*
		new_strings[str_key] = new wchar_t[wcslen(bufstr) + 1];
		memset(new_strings[str_key], 0, (wcslen(bufstr) + 1) * sizeof(wchar_t));
		wchar_t *bufstrptr = bufstr;
		wchar_t *newstrptr = new_strings[str_key];
		while(*bufstrptr)
		{
			if (*bufstrptr == L'\\')
			{
				bufstrptr++;
				if (*bufstrptr == L'\0') break;
				if (*bufstrptr == L'r') { *newstrptr++ = L'\r'; bufstrptr++; }
				else if (*bufstrptr == L'n') { *newstrptr++ = L'\n'; bufstrptr++; }
				else *newstrptr++ = *bufstrptr++;
			}
			else *newstrptr++ = *bufstrptr++;
		}
		*newstrptr = L'\0';
		*/
						
		//wprintf_api(0, L"%s\r\n", new_strings[str_key]);
	}

	delete[] strtbuf;
	delete[] buf;

	//wchar_t **old_strings = new wchar_t*[SceneHeader.StringsCount];
	//ZeroMemory(old_strings, SceneHeader.StringsCount * sizeof(wchar_t*));

	// Создаём копию таблицы строк
	FileTable *string_table = new FileTable[SceneHeader.stringtable.size * sizeof(FileTable)];
	memcpy(string_table, (scriptdata + SceneHeader.stringtable.offset), SceneHeader.stringtable.size * sizeof(FileTable));

	//int str_encrypt = 0;
	//if (string_table[0].offset != 0) str_encrypt = 1;

	int oldfullstrlen = 0;
	int newfullstrlen = 0;
	// Восстанавливаем строки которые отсутствовали в текстовом файле
	for (int i = 0; i < SceneHeader.stringtable.size; i++)
	{
		oldfullstrlen += string_table[i].size;
		if (new_strings[i] == 0)
		{
			wchar_t *str = new wchar_t[string_table[i].size + 1];
			memset(str, 0, (string_table[i].size + 1) * 2);
			memcpy(str, scriptdata + SceneHeader.stringdata.offset + string_table[i].offset * 2, string_table[i].size * 2);

			//if (str_encrypt) 
			for (int j = 0; j < string_table[i].size; j++) str[j] = str[j] ^ (i * 0x7087);

			new_strings[i] = str;
		}

		newfullstrlen += wcslen(new_strings[i]);

		//wprintf_api(0, L"%s\r\n", new_strings[i]);
	}


	int newoffsetforall = newfullstrlen * 2 - oldfullstrlen * 2;

	int newscriptsize = *scriptsize + newoffsetforall;
	
	unsigned char *newscriptdata = new unsigned char[newscriptsize];

	unsigned char *newscriptdata_ptr = newscriptdata;

	//newscriptdata_ptr += 0x84 + SceneHeader.StringTCount * sizeof(FileTable);

	//wchar_t *newtextdata = (wchar_t*) (newscriptdata + 0x84 + SceneHeader.stringtable.size * sizeof(FileTable));
	wchar_t *newtextdata = (wchar_t*) (newscriptdata + SceneHeader.stringdata.offset);
	wchar_t *newtext_ptr = newtextdata;

	/*
	for (int i = 0; i < SceneHeader.stringtable.size; i++)
	{
		string_table[i].offset = newtext_ptr - newtextdata;
		string_table[i].size = wcslen(new_strings[i]);
		wchar_t *str_ptr = new_strings[i];
		//wchar_t *newstr_ptr = newtext_ptr;
		//while(*str_ptr) *newtext_ptr++ = *str_ptr++;
		//if (str_encrypt) 
		//for (int j = 0; j < string_table[i].size; j++) newstr_ptr[j] = newstr_ptr[j] ^ (i * 0x7087);
		for (int j = 0; j < string_table[i].size; j++) *newtext_ptr++ = *str_ptr++ ^ (i * 0x7087);
	}
	*/
	if (packstrings(&string_table, &newtext_ptr, &newfullstrlen, SceneHeader.stringtable.size, new_strings, 1) != SceneHeader.stringtable.size)
	{
		freestrings(SceneHeader.stringtable.size, new_strings);
		delete[] newscriptdata;
		delete[] string_table;
		wprintf_api(2, L"could not pack strings in file: %s\r\n", b_file_name);
		return 1;
	}

	if (SceneHeader.stringdata.offset < SceneHeader.bytecode.offset)
	{
	int oldBytecodeOffset = SceneHeader.bytecode.offset;

	SceneHeader.bytecode.offset += newoffsetforall;
	SceneHeader.labels.offset += newoffsetforall;
	SceneHeader.entrypoints.offset += newoffsetforall;
	SceneHeader.functions.offset += newoffsetforall;
	SceneHeader.local_vars.offset += newoffsetforall;
	SceneHeader.local_varsnt.offset += newoffsetforall;
	SceneHeader.local_varsn.offset += newoffsetforall;
	SceneHeader.local_func.offset += newoffsetforall;
	SceneHeader.local_funcnt.offset += newoffsetforall;
	SceneHeader.local_funcn.offset += newoffsetforall;
	SceneHeader.call_varsnt.offset += newoffsetforall;
	SceneHeader.call_varsn.offset += newoffsetforall;
	SceneHeader.Unknown15.offset += newoffsetforall;
	SceneHeader.savepoints.offset += newoffsetforall;
	
	memcpy(newscriptdata + SceneHeader.bytecode.offset, scriptdata + oldBytecodeOffset, *scriptsize - oldBytecodeOffset);
	}
	else
	{
		memcpy(newscriptdata, scriptdata, SceneHeader.stringtable.offset);
	}

	memcpy(newscriptdata, &SceneHeader, sizeof(SceneSSHead));
	memcpy(newscriptdata + SceneHeader.stringtable.offset, string_table, SceneHeader.stringtable.size * sizeof(FileTable));

	delete[] scriptdata;
	scriptdata = newscriptdata;
	*scriptsize = newscriptsize;

	//while (SceneHeader.stringtable.size--) if (new_strings[SceneHeader.stringtable.size]) delete[] new_strings[SceneHeader.stringtable.size];
	//delete[] new_strings;
	freestrings(SceneHeader.stringtable.size, new_strings);
	delete[] string_table;

	return 0;
}