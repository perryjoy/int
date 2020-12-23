#include "text_display.h"
#include "text.h"
#include "error.h"
#include <windows.h>
#include <commdlg.h>

#define REPAINT_ALL__ 666
#define MAX_SCROLLBAR_VAL 32767

    #ifndef TRYING_FOR_THE_MARK
    #define MOVETEXT(entry,b,c,d) MoveText(entry, b, c, d)
    #else
    #define MOVETEXT(entry,b,c,d) MoveText(entry, b, c, d); \
    FixScrollVals(entry); \
    entry->t_info.siX.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS; \
    SetScrollInfo(myWin, SB_HORZ, &(entry->t_info.siX), TRUE); \
    entry->t_info.siY.fMask = SIF_RANGE | SIF_PAGE | SIF_POS| SIF_TRACKPOS; \
    SetScrollInfo(myWin, SB_VERT, &(entry->t_info.siY), TRUE); \
    SendMessage(myWin, WM_NCPAINT, 0, 0)
    #endif // TRYING_FOR_THE_MARK




typedef struct text_scroll_info
{
    text_to_display* text; // text we're responsible to draw
    size_t current_string;   // current string
    size_t current_first_symbol; // first symbol in current string to appear on the screeen
    SCROLLINFO siX;
    SCROLLINFO siY;
    SIZE scroll_multiplier; // coefficent wich serves correct scrollbar usage
}text_scroll_info;


struct display_meta  // structure which is used to display text and navigate through it
{
    text_scroll_info t_info;  // all data about position in text

    text_display_mode mode; // layout or not

    HDC context_to_draw; // virtual DC that we are drawing to
    HBITMAP bitmap; // 1x1 bitmap handler: we use it to swap back to release the memory on delete
    HFONT text_style; // dummy font handler: we use it to swap back to release the memory on delete
    HBRUSH background_filler; // dummy brush handler: we use it to swap back to release the memory on delete

    SIZE textwin_limits;  // window size of virtual DC we're using to draw to
    SIZE textwin_symb_size; // number of symbols we can fit into the window with size set above
};

void SwapBitmap (display_meta* dm)
{
    dm->bitmap = (HBITMAP) SelectObject(dm->context_to_draw, dm->bitmap);
}

void SwapFont (display_meta* dm)
{
    dm->text_style = (HFONT) SelectObject(dm->context_to_draw, dm->text_style);
}

void SwapBrush(display_meta* dm)
{
  dm->background_filler = (HBRUSH)SelectObject(dm->context_to_draw, dm->background_filler);
}


LOGFONT DEFAULT_FONT = { 30, /*height in pixels*/
20, /*width in pixels*/
0,  /*text rotation angle in 0.1 grad*/
0,  /*symbol rotation angle in 0.1 grad*/
FW_NORMAL, /*weight (boldness) of font: between 0 and 1000*/
0, /*if not 0, set Italic  flag*/
0, /*if not 0, set Bold  flag*/
0, /*if not 0, set Strikeout  flag*/
ANSI_CHARSET, /*charset, one of: ANSI_CHARSET, DEFAULT_CHARSET, SYMBOL_CHARSET, SHIFTJIS_CHARSET (japan), OEM_CHARSET*/
OUT_DEFAULT_PRECIS, /*one of the OUT_* constants, setting preferable font*/
CLIP_DEFAULT_PRECIS, /*one of the CLIP_* constants, setting behavior for the symbols being cut off by the rect border*/
DEFAULT_QUALITY,  /*one of the DEFAULT_QUALITY, DRAFT_QUALITY (bad), PROOF_QUALITY (good) constants */
FIXED_PITCH || FF_DONTCARE, /*width of symbols: DEFAULT_PITCH (no care), FIXED_PITCH, VARIABLE_PITCH   OR   font family parameter: FF_DONTCARE, FF_DECORATIVE, FF_MODERN, FF_ROMAN, FF_SCRIPT, FF_SWISS*/
"" //"Arial"/*string, containing font name*/
} ;


size_t DMShiftStrBeginPos (display_meta *dm, size_t times, int direction)
{
    if (dm->mode == TDM_ASIS)
    {
      if (direction == BACKWARD__)
      {
        times = min(times, dm->t_info.current_string);
        dm->t_info.current_string -= times;
      }
      if (direction == FORWARD__)
      {
        times = min(times, GetStrCount(dm->t_info.text) - dm->t_info.current_string - 1);
        dm->t_info.current_string += times;
      }
      return times;
    }
    if (dm->mode == TDM_FITTED)
      return ShiftStrBeginPos(dm->t_info.text, &(dm->t_info.current_string), &(dm->t_info.current_first_symbol), dm->textwin_limits.cx / dm->textwin_symb_size.cx, times, direction);
    return 0;
}

size_t DMShiftFirstSymbolPos(display_meta* dm, size_t times, int direction)
{
  if (direction == BACKWARD__)
  {
    if (dm->t_info.current_first_symbol < times)
    {
      times = dm->t_info.current_first_symbol;
    }
    dm->t_info.current_first_symbol -= times;
  }
  if (direction == FORWARD__)
  {
    if (GetMaxLen(dm->t_info.text) <= dm->t_info.current_first_symbol + times + 1)
    {
      times = GetMaxLen(dm->t_info.text) - dm->t_info.current_first_symbol - 2; // one for the indexes shift and another for the /n symbol
    }
    dm->t_info.current_first_symbol += times;
  }
  return times;
}


void InitDMPixLimits(display_meta* dm, HWND hwnd)
{
    RECT winR;
    GetClientRect(hwnd, &winR);
    dm->textwin_limits.cy = winR.bottom-winR.top;
    dm->textwin_limits.cx = winR.right-winR.left;
}

void FixScrollVals(display_meta* dm)
{
  if (dm->t_info.text==NULL)
    return;


  if (dm->mode == TDM_FITTED) // Ive decided not to use scrollbars via layout mode. It should be done in text module otherwise (or would take a tone of time and/or memory to do)
  {
      dm->t_info.siX.nMax = 0;
      dm->t_info.siX.nPos = 0;
      dm->t_info.siY.nMax = 0;
      dm->t_info.siY.nPos = 0;
      return;
  }

  size_t textWidth = GetMaxLen(dm->t_info.text);
  size_t textHeight = GetStrCount(dm->t_info.text);
  size_t screenWidth = dm->textwin_limits.cx / dm->textwin_symb_size.cx;
  size_t screenHeight = dm->textwin_limits.cy / dm->textwin_symb_size.cy;
  size_t totalWidth = textWidth + screenWidth;
  size_t totalHeight = textHeight + (dm->mode == TDM_FITTED ? GetTextLen(dm->t_info.text) / screenWidth : 0) + screenHeight;

  dm->t_info.scroll_multiplier.cx = totalWidth / MAX_SCROLLBAR_VAL + (totalWidth % MAX_SCROLLBAR_VAL == 0 ? 0 : 1);
  dm->t_info.scroll_multiplier.cy = totalHeight / MAX_SCROLLBAR_VAL + (totalHeight % MAX_SCROLLBAR_VAL == 0 ? 0 : 1);
  dm->t_info.siY.nMax = totalHeight/dm->t_info.scroll_multiplier.cy;;


  if (dm->t_info.scroll_multiplier.cx != 0 && dm->t_info.scroll_multiplier.cy != 0)
  {
    if (dm->mode == TDM_ASIS)
    {
      dm->t_info.siX.nMax = totalWidth/dm->t_info.scroll_multiplier.cx;
      dm->t_info.siX.nPos = dm->t_info.current_first_symbol / dm->t_info.scroll_multiplier.cx;
      dm->t_info.siY.nPos = dm->t_info.current_string / dm->t_info.scroll_multiplier.cy;
    }
    if (dm->mode == TDM_FITTED)
    {
      dm->t_info.siX.nMax = 0;
      dm->t_info.siX.nPos = 0;
      dm->t_info.siY.nPos = (dm->t_info.current_string + (GetTextLenUpToStr(dm->t_info.text, dm->t_info.current_string) + dm->t_info.current_first_symbol) / textWidth)  / dm->t_info.scroll_multiplier.cy;// + (dm->t_info.current_string == 0 && dm->t_info.current_first_symbol == 0 ? 0 : 1);
    }
    dm->t_info.siX.nPage = screenWidth;
    dm->t_info.siY.nPage = screenHeight;
  }
  else
  {
    dm->t_info.siX.nPos = 0;
    dm->t_info.siY.nPos = 0;
    dm->t_info.siX.nPage = 0;
    dm->t_info.siY.nPage = 0;
  }

    dm->t_info.siX.nTrackPos = dm->t_info.siX.nPos;
    dm->t_info.siY.nTrackPos = dm->t_info.siY.nPos;

}

void InitDMText(display_meta* dm, text_to_display *t)
{
    dm->t_info.text = t;
    dm->t_info.current_string = 0;
    dm->t_info.current_first_symbol = 0;

    dm->t_info.siX.cbSize = sizeof(SCROLLINFO);
    dm->t_info.siY.cbSize = sizeof(SCROLLINFO);
    dm->t_info.siX.nMin = 0;
    dm->t_info.siY.nMin = 0;

    FixScrollVals(dm);
}

int SetDMSymbSize(display_meta* dm)
{
    TEXTMETRIC tm;
    if (dm == NULL)
        return MEM_ERR_NULLPTR_DEREFERENCING;

    if (GetTextMetrics(dm->context_to_draw, &tm) == FALSE) // getting text metrics
        return WINAPI_ERR_HDC_DATA_UNREACHED;

    dm->textwin_symb_size.cx = tm.tmAveCharWidth;
    dm->textwin_symb_size.cy = tm.tmHeight;

    return DONE;
}



int InitDisplayMeta (HWND hwnd, display_meta *meta_to_set)
{
    if (meta_to_set == NULL)
        return MEM_ERR_NULLPTR_DEREFERENCING;
    if (!IsWindow(hwnd))
        return WINAPI_ERR_WIN_HANGING_HANDLER;

    meta_to_set->t_info.text = NULL;
    meta_to_set->t_info.current_first_symbol=0;
    meta_to_set->t_info.current_string = 0;

    InitDMPixLimits(meta_to_set, hwnd);

    meta_to_set->mode = TDM_ASIS;

    meta_to_set->text_style = CreateFontIndirect(&DEFAULT_FONT);
    meta_to_set->background_filler = CreateSolidBrush(RGB(0,0,0));

    HDC winDC = GetDC(hwnd);
    meta_to_set->context_to_draw=CreateCompatibleDC(winDC); // creating memory DC
    meta_to_set->bitmap = CreateCompatibleBitmap(winDC, meta_to_set->textwin_limits.cx, meta_to_set->textwin_limits.cy); // setting up bitmap for memory DC
    ReleaseDC(hwnd, winDC);

    SetMapMode(meta_to_set->context_to_draw, MM_TEXT); // logical units are the pixels now :)
    SwapFont(meta_to_set); // selecting the font, saving the previous font handler to swap back on delete
    SwapBitmap(meta_to_set); // selecting the bitmap, saving 1x1 bitmap to swap back on delete
    SwapBrush(meta_to_set); // selecting the brush, saving the previous brush handler to swap back on delete


    return SetDMSymbSize(meta_to_set);
}


int InitDisplay (HWND hwnd)
{
    int ret_code;
    display_meta* to_create;

    if ((display_meta *)GetWindowLongPtrA(hwnd, GWLP_USERDATA) != NULL)
        return MEM_ERR_HANGING_POINTER_OCCURED;

    to_create = malloc(sizeof(display_meta));

    if (to_create == NULL)
      return MEM_ERR_FAILED_TO_ALLOCATE;

    ret_code = InitDisplayMeta(hwnd, to_create);
    if (ret_code != DONE)
    {
      free(to_create);
      REPORT(ret_code)
    }
    else
    {
      SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)to_create);
    }
    return ret_code;
}



void DestroyDisplayMeta (display_meta* dm)
{
    if (dm == NULL)
        return;

    SwapFont(dm); //swapping back actual font we've created
    SwapBitmap(dm); //swapping back actual bitmap we've created
    SwapBrush(dm); //swapping back actual brush we've created
    DeleteDC(dm->context_to_draw); // deleting memory DC with 1x1 bitmap

    DeleteObject(dm->bitmap); // clearing up for ourselves - bitmap
    DeleteObject(dm->text_style); // clearing up for ourselves - font
    DeleteObject(dm->background_filler); // clearing up for ourselves - brush
}


void DestroyDisplay (HWND hwnd)
{
    DestroyDisplayMeta((display_meta*)GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, 0);
}



/*Draws layout text to memory DC in given area*/
int PaintTextToMemDCFitted(display_meta* dm, RECT* area, size_t shift)
{
  char* output;
  int curr_pen_pos_x = area->left,
    curr_pen_pos_y = area->top;
  size_t strings_left_to_display = (area->bottom - area->top) / dm->textwin_symb_size.cy,
    symbols_in_string = (area->right - area->left) / dm->textwin_symb_size.cx;

  size_t curr_str = dm->t_info.current_string,
    curr_symb = dm->t_info.current_first_symbol;


  size_t symbols_read;
  size_t symb_to_paint;


  SetTextColor(dm->context_to_draw, RGB(200, 0, 0));

  symbols_read = GetTextString(dm->t_info.text, &output, curr_str, curr_symb);
  while (shift > 0)
  {
    symb_to_paint = (symbols_read > symbols_in_string ? symbols_in_string : symbols_read);
    symbols_read -= symb_to_paint;
    if (symbols_read == 0)
    {
      curr_str++;
      symbols_read = GetTextString(dm->t_info.text, &output, curr_str, 0);
    }
    shift--;
  }
  while (symbols_read != 0 && strings_left_to_display > 0)
  {
    symb_to_paint = (symbols_read > symbols_in_string ? symbols_in_string : symbols_read);


    TextOutA(dm->context_to_draw, curr_pen_pos_x, curr_pen_pos_y, output, symb_to_paint - (symb_to_paint == symbols_read ? 0 : 0));

    curr_pen_pos_y += dm->textwin_symb_size.cy;
    output += symb_to_paint;
    symbols_read -= symb_to_paint;
    strings_left_to_display--;
    if (symbols_read == 0)
    {
      curr_str++;
      symbols_read = GetTextString(dm->t_info.text, &output, curr_str, 0);
    }
  }

  return DONE;
}


/*Draws text as it is to memory DC in given area*/
int PaintTextToMemDCAsIs(display_meta* dm, RECT* area, size_t shiftY, size_t shiftX)
{
  char* output = NULL;
  int curr_pen_pos_x = area->left,
    curr_pen_pos_y = area->top;
  size_t strings_left_to_display = (area->bottom - area->top) / dm->textwin_symb_size.cy,
    symbols_in_string = (area->right - area->left) / dm->textwin_symb_size.cx;

  size_t curr_str = dm->t_info.current_string + shiftY,
    curr_symb = dm->t_info.current_first_symbol + shiftX;

  size_t symbols_read;



  SetTextColor(dm->context_to_draw, RGB(200, 0, 0));


  symbols_read = GetTextString(dm->t_info.text, &output, curr_str, curr_symb);
  while (strings_left_to_display > 0)
  {
    TextOutA(dm->context_to_draw, curr_pen_pos_x, curr_pen_pos_y, output, min(symbols_read, symbols_in_string - (symbols_in_string == symbols_read ? 1 : 0)));

    curr_pen_pos_y += dm->textwin_symb_size.cy;
    strings_left_to_display--;
    curr_str++;
    symbols_read = GetTextString(dm->t_info.text, &output, curr_str, curr_symb);
  }

  return DONE;
}

int PaintTextDM(display_meta* dm, RECT* area, size_t shiftStr, size_t shiftSymb)
{
  RECT fullrect = { 0, 0, dm->textwin_limits.cx, dm->textwin_limits.cy };
  if (area == NULL)
    area = &fullrect;
  Rectangle(dm->context_to_draw, area->left, area->top, area->right, area->bottom);

  if (dm->mode == TDM_FITTED)
  {
    return PaintTextToMemDCFitted(dm, area, shiftStr);
  }
  if (dm->mode == TDM_ASIS)
    return PaintTextToMemDCAsIs(dm, area, shiftStr, shiftSymb);

}

RECT InitDisturbedArea(SIZE winLimits, SIZE letterSize, size_t timesToMove, int direction, int movingLeftRightFlag)
{
  LONG top = 0;
  LONG bot = winLimits.cy;
  LONG left = 0;
  LONG right = winLimits.cx;

  if (movingLeftRightFlag == 0)
  {
    if (direction == FORWARD__)
    {
      top = ((winLimits.cy / letterSize.cy) - timesToMove) * letterSize.cy;
      bot = winLimits.cy;
      left = 0;
      right = winLimits.cx;
    }
    if (direction == BACKWARD__)
    {
      top = 0;
      bot = timesToMove * letterSize.cy;
      left = 0;
      right = winLimits.cx;
    }
  }
  else
  {
    if (direction == FORWARD__)
    {
      top = 0;
      bot = winLimits.cy;
      left = ((winLimits.cx / letterSize.cx) - timesToMove) * letterSize.cx;
      right = winLimits.cx;
    }
    if (direction == BACKWARD__)
    {
      top = 0;
      bot = winLimits.cy;
      left = 0;
      right = timesToMove * letterSize.cx;
    }
  }

  RECT res = { left, top, right, bot };
  return res;
}

int MoveText (display_meta* dm, size_t timesToMove, int direction, int movingLeftRightFlag)
{
    if (dm->mode == TDM_FITTED && movingLeftRightFlag != 0)
      return DONE;

    RECT areaToRepaint;

    if (movingLeftRightFlag == 0)
    {
      timesToMove = DMShiftStrBeginPos(dm, timesToMove, direction);

      #ifdef TRYING_FOR_THE_MARK
      return PaintTextDM(dm, NULL, 0, 0);
      #else
      areaToRepaint = InitDisturbedArea(dm->textwin_limits, dm->textwin_symb_size, timesToMove, direction, movingLeftRightFlag);
      ScrollDC(dm->context_to_draw, 0, dm->textwin_symb_size.cy * (int)timesToMove * (direction == FORWARD__ ? -1 : (direction == BACKWARD__ ? 1 : 0)), NULL, NULL, 0, 0);
      if (direction == BACKWARD__)
      {
        Rectangle(dm->context_to_draw, 0, dm->textwin_limits.cy / dm->textwin_symb_size.cy * dm->textwin_symb_size.cy, dm->textwin_limits.cx, dm->textwin_limits.cy);
      }

      return PaintTextDM(dm, &areaToRepaint, (direction == BACKWARD__ ? 0: (dm->textwin_limits.cy / dm->textwin_symb_size.cy < timesToMove ? 0 : dm->textwin_limits.cy / dm->textwin_symb_size.cy - timesToMove)), 0);
      #endif // TRYING_FOR_THE_MARK
    }
    else
    {
      timesToMove= DMShiftFirstSymbolPos(dm, timesToMove, direction);

      #ifdef TRYING_FOR_THE_MARK
      return PaintTextDM(dm, NULL, 0, 0);
      #else
      areaToRepaint = InitDisturbedArea(dm->textwin_limits, dm->textwin_symb_size, timesToMove, direction, movingLeftRightFlag);
      ScrollDC(dm->context_to_draw, dm->textwin_symb_size.cx * (int)timesToMove * (direction == FORWARD__ ? -1 : (direction == BACKWARD__ ? 1 : 0)), 0, NULL, NULL, 0, 0);
      if (direction == BACKWARD__)
      {
        Rectangle(dm->context_to_draw, dm->textwin_limits.cx / dm->textwin_symb_size.cx * dm->textwin_symb_size.cx, 0, dm->textwin_limits.cx, dm->textwin_limits.cy);
      }
      return PaintTextDM(dm, &areaToRepaint, 0, (direction == BACKWARD__ ? 0 : (dm->textwin_limits.cx / dm->textwin_symb_size.cx < timesToMove ? 0 : dm->textwin_limits.cx / dm->textwin_symb_size.cx - timesToMove)));
      #endif // TRYING_FOR_THE_MARK
    }


}

int OnScrollX(HWND myWin, int winCode)
{
  size_t shift;
  int direction;
  display_meta* dm = ((display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA));
  if (dm == NULL)
    return DONE;
  dm->t_info.siX.cbSize = sizeof(SCROLLINFO);
  dm->t_info.siX.fMask = SIF_POS| SIF_TRACKPOS;
  GetScrollInfo(myWin, SB_HORZ, &(dm->t_info.siX));
  int oldPos = dm->t_info.siX.nPos;

  dm->t_info.siX.cbSize = sizeof(SCROLLINFO);
  dm->t_info.siX.fMask = SIF_ALL;
  switch (winCode)
  {
  case SB_LINELEFT:
    shift = dm->t_info.scroll_multiplier.cx;
    direction = BACKWARD__;
    break;

  case SB_LINERIGHT:
    shift = dm->t_info.scroll_multiplier.cx;
    direction = FORWARD__;
    break;

  case SB_PAGELEFT:
    shift = dm->t_info.siX.nPage;
    direction = BACKWARD__;
    break;

  case SB_PAGERIGHT:
    shift = dm->t_info.siX.nPage;
    direction = FORWARD__;
    break;

  case SB_THUMBTRACK:
    if (dm->t_info.siX.nPos > dm->t_info.siX.nTrackPos)
    {
        shift = (dm->t_info.siX.nPos - dm->t_info.siX.nTrackPos) * dm->t_info.scroll_multiplier.cx;
        direction = BACKWARD__;
    }
    else
    {
        shift = (dm->t_info.siX.nTrackPos - dm->t_info.siX.nPos) * dm->t_info.scroll_multiplier.cx;
        direction = FORWARD__;
    }
    break;

  default:
    break;
  }

  MOVETEXT(dm, shift, direction, 1);


}

int OnScrollY(HWND myWin, int winCode)
{
  size_t shift = 0;
  int direction;
  display_meta* dm = ((display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA));
  if (dm == NULL)
    return DONE;
  dm->t_info.siY.cbSize = sizeof(SCROLLINFO);
  dm->t_info.siY.fMask = SIF_TRACKPOS;
  GetScrollInfo(myWin, SB_VERT, &(dm->t_info.siY));
  int oldPos = dm->t_info.siY.nPos;

  dm->t_info.siY.cbSize = sizeof(SCROLLINFO);
  dm->t_info.siY.fMask = SIF_ALL;
  switch (winCode)
  {
  case SB_TOP:
    shift = dm->t_info.scroll_multiplier.cy * dm->t_info.siY.nMax;
    direction = BACKWARD__;
    break;

  case SB_BOTTOM:
    shift = dm->t_info.scroll_multiplier.cy * dm->t_info.siY.nMax;
    direction = FORWARD__;
    break;

  case SB_LINEUP:
    shift = dm->t_info.scroll_multiplier.cy;
    direction = BACKWARD__;
    break;

  case SB_LINEDOWN:
    shift = dm->t_info.scroll_multiplier.cy;
    direction = FORWARD__;
    break;

  case SB_PAGEUP:
    shift = dm->t_info.siY.nPage;
    direction = BACKWARD__;
    break;

  case SB_PAGEDOWN:
    shift = dm->t_info.siY.nPage;
    direction = FORWARD__;
    break;

  case SB_THUMBTRACK:
    if (dm->t_info.siY.nPos > dm->t_info.siY.nTrackPos)
    {
        shift = (dm->t_info.siY.nPos - dm->t_info.siY.nTrackPos) * dm->t_info.scroll_multiplier.cy;
        direction = BACKWARD__;
    }
    else
    {
        shift = (dm->t_info.siY.nTrackPos - dm->t_info.siY.nPos) * dm->t_info.scroll_multiplier.cy;
        direction = FORWARD__;
    }
    break;

  default:
    break;
  }

  MOVETEXT(dm, shift, direction, 0);


}

int OnSize (HWND myWin)
{
    RECT new_sz;
    HDC real_DC;
    display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
    if (entry == NULL)
      return DONE;
    GetClientRect(myWin, &new_sz);
    entry->textwin_limits.cx = new_sz.right-new_sz.left;
    entry->textwin_limits.cy = new_sz.bottom-new_sz.top;
    SwapBitmap(entry);
    DeleteObject(entry->bitmap);

    real_DC = GetDC(myWin);
    entry->bitmap = CreateCompatibleBitmap(real_DC, entry->textwin_limits.cx, entry->textwin_limits.cy);
    ReleaseDC(myWin, real_DC);

    SwapBitmap(entry);

    FixScrollVals(entry);
    entry->t_info.siX.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    SetScrollInfo(myWin, SB_HORZ, &(entry->t_info.siX), TRUE);
    entry->t_info.siY.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    SetScrollInfo(myWin, SB_VERT, &(entry->t_info.siY), TRUE);

    PaintTextDM(entry, NULL, 0, 0);

    return DONE;
}


int OnPaint (HWND myWin)
{
    display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
    if (entry == NULL || entry->t_info.text == NULL)
    {
        Rectangle(entry->context_to_draw, 0, 0, entry->textwin_limits.cx, entry->textwin_limits.cy);
        return DONE;
    }

    HDC deviceDC = GetDC(myWin);
    BitBlt(deviceDC, 0, 0, entry->textwin_limits.cx, entry->textwin_limits.cy, entry->context_to_draw, 0, 0, SRCCOPY);
    ReleaseDC(myWin, deviceDC);
    return DONE;
}



int OnArrow (HWND myWin, int arrow_code)
{
    int res;
    display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
    if (entry == NULL)
      return DONE;
    if (arrow_code & (ARROW_DOWN__ | ARROW_UP__))
    {
        res = MOVETEXT(entry, 1 , ((arrow_code & ARROW_DOWN__) == 0 ? ((arrow_code & ARROW_UP__) == 0 ? 0 : BACKWARD__) : FORWARD__), 0);
    }

    if (arrow_code & (ARROW_LEFT__ | ARROW_RIGHT__))

    {
        res = MOVETEXT(entry, 1, ((arrow_code & ARROW_RIGHT__) == 0 ? ((arrow_code & ARROW_LEFT__) == 0 ? 0 : BACKWARD__) : FORWARD__), 1);
    }
    return res;

}


int OnButton(HWND myWin, int button_code)
{
  int res;
  display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
  if (entry == NULL)
    return DONE;
  if ((button_code & BUTTON_PGUP__) || (button_code & BUTTON_PGDWN__))
  {
    size_t strings_to_move = entry->textwin_limits.cy / entry->textwin_symb_size.cy;
    res = MOVETEXT(entry, strings_to_move, ((button_code & BUTTON_PGDWN__) == 0 ? ((button_code & BUTTON_PGUP__) == 0 ? 0 : BACKWARD__) : FORWARD__), 0);
  }
  return res;
}


int SetDisplayMode(HWND myWin, text_display_mode m)
{
  display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
  if (entry == NULL)
    return DONE;

  entry->mode = m;
  FixScrollVals(entry);

  if (m == TDM_FITTED)
  {
    //entry->t_info.siX.fMask = SIF_DISABLENOSCROLL;
    SetScrollInfo(myWin, SB_HORZ, &(entry->t_info.siX), TRUE);
    //SetScrollInfo(myWin, SB_VERT, &(entry->t_info.siY), TRUE);

    //entry->t_info.current_first_symbol = 0;
  }
  if (m == TDM_ASIS)
  {
    entry->t_info.siX.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    SetScrollInfo(myWin, SB_HORZ, &(entry->t_info.siX), TRUE);
  }

  PaintTextDM(entry, NULL, 0, 0);
  return DONE;
}

int SetText(HWND myWin, text_to_display* t)
{
  display_meta* entry = (display_meta*)GetWindowLongPtrA(myWin, GWLP_USERDATA);
  RECT r = {0, 0, entry->textwin_limits.cx, entry->textwin_limits.cy};
  if (entry == NULL)
  {
        Rectangle(entry->context_to_draw, r.left, r.top, r.right, r.bottom);
        return DONE;
  }
  InitDMText(entry, t);
  PaintTextDM(entry, NULL, 0, 0);
  InvalidateRect(myWin, &r, TRUE);
  return DONE;
}
