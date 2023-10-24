
#ifndef MATSANDBOX_WINDOW_H
#define MATSANDBOX_WINDOW_H

#include <peridot.h>
#include <opal.h>

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>

typedef struct MsbWindow
{
  HWND hwnd;
  HINSTANCE hinstance;

  uint32_t windowStyle;
  uint32_t windowExStyle;

  OpalWindow opal;

  bool isOpen;
} MsbWindow;

typedef struct MsbWindowInitInfo
{
  Vec2U extents;
  Vec2I position;
  const char* title;
} MsbWindowInitInfo;

void WindowInit(MsbWindow* window, MsbWindowInitInfo initInfo);
void WindowShutdown(MsbWindow* window);
void WindowUpdate(MsbWindow* window);

#endif // !MATSANDBOX_WINDOW_H
