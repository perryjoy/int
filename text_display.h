#define TRYING_FOR_THE_MARK

#ifndef TEXT_DISPLAY_H_INCLUDED
#define TEXT_DISPLAY_H_INCLUDED
#include <windows.h>

typedef struct text_to_display text_to_display;
typedef struct display_meta display_meta;


enum arrow_codes // values ment to be connected with "||" in order to report arrow pressing
{
    ARROW_UP__ = 1,
    ARROW_DOWN__ = 2,
    ARROW_LEFT__ = 4,
    ARROW_RIGHT__ = 8
};

enum button_codes // values ment to be connected with "||" in order to report keyboard button pressing
{
  BUTTON_PGUP__ = 1,
  BUTTON_PGDWN__ = 2,
  BUTTON_ESC__ = 4
};

typedef enum text_display_mode
{
  TDM_FITTED,  // layout text
  TDM_ASIS // text as it is
} text_display_mode;

/*return value is text_display_err value = DONE or error code
text_to_display* src = text we are going to display
HWND hwnd = handler of the window we are going to use to display
Function attaches display_meta_ptr to the window*/
int InitDisplay (HWND hwnd);

void DestroyDisplay (HWND hwnd);

int OnPaint (HWND myWin);
int OnSize (HWND myWin);
int OnArrow (HWND myWin, int arrow_code);
int OnButton(HWND myWin, int button_code);
int OnScrollX(HWND myWin, int winCode);
int OnScrollY(HWND myWin, int winCode);
int SetDisplayMode(HWND myWin, text_display_mode m);
int SetText(HWND myWin, text_to_display* t);




#endif // TEXT_DISPLAY_H_INCLUDED
