
#include "SiglusDebugger3.h"


DWORD RlVersion(HMODULE mod)
{
    // Get version number of host RealLive instance.
    //HMODULE mod = GetModuleHandleW(NULL);
    LPVOID lres = LockResource(LoadResource(mod,
					    FindResourceW(mod, (LPCWSTR) 1,
							 /*MAKEINTRESOURCEW(16))));*/RT_VERSION)));
	if (lres == NULL) return 0;
    // With any luck, this offset is constant...
    VS_FIXEDFILEINFO *info = (VS_FIXEDFILEINFO*) ((char*) lres + 0x28);
    return (((info->dwFileVersionMS >> 16) & 0xff) << 24)
       	 | ((info->dwFileVersionMS & 0xff) << 16)
       	 | (((info->dwFileVersionLS >> 16) & 0xff) << 8)
       	 | (info->dwFileVersionLS & 0xff);
}


#pragma function(memset)
void * __cdecl memset(void *dst, int val, size_t count)
{
	void *start = dst;
	while (count--) {
		*(char *)dst = (char)val;
		dst = (char *)dst + 1;
	}
	return start;
}

#pragma function(memcpy)
void * __cdecl memcpy(void *dst, const void *src, size_t count)
{
	void *start = dst;
	while (count--) {
		*(char*) dst = *(char*) src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}
	return start;
}

#pragma function(memcmp)
int __cdecl memcmp(const void *ptr1, const void *ptr2, size_t len)
{
	if (len == 0) return 0;

	unsigned char *_ptr1 = (unsigned char *) ptr1;
	unsigned char *_ptr2 = (unsigned char *) ptr2;

	while (--len)
	{
		if (*_ptr1 != *_ptr2) break;
		_ptr1++;
		_ptr2++;
	}

	return *_ptr2 - *_ptr1;
}


BOOL WINAPI CryptEncryptFake(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen)
{
	if (dwBufLen == 64 && !memcmp(pbData, MinorVerData, sizeof(MinorVerData)))
	{
		memcpy(pbData, SLDebugDllID, sizeof(SLDebugDllID));
		*pdwDataLen = 40;
		return TRUE;
	}
	else
	{
		jump_near* lpFunc;

		memcpy(hCryptEncrypt, CryptEncryptOldBytes, 5);

		BOOL ret = hCryptEncrypt(hKey, hHash, Final, dwFlags, pbData, pdwDataLen, dwBufLen);
		
		lpFunc = (jump_near*) hCryptEncrypt;
		lpFunc->opcode = 0xe9;
		lpFunc->relativeAddress = (DWORD) &CryptEncryptFake - ((DWORD) lpFunc + 5);

		return ret;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		hSLModule = GetModuleHandleW(NULL);

		/*DWORD SLVer = RlVersion(hSLModule);
		DWORD SLDebugDllIDAdrR = SLDebugDllIDAdr + (DWORD) hSLModule;
		if (SLVer == 0x01015007) memcpy((void*) SLDebugDllIDAdrR, SLDebugDllIDNew, sizeof(SLDebugDllIDNew));*/

		DWORD dwOldProtect, dwNewProtect, dwDontCare;
		BOOL bVPError = FALSE;
		dwNewProtect = PAGE_EXECUTE_READWRITE;
		jump_near* lpFunc;

		HMODULE hAdvapi = GetModuleHandleW(L"ADVAPI32");
		if (hAdvapi)
		{
			hCryptEncrypt = (CEFUNC) GetProcAddress(hAdvapi, "CryptEncrypt");
			if (hCryptEncrypt)
			{
				if (VirtualProtect((LPVOID) hCryptEncrypt, 5, dwNewProtect, &dwOldProtect))
				{
					memcpy(CryptEncryptOldBytes, hCryptEncrypt, 5);
					lpFunc = (jump_near*) hCryptEncrypt;
					lpFunc->opcode = 0xe9;
					lpFunc->relativeAddress = (DWORD) &CryptEncryptFake - ((DWORD) lpFunc + 5);
					//VirtualProtect((LPVOID) hCryptEncrypt, 5, dwOldProtect, &dwDontCare);
				}
			}
		}

		break;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	
	return TRUE;
}


extern "C" DWORD __cdecl get_major_version(DWORD* dllMajorVer)
{
	memset(dllMajorVer, 0xA0, 32);
	return 3;
}

extern "C" void __cdecl get_minor_version(DWORD* dllMinorVer)
{
	//memset(dllMinorVer, 0xB0, 32);
	memcpy(dllMinorVer, MinorVerData, sizeof(MinorVerData));
}

// Это вроде wstring, но он что-то не пошёл.
struct lv_add_item_002_str {
	WCHAR Text[8];
	DWORD StrLen1;
	DWORD StrLen2;
	//DWORD MaxLen;
};

extern "C" LRESULT __cdecl lb_add_item_001(HWND hWnd, lv_add_item_002_str* TextStr)
{
	LPARAM lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	if (TextStr->StrLen2 < 8) lParam = (LPARAM) TextStr->Text;
	else lParam = *(LPARAM*) TextStr->Text;
	return SendMessageW(hWnd, LB_ADDSTRING, 0, lParam);
}

extern "C" LRESULT __cdecl lb_insert_item_001(HWND hWnd, WPARAM* wParam, lv_add_item_002_str* TextStr)
{
	LPARAM lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	if (TextStr->StrLen2 < 8) lParam = (LPARAM) TextStr->Text;
	else lParam = *(LPARAM*) TextStr->Text;
	return SendMessageW(hWnd, LB_INSERTSTRING, *wParam, lParam);
}

extern "C" LRESULT __cdecl lv_add_item_001(HWND hWnd)
{
	LVITEMW lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	lParam.pszText = L"";
	lParam.mask = LVIF_TEXT;
	lParam.iItem = 65000;
	return SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM) &lParam);
}


extern "C" LRESULT __cdecl lv_add_item_002(HWND hWnd, int arg2, lv_add_item_002_str* TextStr)
//extern "C" LRESULT __cdecl lv_add_item_002(HWND hWnd, int arg2, std::wstring* TextStr)
{

	LVITEMW lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	if (TextStr->StrLen2 < 8) lParam.pszText = TextStr->Text;
	else lParam.pszText = *(WCHAR**) TextStr->Text;
	lParam.mask = LVIF_TEXT;
	lParam.iItem = 65000;
	lParam.iSubItem = 0;
	lParam.lParam = 0;
	return SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM) &lParam);
}

extern "C" LRESULT __cdecl lv_add_item_visible_001(void)
{
	MessageBoxW(NULL, L"Call: lv_add_item_visible_001", L"AltSiglusDebugger3 Warning", MB_OK);
	return 0;
}

extern "C" LRESULT __cdecl lv_insert_item_001(void)
{
	MessageBoxW(NULL, L"Call: lv_insert_item_001", L"AltSiglusDebugger3 Warning", MB_OK);
	return 0;
}

extern "C" LRESULT __cdecl lv_insert_item_002(void)
{
	MessageBoxW(NULL, L"Call: lv_insert_item_002", L"AltSiglusDebugger3 Warning", MB_OK);
	return 0;
}

extern "C" LRESULT __cdecl lv_insert_item_visible_001(void)
{
	MessageBoxW(NULL, L"Call: lv_insert_item_visible_001", L"AltSiglusDebugger3 Warning", MB_OK);
	return 0;
}

extern "C" LRESULT __cdecl lv_reduce_item_001(void)
{
	MessageBoxW(NULL, L"Call: lv_reduce_item_001", L"AltSiglusDebugger3 Warning", MB_OK);
	return 0;
}

extern "C" LRESULT __cdecl lv_set_item_cnt_001(HWND hWnd, WPARAM wParam)
{
	LRESULT rv;
	rv = SendMessageW(hWnd, LVM_GETITEMCOUNT, 0, 0);
	//LRESULT rv = SendMessageW(hWnd, LVM_DELETEALLITEMS, 0, 0);
	//rv = SendMessageW(hWnd, LVM_SETITEMCOUNT, wParam, 0/*lParam*/);
	UINT items = (UINT) rv;
	LVITEMW lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	//lParam.pszText = L"";
	//lParam.mask = LVIF_TEXT;
	if (items < wParam) {
	for (UINT i = items; i < wParam; i++)
	{
		lParam.iItem = i;
		rv = SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM) &lParam);
	}
	}
	else
	{

	for (UINT i = items; i > wParam; i--)
	{
		rv = SendMessageW(hWnd, LVM_DELETEITEM, i - 1, 0);
	}
	}
	return rv;
}

extern "C" LRESULT __cdecl lv_set_cell_text_001(HWND hWnd, int arg2, int arg3, lv_add_item_002_str* TextStr)
{
	LVITEMW lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	if (TextStr->StrLen1 < 8)	lParam.pszText = TextStr->Text;
	else lParam.pszText = *(WCHAR**) TextStr->Text;
	lParam.mask = LVIF_TEXT;
	lParam.iItem = arg2;
	lParam.iSubItem = arg3;	
	return SendMessageW(hWnd, LVM_SETITEMTEXTW, arg2, (LPARAM) &lParam);
}

extern "C" LRESULT __cdecl lv_set_cell_text_visible_001(HWND hWnd, int arg2, int arg3, lv_add_item_002_str* TextStr)
{
	LVITEMW lParam;
	ZeroMemory(&lParam, sizeof(LVITEMW));
	if (TextStr->StrLen1 < 8)	lParam.pszText = TextStr->Text;
	else lParam.pszText = *(WCHAR**) TextStr->Text;
	lParam.mask = LVIF_TEXT;
	lParam.iItem = arg2;
	lParam.iSubItem = arg3;
	SendMessageW(hWnd, LVM_SETITEMTEXTW, arg2, (LPARAM) &lParam);
	return SendMessageW(hWnd, LVM_ENSUREVISIBLE, (WPARAM) arg2, (LPARAM) TRUE);
}