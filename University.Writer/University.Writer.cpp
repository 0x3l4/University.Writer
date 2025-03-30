#pragma comment(lib, "Comctl32.lib")

#include "framework.h"
#include "University.Writer.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T("Global\\MyUniqueMutexName"));

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, _T("Программа уже запущена!"), _T("Ошибка"), MB_OK | MB_ICONERROR);
        CloseHandle(hMutex);
        return 1;
    }

    // Основной код программы

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_UNIVERSITYWRITER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UNIVERSITYWRITER));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(hMutex);

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UNIVERSITYWRITER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_UNIVERSITYWRITER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    HWND hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_X, WINDOW_Y, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
        switch (wmId)
        {
        case IDC_EDIT_FILE: {
            if (HIWORD(wParam) == EN_CHANGE) {
                KillTimer(hWnd, ID_DEBOUNCE_TIMER);          // сбрасываем предыдущий таймер, если он был установлен
                SetTimer(hWnd, ID_DEBOUNCE_TIMER, 1000, NULL);
            }
            break;
        }
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:
        OnCreate(hWnd);
        break;
    case WM_TIMER:
    {
        if (wParam == ID_DEBOUNCE_TIMER)
        {
            KillTimer(hWnd, ID_DEBOUNCE_TIMER);

            int textLen = GetWindowTextLengthA(hEditFile);
            if (textLen == 0) break; // Если текста нет, выходим. По идее эта проверка не должна активироваться, но на всякий случай
            // Выделяем буфер для текста +1 для конца строки
            char* textBuffer = new char[FILE_SIZE];
            memset(textBuffer, '\0', FILE_SIZE);

            GetWindowTextA(hEditFile, textBuffer, textLen + 1);

            if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
                MessageBox(NULL, _T("Ошибка дескриптора файла"), _T("Ошибка"), MB_OK | MB_ICONERROR);
                break;
            }

            /*UnmapViewOfFile(mapLP);
            CloseHandle(hMapping);*/

            // Тут какая-то магия происходит, эта строчка автоматом делает код рабочим, почему не знаю
            ResetEvent(hFileUpdateEvent);

            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

            PDWORD lpNumberOfBytesWritten = new DWORD;
            if (!WriteFile(hFile, textBuffer, FILE_SIZE, lpNumberOfBytesWritten, NULL)) {
                MessageBox(NULL, _T("Ошибка записи в файл"), _T("Ошибка"), MB_OK | MB_ICONERROR);
                break;
            }

            SetEndOfFile(hFile);

            //MapFileAndCreateView();

            SetEvent(hFileUpdateEvent);

            delete lpNumberOfBytesWritten;
            delete[] textBuffer;
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        pmmi->ptMinTrackSize.x = WINDOW_X;
        pmmi->ptMinTrackSize.y = WINDOW_Y;
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY: {
        if (mapLP) UnmapViewOfFile(mapLP);
        if (hMapping) CloseHandle(hMapping);
        if (hFileUpdateEvent) CloseHandle(hFileUpdateEvent);
        KillTimer(hWnd, ID_DEBOUNCE_TIMER);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void OnCreate(HWND hWnd) {
    InitCommonControls();

    char str[] = "Hello World!";

    hEditFile = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", str,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        BASE_MARGIN, BASE_MARGIN, WINDOW_X - BASE_MARGIN * 2, WINDOW_Y - BASE_MARGIN * 2, hWnd, (HMENU)IDC_EDIT_FILE, hInst, NULL);
    if (hEditFile == NULL) {
        MessageBox(hWnd, L"Не удалось создать элемент управления.", L"Ошибка", MB_ICONERROR);
        return;
    }

    hFile = CreateFileA(FILE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Ошибка создания файла", "Ошибка", MB_OK);
        return;
    }

    hFileUpdateEvent = CreateEventA(NULL, TRUE, FALSE, "FileUpdateEvent");
    if (!hFileUpdateEvent) {
        MessageBoxA(NULL, "Ошибка открытия именого события", "Ошибка", MB_OK);
        return;
    }

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN); //Размеры файла - 2Кб, кста
    PDWORD lpNumberOfBytesWritten = new DWORD;

    if (!WriteFile(hFile, str, strlen(str), lpNumberOfBytesWritten, NULL)) {
        MessageBoxA(NULL, "Ошибка записи в файл", "Ошибка", MB_OK);
        return;
    }
    SetEndOfFile(hFile);
    delete lpNumberOfBytesWritten; //Почистим на всякий случай

    SetEvent(hFileUpdateEvent);

    MapFileAndCreateView();
}

void MapFileAndCreateView() {
    hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, FILE_SIZE, MAPPING_NAME);
    if (hMapping == NULL)
    {
        MessageBoxA(NULL, "Ошибка создания объекта отображения файла", "Ошибка", MB_OK);
        CloseHandle(hFile);
        return;
    }

    mapLP = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, FILE_SIZE);
    if (mapLP == NULL)
    {
        MessageBoxA(NULL, "Ошибка отображения файла", "Ошибка", MB_OK);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }
}