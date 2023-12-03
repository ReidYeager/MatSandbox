
#ifndef MAT_SANDBOX_DEFINES_NEW_H
#define MAT_SANDBOX_DEFINES_NEW_H

#include "src/defines.h"

struct MsbWindow
{
  LapisWindow lapis;
  OpalWindow opal;

  OpalImage renderBufferImage;

  Vec2U extents = { 1280, 720 };
  Vec2I screenPosition = { 50, 50 };
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

  OpalInputLayout singleImageInputLayout;
  OpalInputSet inputSet;
};

#endif // !MAT_SANDBOX_DEFINES_NEW_H
