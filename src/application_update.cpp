
#include "src/application.h"

#include <chrono>

MsbResult MsbApplication::Update()
{
  while (!LapisWindowGetShouldClose(m_window.lapis))
  {
    UpdateTime();
    LapisWindowUpdate(m_window.lapis);

    if (LapisWindowGetResized(m_window.lapis))
    {
      MSB_ATTEMPT(ResizeUiRenderResources());
    }

    // Input
    MSB_ATTEMPT(HandleHidInput());

    // Re-load images
    // Re-compile shaders
    // Re-create material

    // Update buffer data
    MSB_ATTEMPT(m_globalSet.PushBuffersData());
    MSB_ATTEMPT(m_customSet.PushBuffersData());

    // Render
    if (LapisWindowGetMinimized(m_window.lapis) || !LapisWindowGetVisible(m_window.lapis))
      continue;

    MSB_ATTEMPT_OPAL(OpalRenderBegin(m_window.opal));

    //MSB_ATTEMPT(RenderScene());
    MSB_ATTEMPT(m_ui.Render());

    MSB_ATTEMPT_OPAL(OpalRenderEnd());

  }

  return Msb_Success;
}

void MsbApplication::UpdateTime()
{
  static const float microToSecond = 0.000001f;
  static auto frameStart = std::chrono::steady_clock::now();
  static auto frameEnd = std::chrono::steady_clock::now();

  frameEnd = std::chrono::steady_clock::now();
  m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
  frameStart = frameEnd;

  *(float*)m_globalBuffer->pElements[0].data += m_deltaTime;
}

MsbResult MsbApplication::ResizeUiRenderResources()
{
  uint32_t width, height;
  LapisWindowGetExtents(m_window.lapis, &width, &height);

  if (LapisWindowGetMinimized(m_window.lapis) || width == 0 || height == 0)
    return Msb_Success;

  MSB_ATTEMPT_OPAL(OpalWindowReinit(m_window.opal));
  MSB_ATTEMPT_OPAL(OpalFramebufferReinit(m_uiRenderResources.framebuffer));

  return Msb_Success;
}

MsbResult MsbApplication::HandleHidInput()
{
  return Msb_Success;
}
