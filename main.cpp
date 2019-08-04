#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#ifndef UNICODE
#define tstring std::string
#else
#define tstring std::wstring
#endif

#include <tchar.h>
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

//#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

tstring ConvertFile (TCHAR *name);

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;   //Creating window class

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    if (!RegisterClassEx (&wincl))          //Registering window class
        return 0;

    HMENU hMenu = CreateMenu();         //Creating simple menu
    HMENU hFile = CreateMenu();

    AppendMenu(hFile, MF_STRING, IDM_FILE_OPEN, "&Open");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_QUIT, "&Exit");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "&File");

    hwnd = CreateWindowEx (
           0,
           szClassName,
           _T("File converter 2.1"),
           WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           544,                                         //window parameters
           375,
           HWND_DESKTOP,
           hMenu,
           hThisInstance,
           NULL
           );

    ShowWindow (hwnd, nCmdShow);

    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

std::vector<std::string> v;  //holds data


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, y, k;

    static OPENFILENAME file;
    static HINSTANCE hInst;
    static TCHAR name[256] = _T("");

    static int n, length, sx, sy, cx, iVscrollPos, iHscrollPos, COUNT, MAX_WIDTH;
    static SIZE siz = {8, 16};

    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;

    std::vector<std::string>::iterator it;

    switch (message)
        {
        case WM_CREATE:
            file.lStructSize = sizeof(OPENFILENAME);
            file.hInstance = hInst;
            file.lpstrFilter = _T("Text\0*.txt");
            file.lpstrFile = name;
            file.nMaxFile = 256;
            file.lpstrInitialDir = _T(".\\");
            file.lpstrDefExt = _T("txt");
            break;
        case WM_SIZE:
            sx = LOWORD(lParam);
            sy = HIWORD(lParam);

            k = n - sy / 20;
            if (k > 0) COUNT = k;
            else COUNT = iVscrollPos = 0;
            SetScrollRange(hwnd, SB_VERT, 0, COUNT, FALSE);
            SetScrollPos(hwnd, SB_VERT, iVscrollPos, TRUE);

            k = length - sx / 16;
            if (k > 0) MAX_WIDTH = k;
            else MAX_WIDTH = iHscrollPos = 0;
            SetScrollRange(hwnd, SB_HORZ, 0, MAX_WIDTH, FALSE);
            SetScrollPos(hwnd, SB_HORZ, iHscrollPos, TRUE);

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_MOUSEWHEEL:
            iVscrollPos -=(short)HIWORD(wParam)/WHEEL_DELTA;
            iVscrollPos = std::max(0, std::min(iVscrollPos, COUNT));
            SetScrollPos(hwnd, SB_VERT, iVscrollPos, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_VSCROLL:
            switch (LOWORD(wParam)){
                case SB_LINEUP:   iVscrollPos--; break;
                case SB_LINEDOWN: iVscrollPos++; break;
                case SB_PAGEUP:   iVscrollPos -= sy / 20; break;
                case SB_PAGEDOWN: iVscrollPos += sy / 20; break;
                case SB_THUMBPOSITION: iVscrollPos = HIWORD(wParam); break;
            }
            iVscrollPos = std::max(0, std::min(iVscrollPos, COUNT));
            if (iVscrollPos != GetScrollPos(hwnd, SB_VERT))
            {
                SetScrollPos(hwnd, SB_VERT, iVscrollPos, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_HSCROLL:
            switch (LOWORD(wParam)){
                case SB_LINEUP:   iHscrollPos--; break;
                case SB_LINEDOWN: iHscrollPos++; break;
                case SB_PAGEUP:   iHscrollPos -= sx / 16; break;
                case SB_PAGEDOWN: iHscrollPos += sx / 16; break;
                case SB_THUMBPOSITION: iHscrollPos = HIWORD(wParam); break;
            }
            iHscrollPos = std::max(0, std::min(iHscrollPos, MAX_WIDTH));
            if (iHscrollPos != GetScrollPos(hwnd, SB_HORZ))
            {
                SetScrollPos(hwnd, SB_HORZ, iHscrollPos, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            switch (wmId){
                case IDM_FILE_OPEN:     //opening of the source file
                    file.lpstrTitle = _T("Open source file");
                    file.Flags = OFN_HIDEREADONLY;
                    if (!GetOpenFileName(&file)) {MessageBox(hwnd, _T("Failed to open file"), _T("Error!"), MB_OK | MB_ICONERROR); return 1;}

                    if (!v.empty()) std::vector<std::string>().swap(v);

                    if (ConvertFile(name) == "error") MessageBox(hwnd, _T("Impossible to convert selected file"), _T("Error!"), MB_OK | MB_ICONERROR);      //function for conversion called here
                    else MessageBox(hwnd, name, _T("File converted successfully"), MB_OK);
                    n = v.size();
                    length = v[0].length();
                    iVscrollPos = 0;
                    iHscrollPos = 0;
                    SendMessage(hwnd, WM_SIZE, 0, sy << 16 | sx);
                    InvalidateRect(hwnd, NULL, 1);
                    break;
                case IDM_FILE_QUIT:
                    DestroyWindow(hwnd);
                    break;
                default:
                    return DefWindowProc (hwnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            //SelectObject(hdc, &tm);
            //GetTextMetrics(hdc, &tm);
            SetBkMode(hdc, TRANSPARENT);
                //writing output to the window here
            for (y = 0, it = v.begin() + iVscrollPos; it != v.end() && y < sy; ++it, y += 20)//tm.tmHeight)
                if (iHscrollPos < it->length())
                    TabbedTextOut(hdc, 0, y, it->data()+iHscrollPos, it->length()-iHscrollPos, 0, NULL, 0);
                //TextOut(hdc, 0, y, it->data(), it->length());
            EndPaint(hwnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


tstring ConvertFile(TCHAR *name)
{
    tstring name2 = name;
    int k;
    std::string str, holder, tempstr = "";
    int val1 = 0, val2;

    std::ifstream in;
    in.open(name);
    if (!in) return "error";
    else {}

    name2.insert(name2.length() - 4, "_rec");

    std::ofstream out;
    out.open(name2);
    if (!out) return "error";

    int t_num0 = 1; //counters for the first three records in a row
    int t_num1 = 2;
    int t_num2 = 3;

    in.ignore();
    for (unsigned long i = 1; !in.eof(); i++)  //searching elements through the source file,  writing to the new file
    {
        in >> str;

        if (i == t_num0) {              //first position (time)
            holder = str;
            t_num0 += 11;
        }

        if (i == t_num1) {
            if (str != "***") {              //second position
                out << holder << "\t" << str << "\t";
                tempstr += holder + "\t" + str + "\t";
                val1 = 0;
                for (int j = 0; j < str.length(); j++) val1 = val1*10 + str[j] - 0x30;
            }
            t_num1 += 11;
        }
        if (i == t_num2) {
            if (str != "***" && val1) {      //third position
                val2 = 0;
                for (int j = 0; j < str.length(); j++) val2 = val2*10 + str[j] - 0x30;
                val2 -= val1;                   //temperature difference
                out << str << "\t" << val2 << "\n";
                tempstr += str + "\t" + std::to_string(val2) + "\n";
                v.push_back(tempstr);
                tempstr = "";
            }
            t_num2 += 11;

        }
    }


    in.close();
    out.close();

    return "OK";
}
