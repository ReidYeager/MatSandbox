
#ifndef MAT_SANDBOX_DEFINES_NEW_H
#define MAT_SANDBOX_DEFINES_NEW_H

#include "src/defines.h"

extern OpalInputLayout msbSingleImageLayout;

struct MsbWindow
{
  LapisWindow lapis;
  OpalWindow opal;

  OpalImage renderBufferImage;

  Vec2U extents = { 1280, 720 };
  Vec2I screenPosition = { 1700, 50 };
  const char* title = "Material sandbox";
};

struct MsbSceneRenderResources
{
  OpalImage depthImage;
  OpalImage sceneImage;

  OpalFramebuffer framebuffer;
  OpalRenderpass renderpass;
};

struct MsbUiRenderResources
{
  OpalFramebuffer framebuffer;
  OpalRenderpass renderpass;

  OpalInputSet inputSet;
};

#endif // !MAT_SANDBOX_DEFINES_NEW_H
