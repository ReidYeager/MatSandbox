
#include "src/application.h"

MsbResult MsbApplication::Update()
{
  while (!LapisWindowGetShouldClose(window.lapis))
  {
    LapisWindowUpdate(window.lapis);
    MSB_ATTEMPT(HandleHidInput());

    // Render
    MSB_ATTEMPT(RenderScene());
    MSB_ATTEMPT(RenderUi());
  }

  return Msb_Success;
}

MsbResult MsbApplication::Resize(Vec2U newExtents)
{
  return Msb_Success;
}

MsbResult MsbApplication::RenderScene()
{
  return Msb_Success;
}

MsbResult MsbApplication::RenderUi()
{
  return Msb_Success;
}

MsbResult MsbApplication::HandleHidInput()
{
  return Msb_Success;
}
