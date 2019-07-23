/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

//#include "common.h"
#include "vasa.h"
//#include "compression.h"

#include "siglus.h"
#include "vadbs.h"

int init_config(void);
void print_help(void);

int wmain(int argc, WCHAR* argv[])
{
	//INT nArgs = 0;
	//LPWSTR lpCommandLine = GetCommandLineW();
	//LPWSTR* lpArgs = CommandLineToArgvW(lpCommandLine, &nArgs);
	//wprintf(L"\r\n");
	wprintf_api(0, L"\r\n");
	wprintf_api(0, L"VASArc 1.20: Visual Art's Script Archiver\r\n");
	wprintf_api(0, L"Copyright (C) 2019 AlexHatsuyuki\r\n\r\n");

	if (argc < 2) {
		wprintf_api(0, L"Usage: vasa <options> <file> <files or directory>\r\n");
		wprintf_api(0, L"Type 'vasa --help' for detailed usage information.\r\n");
		return 0;
	}

	int mode = 0;
	bool encrypt_scene = false;
	bool duplicate_strings = false;
	int compress_lv = 6;
	// 0 - 0, 1 - 256, 2 - 512, 3 - 1024, 4 - 2048,  5 - 4096, 6 - auto
	int game_id = 0;
	wchar_t *dir_name = new wchar_t[MAX_PATH + 1];
	memset(dir_name, 0, MAX_PATH + 1);

	wchar_t argv6[20];
	wchar_t argv7[20];
	wchar_t argv9[20];
	wchar_t game_id_str[20];
	memset(&game_id_str, 0, 20);

	wchar_t **args = new wchar_t* [argc];
	int args_count = 0;
	
	for(int i = 1; i < argc; i++)
	{
		int len = wcslen(argv[i]);
		wcsncpy_s(argv6, 7, argv[i], -1);
		wcsncpy_s(argv7, 8, argv[i], -1);
		wcsncpy_s(argv9, 10, argv[i], -1);
		if (len >= 2 && argv[i][0] == L'-')
		{
			if (_wcsicmp(argv[i], L"-c") == 0 || _wcsicmp(argv[i], L"--create") == 0) {
				mode = 1;
				continue;
			} else if (_wcsicmp(argv[i], L"-a") == 0 || _wcsicmp(argv[i], L"--add") == 0) {
				mode = 2;
				continue;
			} else if (_wcsicmp(argv[i], L"-d") == 0 || _wcsicmp(argv[i], L"--delete") == 0) {
				mode = 3;
				continue;
			} else if (_wcsicmp(argv[i], L"-b") == 0 || _wcsicmp(argv[i], L"--break") == 0) {
				mode = 4;
				continue;
			} else if (_wcsicmp(argv[i], L"-x") == 0 || _wcsicmp(argv[i], L"--extract") == 0) {
				mode = 5;
				continue;
			} else if (_wcsicmp(argv[i], L"-i") == 0 || _wcsicmp(argv[i], L"--info") == 0) {
				mode = 6;
			} else if (_wcsicmp(argv[i], L"-l") == 0 || _wcsicmp(argv[i], L"--list") == 0) {
				mode = 7;
				continue;
			} else if (_wcsicmp(argv[i], L"-v") == 0 || _wcsicmp(argv[i], L"--verbose") == 0) {
				verbose = true;
				continue;
			} else if (_wcsicmp(argv[i], L"-p") == 0) {
				i++; wcscpy(dir_name, argv[i]);
				continue;
			} else if (_wcsicmp(argv6, L"--dir=") == 0) {
				wcscpy(dir_name, argv[i]+6);
				continue;
			} else if (_wcsicmp(argv[i], L"-e") == 0 || _wcsicmp(argv[i], L"--encrypt") == 0) {
				encrypt_scene = true;
				continue;
			} else if (_wcsicmp(argv[i], L"-t") == 0 || _wcsicmp(argv[i], L"--translation-prep") == 0) {
				duplicate_strings = true;
				continue;
			/*} else if (_wcsicmp(argv[i], L"-s") == 0 || _wcsicmp(argv[i], L"--best") == 0) {
				best_compress = true;*/
			} else if (_wcsicmp(argv9, L"--complv=") == 0) {
				wcsncpy_s(argv9, 20, argv[i]+9, -1);
				wchar_t *end_ptr;
				compress_lv = wcstol(argv9, &end_ptr, 10);
				if (errno == 34 || *end_ptr != 0 || compress_lv > 6) compress_lv = 6;
				//best_compress = true;
				continue;
			} else if (_wcsicmp(argv[i], L"-G") == 0) {
				i++; wcsncpy_s(game_id_str, 20, argv[i], -1);
				continue;
			} else if (_wcsicmp(argv7, L"--game=") == 0) {
			wcsncpy_s(game_id_str, 20, argv[i]+7, -1);
			continue;
			} else if (_wcsicmp(argv[i], L"--gamelist") == 0) {
				mode = 11;
				continue;
			} else if (_wcsicmp(argv[i], L"--help") == 0) {
				mode = 10;
				continue;
			} else { wprintf_api(1, L"unknown argument: '%s'.\r\n", argv[i]); }
		} else {
			/*
			if (_wcsicmp(argv[i]+len-3, L"TXT") == 0) {

			} else if (_wcsicmp(argv[i]+len-3, L"PCK") == 0) {

			} else if (_wcsicmp(argv[i]+len-2, L"SS") == 0) {

			}
			*/

			args[args_count] = new wchar_t[MAX_PATH + 1];
			wcsncpy_s(args[args_count], MAX_PATH, argv[i], -1);
			args_count++;
		}
	}

	errno_t err;
	err = init_config();
	//return 0;
	//wchar_t **file_list = 0;
	//std::vector<std::wstring> file_list;
	//int files = get_files_in_dir(L"SceneAB", L"*.slbc", &file_list);

	//for (unsigned int i = 0; i < file_list.size(); i++) wprintf_api(0, L"%s\n", file_list[i].c_str());
	
	//wprintf_api(0, L"%d\n", files);

	//const wchar_t *tmp_str = file_list[0].c_str();
	//wprintf_api(0, L"%s\n", tmp_str);

	//file_list.clear();

	//delete file_list;
	//wprintf_api(0, L"%s\n", tmp_str);

	//return 0;

	if (game_id_str[0] != L'\0')
	{
		for (unsigned int i = 1; i < GameIDs.size(); i++)
		{
			if (wcsicmp(game_id_str, GameIDs[i]) == 0) game_id = i;
		}
		if (!game_id)
		{
			wprintf_api(0, L"Unknown GameID: %s\r\n  Для получения списка поддерживаемых игр,\r\n  запустите программу с ключом '--gamelist'.\r\n", game_id_str);
			return 0;
		}
	}
/*
	if (encrypt_scene && !game_id)
	{
		wprintf_api(1, L"ключ '-e' имеет смысл только совместно с ключом '-G' (--game).\n");
		encrypt_scene = false;
	}
*/
	if (mode == 10)
	{
		print_help();
	}
	else if (mode == 11)
	{
		wprintf_api(0, L"Print game list:\r\n\r\n");
		for (unsigned int i = 1; i < GameIDs.size(); i++)
		{
			wprintf_api(0, L"Name:   %s\r\n", GameNemes[i]);
			wprintf_api(0, L"ID:     %s\r\n", GameIDs[i]);
			wprintf_api(0, L"Vendor: %s\r\n", GameVendors[i]);
			wprintf_api(0, L"Engine: %s\r\n\r\n", GameEngineS[GameEngine[i]]);
		}
	}
	else if (mode == 6 || mode == 7)
	{
		if (args_count < 1) { wprintf_api(2, L"no files to process.\r\n"); return 0; }
		else if (args_count > 1) { wprintf_api(2, L"not allowed a range of files.\r\n"); return 0; }

		wprintf_api(0, L"Read: %s\r\n", args[0]);

		int len = wcslen(args[0]);
		if (len > 4 && _wcsicmp(args[0]+len-4, L".TXT") == 0)
		{

			err = print_rl_arc_info(args[0], mode);
			if (err == 0) wprintf_api(2, L"unknown error.\r\n");

		}
		else if (len > 4 && _wcsicmp(args[0]+len-4, L".PCK") == 0)
		{
			err = print_se_arc_info(args[0], mode);
			if (err == 0) wprintf_api(2, L"unknown error.\r\n");
		}
		else if (len > 4 && _wcsicmp(args[0]+len-4, L".DBS") == 0)
		{
			err = print_dbs_info(args[0], mode);
			if (err == 0) wprintf_api(2, L"unknown error.\r\n");
		}
		else wprintf_api(2, L"unknown file format.\r\n");
	}
	else if (mode == 4 || mode == 5)
	{
		if (args_count < 1) { wprintf_api(2, L"no files to process\r\n"); return 0; }
		else if (args_count > 1) { wprintf_api(2, L"not allowed a range of files\r\n"); return 0; }

		wprintf_api(0, L"Read: %s\r\n", args[0]);

		int len = wcslen(args[0]);
		if (len > 4 && _wcsicmp(args[0]+len-4, L".TXT") == 0)
		{

			//err = print_rl_arc_info(args[0], mode);
			//if (err == 0) wprintf_api(2, L"Unknown error\n");

		}
		else if (len > 4 && _wcsicmp(args[0]+len-4, L".PCK") == 0)
		{
			if (mode == 4)
			{
				wprintf_api(2, L"Архивы Scene.pck (SiglusEngine) нет смысла разбирать на файлы.\r\n  Используйте ключ '-v' (--verbose) для сохранения в файл промежуточных\r\n  результатов.\r\n");
				err = 1;
			}
			else
			{
				err = extract_se_arc(args[0], dir_name, game_id, duplicate_strings);
				if (err == 0) wprintf_api(2, L"unknown error.\r\n");
			}
		}
		else if (len > 4 && _wcsicmp(args[0]+len-4, L".DBS") == 0)
		{
			err = convert_dbs_to_csv(args[0], dir_name);
			if (err == 0) wprintf_api(2, L"unknown error.\r\n");
		}
		else wprintf_api(2, L"unknown file format.\r\n");
	}
	else if (mode == 1)
	{
		if (args_count < 1) { wprintf_api(2, L"no files to process.\r\n"); return 0; }
		//else if (args_count > 1) { wprintf_api(2, L"not allowed a range of files\n"); return 0; }

		//wprintf_api(0, L"Create: %s\n", args[0]);

		int len = wcslen(args[0]);
		/*if (len > 4 && _wcsicmp(args[0]+len-4, L".TXT") == 0)
		{

			//err = print_rl_arc_info(args[0], mode);
			//if (err == 0) wprintf_api(2, L"Unknown error\n");

		}
		else */if (len > 4 && _wcsicmp(args[0]+len-4, L".PCK") == 0)
		{
			err = create_se_arc(args[0], dir_name, compress_lv, game_id, encrypt_scene);
			if (err == 0) wprintf_api(2, L"unknown error.\r\n");
		}
		else wprintf_api(2, L"unknown file format.\r\n");
	}
	else wprintf_api(2, L"unknown command.\r\n");

	delete[] dir_name;
	while (args_count--) delete[] args[args_count];
	delete[] args;
	return 0;
}

int init_config()
{
	errno_t err = -1;

	GameIDs.push_back(L"NONE");
	GameEngine.push_back(0);
	GameNemes.push_back(L"NONE");
	GameVendors.push_back(L"NONE");
	//unsigned char xor_key_NULL[16] = { 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	XorKeys.push_back(xor_key_NULL);

	GameIDs.push_back(L"RW");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Rewrite");
	GameVendors.push_back(L"Key");
	//unsigned char xor_key_RW[16] = { 0x36, 0x0F, 0xC9, 0x6E, 0x2E, 0xD3, 0x8B, 0x0F, 0x66, 0xA5, 0x70, 0xC3, 0x23, 0xA3, 0x6E, 0x94 };
	XorKeys.push_back(xor_key_RW);

	GameIDs.push_back(L"RWHF");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Rewrite Harvest Festa");
	GameVendors.push_back(L"Key");
	//unsigned char xor_key_RWhf[16] = { 0x36, 0x25, 0xca, 0x97, 0x2e, 0xc2, 0x09, 0xb8, 0xe4, 0x63, 0x70, 0xc3, 0x20, 0x8b, 0x6d, 0xeb };
	XorKeys.push_back(xor_key_RWhf);

	GameIDs.push_back(L"RWp");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Rewrite+");
	GameVendors.push_back(L"Key");
	//unsigned char xor_key_RW[16] = { 0x36, 0x0F, 0xC9, 0x6E, 0x2E, 0xD3, 0x8B, 0x0F, 0x66, 0xA5, 0x70, 0xC3, 0x23, 0xA3, 0x6E, 0x94 };
	XorKeys.push_back(xor_key_RWp);

	GameIDs.push_back(L"HS");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Hatsuyuki Sakura");
	GameVendors.push_back(L"Saga Planets");
	//unsigned char xor_key_HS[16] = { 0xdd, 0xe0, 0x2e, 0xf6, 0xde, 0x30, 0xef, 0x64, 0x07, 0x8b, 0x19, 0xa8, 0x4e, 0xd3, 0xbb, 0x14 };
	XorKeys.push_back(xor_key_HS);

	GameIDs.push_back(L"KC");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Karumaruka Circle");
	GameVendors.push_back(L"Saga Planets");
	//unsigned char xor_key_KC[16] = { 0x19, 0x72, 0xa2, 0x11, 0x4f, 0xe5, 0x05, 0xc0, 0x97, 0x6b, 0x6d, 0x7b, 0xeb, 0xb5, 0xb7, 0xfe };
	XorKeys.push_back(xor_key_KC);

	GameIDs.push_back(L"SP");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Seihou no Prismgear");
	GameVendors.push_back(L"Elysion");
	//unsigned char xor_key_SnP[16] = { 0xb4, 0x64, 0x46, 0x4e, 0xac, 0x60, 0x8b, 0xc6, 0xe4, 0x5c, 0xf9, 0x67, 0xae, 0xd4, 0xed, 0xb9 };
	XorKeys.push_back(xor_key_SnP);

	GameIDs.push_back(L"RG");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Rinkai Gakuen");
	GameVendors.push_back(L"Frill");
	//unsigned char xor_key_RG[16] = { 0x5b, 0x30, 0xa7, 0x1c, 0x03, 0xc6, 0x68, 0xc8, 0x85, 0x21, 0x9b, 0x04, 0xcd, 0x19, 0x9f, 0x1d };
	XorKeys.push_back(xor_key_RG);

	GameIDs.push_back(L"SR");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Sousouki Reginald");
	GameVendors.push_back(L"Ocelot");
	//unsigned char xor_key_SR[16] = { 0x38, 0xf1, 0xc2, 0xf0, 0x35, 0x67, 0x08, 0x1b, 0xf0, 0x08, 0xf9, 0x20, 0x20, 0xfc, 0x6f, 0xf0 };
	XorKeys.push_back(xor_key_SR);

	GameIDs.push_back(L"HB");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Holy Breaker! Comiket 87 version");
	GameVendors.push_back(L"H.I. design office");
	//unsigned char xor_key_HB[16] = { 0x7f, 0x0d, 0x88, 0x21, 0x7b, 0xea, 0x41, 0xf3, 0xaa, 0x03, 0xa7, 0x2f, 0xeb, 0x60, 0xad, 0x2e };
	XorKeys.push_back(xor_key_HB);

	GameIDs.push_back(L"ABT10");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Angel Beats! -1st beat- Trial 1.00");
	GameVendors.push_back(L"Key");
	//unsigned char xor_key_ABt10[16] = { 0x57, 0x7E, 0xAF, 0x72, 0x28, 0xB5, 0x37, 0x91, 0xF3, 0x51, 0xF2, 0x71, 0x9F, 0x34, 0xFC, 0x6D };
	XorKeys.push_back(xor_key_ABt10);

	GameIDs.push_back(L"AB1");
	GameEngine.push_back(2);
	GameNemes.push_back(L"Angel Beats! -1st beat-");
	GameVendors.push_back(L"Key");
	//unsigned char xor_key_AB1[16] = { 0x5F, 0x07, 0x8A, 0x2A, 0x66, 0xA7, 0x11, 0xA6, 0x84, 0x6D, 0x9D, 0x46, 0x9F, 0x7A, 0xB9, 0x7E };
	XorKeys.push_back(xor_key_AB1);




	FILE *games_f;
	wchar_t fnbuf[MAX_PATH];
	if (GetModuleFileNameW(NULL, fnbuf, MAX_PATH) != 0)
	{
		//PathRemoveFileSpecW(fnbuf);
		wchar_t *lpslptr = wcsrchr(fnbuf, L'\\');
		if (lpslptr)
		{
			*lpslptr = L'\0';
			wcsncat_s(fnbuf, MAX_PATH, L"\\games.ini", -1);
			err = open_bin_to_read(&games_f, fnbuf);
		}
	}
	if (err <= 0) err = open_bin_to_read(&games_f, L"games.ini");
	if (err <= 0) { if (verbose) wprintf_api(1, L"cannot find games.ini\r\n"); return 1; }

	char *utf8_buf = new char[err + 1];
	ZeroMemory(utf8_buf, err + 1);
	wchar_t *buf = new wchar_t[err + 1];
	//ZeroMemory(buf, err * 2 + 1);

	// skeep UTF-8 BOM
	int utf8bom;
	fread(&utf8bom, 1, 3, games_f);
	utf8bom = utf8bom & 0x00FFFFFF;
	if (utf8bom == 0xBFBBEF) { err -= 3; /* fseek(games_f, 3, SEEK_SET);*/ }
	else fseek(games_f, 0, SEEK_SET);

	err = fread(utf8_buf, 1, err, games_f);
	fclose(games_f);

	SetLastError(ERROR_SUCCESS);
	err = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_buf, err + 1, buf, err + 1);
	if (err == 0 || GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
	{
		wprintf_api(2, L"incorrect text encoding in games.ini\r\n");
		delete[] utf8_buf;
		delete[] buf;
		return 1;
	}
	delete[] utf8_buf;

	wchar_t *iniptr = buf;
	wchar_t initstr[MAX_PATH];

	err = 0;

	int section = 0;
	int sections = 0;
	wchar_t game_id_tmp[10];
	wchar_t game_engine_tmp[10];
	wchar_t game_name_tmp[MAX_PATH];
	wchar_t game_vendor_tmp[MAX_PATH];
	wchar_t xor_key_tmp[MAX_PATH];
	//ZeroMemory(game_id_tmp, 10);
	*(int*) game_id_tmp = 0;
	*(int*) game_engine_tmp = 0;
	*(int*) game_name_tmp = 0;
	*(int*) game_vendor_tmp = 0;
	*(int*) xor_key_tmp = 0;

	while (get_string(initstr, MAX_PATH, iniptr))
	{
		/*
		wchar_t *inistr = initstr;
		while (is_space_w(*inistr)) inistr++;
		int l = wcslen(inistr);
		while (is_space_w(inistr[l - 1])) inistr[--l] = 0;
		if (*inistr == L'\0') continue;
		if (inistr[0] == L'/' && inistr[1] == L'/') continue;
		*/

		wchar_t *inistr = wcstrim(wcsslccut(initstr));

		//wprintf(L"|%s|\n", inistr);
		//continue;

		if (wcscmp(inistr, L"GameEnd") == 0)
		{
			int nonunique = 0;

			for (unsigned int i = 0; i < GameIDs.size(); i++)
			{
				if (wcsicmp(GameIDs[i], game_id_tmp) == 0) { nonunique = 1; }
			}

			if (game_id_tmp[0] != L'\0' && !nonunique)
			{
			wchar_t *temp_str;
			wchar_t *end_ptr;

			temp_str = new wchar_t[16];
			wcsncpy_s(temp_str, 16, game_id_tmp, -1);
			GameIDs.push_back(temp_str);
			//wprintf_api(0, L"%s\n", GameIDs.back());

			char temp_ge_id = wcstoul(game_engine_tmp, &end_ptr, 10);
			GameEngine.push_back(temp_ge_id);
			//wprintf_api(0, L"%d\n", GameEngine.back());
			
			temp_str = new wchar_t[MAX_PATH];
			wcsncpy_s(temp_str, MAX_PATH, game_name_tmp, -1);
			GameNemes.push_back(temp_str);
			//wprintf_api(0, L"%s\n", GameNemes.back());

			temp_str = new wchar_t[MAX_PATH];
			wcsncpy_s(temp_str, MAX_PATH, game_vendor_tmp, -1);
			GameVendors.push_back(temp_str);
			//wprintf_api(0, L"%s\n", GameVendors.back());

			unsigned char* temp_xor = new unsigned char[16];
			ZeroMemory(temp_xor, 16);		
			XorKeys.push_back(temp_xor);
			int keylen = 0;
			end_ptr = xor_key_tmp;
			while (end_ptr && keylen < 16)
			{
				temp_xor[keylen] = wcstoul(end_ptr, &end_ptr, 16);
				if (errno == 34) temp_xor[keylen] = 0x00;
				//wprintf_api(0, L"0x%02X", temp_xor[len]);
				keylen++;
			}

			//wprintf_api(0, L"%s\n\n", xor_key_tmp);
			//sections++;
			}
			else if (nonunique)
			{
				wprintf_api(1, L"corrypt section %d. GameID '%s' non-unique.\r\n", sections + 1, game_id_tmp);
			}
			else wprintf_api(1, L"corrypt section %d. GameID missing.\r\n", sections + 1);
			sections++;
			section = 0;
			*(int*) game_id_tmp = 0;
			*(int*) game_engine_tmp = 0;
			*(int*) game_name_tmp = 0;
			*(int*) game_vendor_tmp = 0;
			*(int*) xor_key_tmp = 0;
			continue;
		}
		else if (wcscmp(inistr, L"GameStart") == 0)
		{
			if (section) wprintf_api(1, L"expacted GameStart\r\n");
			section = 1;
			continue;
		}

		if (section)
		{
			wchar_t *str_var = 0;
			str_var = get_str_val(L"GameID", inistr);
			if (str_var) wcsncpy_s(game_id_tmp, 10, str_var, -1);

			str_var = get_str_val(L"GameEngine", inistr);
			if (str_var) wcsncpy_s(game_engine_tmp, 10, str_var, -1);

			str_var = get_str_val(L"GameName", inistr);
			if (str_var) wcsncpy_s(game_name_tmp, MAX_PATH, str_var, -1);

			str_var = get_str_val(L"Vendor", inistr);
			if (str_var) wcsncpy_s(game_vendor_tmp, MAX_PATH, str_var, -1);

			str_var = get_str_val(L"XorKey", inistr);
			if (str_var) wcsncpy_s(xor_key_tmp, MAX_PATH, str_var, -1);

			//wprintf_api(0, L"%s\n", inistr);
		}

		//wprintf_api(0, L"%s\n", inistr);
		if (err != 0) break;
	}

	delete[] buf;
	return 0;
}




//// ---- SEEN.TXT ----
int read_chek_seen_head(FILE *seen_txt_f, int file_size, SeenTXThead *seen_header, int *seen_count, int *seen_size)
{
	/*
	Зоголовок является таблицей фиксированного размера на 10000 файлов.
	Формат: DWORD offset
			DWORD size
	Следовательно, размер корректного файла не может быть меньше 80000 байт.
	Файлы адресуются индексами в архиве.
	Если файл с данным индексом отсутствует, то offet и size равно 0.
	*/
	if (file_size < 80000) return 0;
	fread(seen_header, sizeof(SeenTXThead), 10000, seen_txt_f);
	for (int i = 0; i < 10000; i++)
	{
		if (seen_header[i].offset != 0)
		{
			if (i > 0)
			{
				if (seen_header[i].offset >= file_size) return 0;
				if (seen_header[i].offset <= seen_header[i-1].offset) return 0;
			}
			if (seen_header[i].size == 0) return 0;
			*seen_count = *seen_count + 1;
			*seen_size = *seen_size + seen_header[i].size;
		}
		else
		{
			if (seen_header[i].size != 0) return 0;
		}
	}
	return *seen_count;
}

int print_rl_arc_info(wchar_t *file_name, int mode)
{
	errno_t err;
	FILE* txt_f;
	err = open_bin_to_read(&txt_f, file_name);
	if (err <= 0) { wprintf_api(2, L"Cannot open file: %s\n", file_name); return 0; }

	int file_size = err;
	int seen_count = 0;
	int seen_size = 0;
	
	SeenTXThead *seen_header = new SeenTXThead[10000];
	//ZeroMemory(seen_header, sizeof(SeenTXThead)*10000);

	err = read_chek_seen_head(txt_f, file_size, seen_header, &seen_count, &seen_size);
	if (err == 0) { wprintf_api(2, L"Invalid RealLive archive\n"); fclose(txt_f); return 0; }

	wchar_t *buf = new wchar_t[MAX_PATH];
	wprintf_api(0, L"Archive type: RealLive SEEN.TXT\r\n");
	hum_size(buf, file_size);
	wprintf_api(0, L"Archive size: %s (%u B)\r\n", buf, file_size);
	wprintf_api(0, L"Files count:  %u\r\n", seen_count);
	hum_size(buf, seen_size);
	wprintf_api(0, L"Files size:   %s (%u B)\r\n", buf, seen_size);
	if (mode == 6)
	{
		wprintf_api(0, L"\r\n  File list\r\n");
		wprintf_api(0, L"| File    | Offet   | Size    |\r\n");
		for (int i = 0; i < 10000; i++)
		{
			if (seen_header[i].offset != 0)
			{
				wprintf_api(0, L"| SEEN%04u| %8u| %8u|\r\n", i, seen_header[i].offset, seen_header[i].size);
			}
		}
		wprintf_api(0, L"\r\n  %s in %u files\r\n", buf, seen_count);
	}
	delete[] buf;

	delete[] seen_header;
	fclose(txt_f);
	return 1;
}


void print_help(void)
{
	wprintf_api(0, L"Usage: vasa <options> <file> <files or directory>\r\n");
	wprintf_api(0, L"Info:  RealLive not supported in this version.\r\n");
	wprintf_api(0, L"\r\n");
	wprintf_api(0, L"          --help              display this usage information\r\n");
	wprintf_api(0, L"-v        --verbose           describe what vasa is doing\r\n");
	wprintf_api(0, L"\r\n");

	wprintf_api(0, L"-i        --info              display archive info\r\n");
	wprintf_api(0, L"-l        --list              list archive contents\r\n");

	wprintf_api(0, L"-c        --create            create new archive\r\n");
	wprintf_api(0, L"-a        --add               add to or update files in archive\r\n");
	wprintf_api(0, L"                              (for RealLive only)\r\n");
	wprintf_api(0, L"-d        --delete            remove files from archive\r\n");
	wprintf_api(0, L"                              (for RealLive only)\r\n");
	wprintf_api(0, L"-x        --extract           extract and decompress files\r\n");
	wprintf_api(0, L"-b        --break             extract files without decompressing them\r\n");
	wprintf_api(0, L"                              (for RealLive only)\r\n");
	wprintf_api(0, L"\r\n");

	wprintf_api(0, L"-p DIR    --dir=DIR           read/write files in DIR\r\n");
	wprintf_api(0, L"-G GID    --game=GID          GameID game-specific encryption select\r\n");
	wprintf_api(0, L"-e        --encrypt           Create encrypting archive\r\n");
	wprintf_api(0, L"                              (for SiglusEngine only)\r\n");
	wprintf_api(0, L"          --complv=lv         Compression level: 0-6\r\n");
	wprintf_api(0, L"                              0 - min, 5 - max, 6 - auto\r\n");
	wprintf_api(0, L"          --gamelist          Show supported games\r\n");
	wprintf_api(0, L"-t        --translation-prep  output strings twince (first commented)\r\n");
	wprintf_api(0, L"                              (for SiglusEngine only)\r\n");
	wprintf_api(0, L"\r\n");
}

