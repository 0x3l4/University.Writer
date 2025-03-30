#pragma once

#include "resource.h"

#define MAX_LOADSTRING 100
#define WINDOW_X 600
#define WINDOW_Y 300
#define BASE_MARGIN 10

#define IDC_EDIT_FILE 201
#define ID_DEBOUNCE_TIMER  202
#define FILE_SIZE 2048

// ���������� ����������:
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����
HWND hEditFile = NULL;
HANDLE hFile = INVALID_HANDLE_VALUE, hMapping = INVALID_HANDLE_VALUE;
HANDLE hFileUpdateEvent = NULL;
LPVOID mapLP = NULL;
const char* FILE_NAME = "WR-File.txt";
const char* MAPPING_NAME = "Local\\WRFileMapping";

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void				OnCreate(HWND);
void				MapFileAndCreateView();