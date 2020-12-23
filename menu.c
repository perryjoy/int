

#include <windows.h>
#include "menu.h"
#include "menuID.h"
#include "text_display.h"
#include "error.h"
#include "text.h"
#include "main.h"


static text_to_display** ttd;

HMENU CreateMyMenu(HINSTANCE hThisInstance)
{
	HMENU res = LoadMenu(hThisInstance, MAKEINTRESOURCE(ID_MENU));
	EnableMenuItem(res, ID_MENU_FILE_OPEN, MF_GRAYED);
	EnableMenuItem(res, ID_MENU_MODE_ASIS, MF_GRAYED);
	return res;
}

BOOL ActivateSubItem(HMENU m, int id)
{
	return EnableMenuItem(m, id, MF_BYPOSITION | MF_ENABLED);
}

BOOL DisactivateSubItem(HMENU m, int id)
{
	return EnableMenuItem(m, id, MF_BYPOSITION | MF_GRAYED);
}

int OpenText(HWND myWin)
{
	int res;
	char filenameBuf[512] = { 0 };
	char* entry = NULL;
	size_t text_size;

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = myWin;
	ofn.lpstrFilter = "Text files(*.txt)\0 * .txt\0";
	ofn.lpstrFile = filenameBuf;
	ofn.nMaxFile = 256;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;



	res = GetOpenFileName(&ofn);

	if (res != 0)
	{
		DestroyText(ttd);
		#ifdef UNICODE
		if (ReadToBufferW((ofn.lpstrFile), &entry, &text_size) == DONE)
		#else
        if (ReadToBufferA((ofn.lpstrFile), &entry, &text_size) == DONE)
        #endif // UNICODE
		{
			InitTxt(entry, text_size, ttd);
			SetText(myWin, *ttd);
			free(entry);
			entry = NULL;
			OnSize(myWin);
		}
	}
	return res;
}

int CloseText(HWND myWin)
{
	DestroyText(ttd);
	SetText(myWin, NULL);
	OnSize(myWin);
}


int OnCommand (HWND myWin, int code)
{
	RECT r;
	HMENU m = GetMenu(myWin);
	switch (code)
	{
	case ID_MENU_FILE_OPEN:
		if (OpenText(myWin) == 0)
			break;
		EnableMenuItem(m, ID_MENU_FILE_OPEN, MF_GRAYED);
		EnableMenuItem(m, ID_MENU_FILE_CLOSE, MF_ENABLED);
		ActivateSubItem(m, ID_MENU_MODE);
		SetDisplayMode(myWin, TDM_ASIS);
		EnableMenuItem(m, ID_MENU_MODE_ASIS, MF_GRAYED);
		EnableMenuItem(m, ID_MENU_MODE_LAYOUT, MF_ENABLED);
		break;

	case ID_MENU_FILE_CLOSE:
		CloseText(myWin);
		EnableMenuItem(m, ID_MENU_FILE_CLOSE, MF_GRAYED);
		EnableMenuItem(m, ID_MENU_FILE_OPEN, MF_ENABLED);
		DisactivateSubItem(m, ID_MENU_MODE);

		break;

	case ID_MENU_FILE_EXIT:
		SendMessage(myWin, WM_DESTROY, 0, 0);
		break;

	case ID_MENU_MODE_ASIS:
		SetDisplayMode(myWin, TDM_ASIS);
		EnableMenuItem(m, ID_MENU_MODE_ASIS, MF_GRAYED);
		EnableMenuItem(m, ID_MENU_MODE_LAYOUT, MF_ENABLED);
		GetClientRect(myWin, &r);
		InvalidateRect(myWin, &r, 1);
		break;

	case ID_MENU_MODE_LAYOUT:
		SetDisplayMode(myWin, TDM_FITTED);
		EnableMenuItem(m, ID_MENU_MODE_LAYOUT, MF_GRAYED);
		EnableMenuItem(m, ID_MENU_MODE_ASIS, MF_ENABLED);
		GetClientRect(myWin, &r);
		InvalidateRect(myWin, &r, 1);
		break;

	default:
		break;
	}
	return DONE;
}

void RegisterTextPtrKostyl(text_to_display** t)
{
	ttd = t;
}
