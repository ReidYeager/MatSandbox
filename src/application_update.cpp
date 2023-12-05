
#include "src/application.h"

#include <chrono>

MsbResult MsbApplication::Update()
{
  while (!LapisWindowGetShouldClose(window.lapis))
  {
    UpdateTime();
    LapisWindowUpdate(window.lapis);

    // Input
    MSB_ATTEMPT(HandleHidInput());

    // Re-load images
    // Re-compile shaders
    // Re-create material

    // Update buffer data
    MSB_ATTEMPT(globalSet.PushBuffersData());
    MSB_ATTEMPT(customSet.PushBuffersData());

    // Render
    MSB_ATTEMPT_OPAL(OpalRenderBegin(window.opal));

    //MSB_ATTEMPT(RenderScene());
    MSB_ATTEMPT(ui.Render());

    MSB_ATTEMPT_OPAL(OpalRenderEnd());

  }

  return Msb_Success;
}

void MsbApplication::UpdateTime()
{
  static const float microToSecond = 0.000001f;
  static auto appStart = std::chrono::steady_clock::now();
  static auto frameStart = std::chrono::steady_clock::now();
  static auto frameEnd = std::chrono::steady_clock::now();

  frameEnd = std::chrono::steady_clock::now();
  time.delta = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  time.realSinceStart = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - appStart).count() * microToSecond;
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
