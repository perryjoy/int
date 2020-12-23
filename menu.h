
#pragma once
#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

typedef struct text_to_display text_to_display;

HMENU CreateMyMenu(HINSTANCE hThisInstance); // provides us with the properly inited (firstly grayed) menu to use
int OnCommand(HWND myWin, int code); // handles the menu messages from the window in which menu usage occured
void RegisterTextPtrKostyl (text_to_display** t); // stores text (not display) structure in order to free it when its needed as long as we dont develop texts managing system

#endif //MENU_H_INCLUDED
