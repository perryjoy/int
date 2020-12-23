#ifndef USING_DBG
#define USING_DBG
#endif // USING_DBG


#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include "text.h"
#include "error.h"
#include "text_display.h"
#include "menu.h"
#include "main.h"
#include "menuID.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = sizeof(display_meta *);                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    HMENU winMenu = CreateMyMenu(hThisInstance);


    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               0,                   /* Extended possibilities for variation */
               szClassName,         /* Class name */
               _T("Thats window with custom text"),       /* Title Text */
               WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL, /* default window */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               544,                 /* The programs width */
               375,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               winMenu,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               lpszArgument                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CREATESTRUCT *s;
    RECT r;
    int arrow_to_report = 0;
    int button_to_report = 0;
    static text_to_display *ttd;
    char* entry = NULL;
    size_t text_size;
    //static INT tmp_size;


    switch (message)                  /* handle the messages */
    {
    case WM_DESTROY:
        DestroyText(&ttd);
        DestroyDisplay(hwnd);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    case WM_CREATE:
        s = (CREATESTRUCT *) lParam;
        InitDisplay(hwnd);
        RegisterTextPtrKostyl(&ttd);
        if (lParam != 0)
          if (ReadToBufferA(((char*)(s->lpCreateParams)), &entry, &text_size) == DONE)
          {
              InitTxt(entry, text_size, &ttd);
              SetText(hwnd, ttd);
              free(entry);
              entry = NULL;
          }
          else
            OnCommand(hwnd, ID_MENU_FILE_CLOSE);

        OnSize(hwnd);
        break;
    case WM_PAINT:

        OnPaint(hwnd);

        break;

    case WM_SIZE:
        OnSize(hwnd);
        GetClientRect(hwnd, &r);
        InvalidateRect(hwnd, &r, 1);
        break;

    case WM_VSCROLL:
        OnScrollY(hwnd, LOWORD(wParam));
        break;

    case WM_HSCROLL:
        OnScrollX(hwnd, LOWORD(wParam));
        break;

    case WM_KEYDOWN:
        if (wParam == VK_DOWN)
          arrow_to_report |= ARROW_DOWN__;
        if (wParam == VK_UP)
          arrow_to_report |= ARROW_UP__;
        if (wParam == VK_LEFT)
          arrow_to_report |= ARROW_LEFT__;
        if (wParam == VK_RIGHT)
          arrow_to_report |= ARROW_RIGHT__;

        if (wParam == VK_PRIOR)
          button_to_report |= BUTTON_PGUP__;
        if (wParam == VK_NEXT)
          button_to_report |= BUTTON_PGDWN__;

        if (arrow_to_report)
        {
            OnArrow(hwnd, arrow_to_report);
            GetClientRect(hwnd, &r);
            InvalidateRect(hwnd, &r, 1);
        }
        if (button_to_report)
        {
          OnButton(hwnd, button_to_report);
          GetClientRect(hwnd, &r);
          InvalidateRect(hwnd, &r, 1);
        }
        break;


    case WM_KEYUP:
        break;

    case WM_COMMAND:
        OnCommand(hwnd, LOWORD(wParam));
        break;

    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

