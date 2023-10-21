
#include "src/window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

#include <peridot.h>

#include <stdio.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Win32MessageProcess(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
  {
    return true;
  }

  switch (message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  case WM_DESTROY:
  {
    PostQuitMessage(0);
    return 0;
  }
  }

  return DefWindowProcA(hwnd, message, wparam, lparam);
}

void WindowInit(MsbWindow* window, MsbWindowInitInfo initInfo)
{
  bool isResizable = false;
  const char* windowClassName = "MaterialSandboxWindowClass";

  window->hinstance = GetModuleHandleA(0);

  WNDCLASSA wc = {
    /* style;         */ CS_DBLCLKS,
    /* lpfnWndProc;   */ Win32MessageProcess,
    /* cbClsExtra;    */ 0,
    /* cbWndExtra;    */ 0,
    /* hInstance;     */ window->hinstance,
    /* hIcon;         */ LoadIcon(window->hinstance, IDI_APPLICATION),
    /* hCursor;       */ LoadCursor(NULL, IDC_ARROW),
    /* hbrBackground; */ NULL,
    /* lpszMenuName;  */ NULL,
    /* lpszClassName; */ windowClassName
  };

  int x = RegisterClassA(&wc);

  if (x == 0)
  {
    return;
  }

  uint32_t resizability = (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX) * isResizable;

  uint32_t windowStyle = resizability | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  uint32_t windowExStyle = WS_EX_APPWINDOW;

  // Adjust extents so the canvas matches the input extents
  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);
  uint32_t adjustedWidth = initInfo.extents.width + borderRect.right - borderRect.left;
  uint32_t adjustedHeight = initInfo.extents.height + borderRect.bottom - borderRect.top;

  window->hwnd = CreateWindowExA(
    /* dwExStyle    */ windowExStyle,
    /* lpClassName  */ windowClassName,
    /* lpWindowName */ initInfo.title,
    /* dwStyle      */ windowStyle,
    /* X            */ initInfo.position.x, // X screen position
    /* Y            */ initInfo.position.y, // Y screen position
    /* nWidth       */ adjustedWidth,
    /* nHeight      */ adjustedHeight,
    /* hWndParent   */ 0,
    /* hMenu        */ 0,
    /* hInstance    */ window->hinstance,
    /* lpParam      */ 0
  );

  if (window->hwnd == 0)
  {
    printf("Failed to create window\n");
    return;
  }

  ShowWindow(window->hwnd, SW_SHOW);
  window->windowStyle = windowStyle;
  window->windowExStyle = windowExStyle;
  window->isOpen = true;

  return;
}

void WindowUpdate(MsbWindow* window)
{
  MSG message;

  while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
    if (message.message == WM_QUIT || message.message == WM_CLOSE || message.message == WM_DESTROY)
    {
      window->isOpen = false;
    }
  }
}

void WindowShutdown(MsbWindow* window)
{
  DestroyWindow(window->hwnd);
}
