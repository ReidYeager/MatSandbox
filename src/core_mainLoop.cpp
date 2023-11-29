
#include "src/common.h"

#include <chrono>

void HandleInput()
{
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_S) && LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Lctrl))
  {
    for (uint32_t i = 0; i < state.shaderCount; i++)
    {
      MsShaderAddToCompileQueue(&state.pShaderCodeInfos[i]);
    }
  }

  if (state.inputState.previewHovered)
  {
    float scrollAmount = LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Wheel);
    if (scrollAmount != 0.0f)
    {
      state.camera.armLength -= scrollAmount * 0.1f;
      UpdateCamera();
    }
  }

  if (!state.inputState.previewFocused)
    return;

  if (LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Mouse_Right)
    || LapisInputGetValue(state.window.lapis, Lapis_Input_Button_R))
  {
    state.camera.transform.rotation.y -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_X);
    state.camera.transform.rotation.x -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_Y);
    UpdateCamera();
  }
  else if (LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Mouse_Middle)
    || LapisInputGetValue(state.window.lapis, Lapis_Input_Button_G))
  {
    Vec3 camRight = QuaternionMultiplyVec3(state.camera.rotationQuat, { 1.0f, 0.0f, 0.0f });
    Vec3 camUp = QuaternionMultiplyVec3(state.camera.rotationQuat, { 0.0f, 1.0f, 0.0f });

    camRight = Vec3MultiplyFloat(camRight, -LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_X) * 0.01f * state.camera.armLength);
    camUp = Vec3MultiplyFloat(camUp, LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_Y) * 0.01f * state.camera.armLength);

    state.camera.focusPosition = Vec3AddVec3(state.camera.focusPosition, camRight);
    state.camera.focusPosition = Vec3AddVec3(state.camera.focusPosition, camUp);
    UpdateCamera();
  }
  else if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_F))
  {
    state.camera.focusPosition = { 0.0f, 0.0f, 0.0f };
    UpdateCamera();
  }

  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_1)) state.meshIndex = 0;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_2)) state.meshIndex = 1;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_3)) state.meshIndex = 2;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_4)) state.meshIndex = 3;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_5)) state.meshIndex = 4;
}

MsResult Render()
{
  if (LapisWindowGetMinimized(state.window.lapis) || !LapisWindowGetVisible(state.window.lapis))
    return Ms_Success;

  MS_ATTEMPT_OPAL(OpalRenderBegin(state.window.opal));

  // Scene
  OpalRenderBeginRenderpass(state.sceneRenderpass, state.sceneFramebuffer);
  OpalRenderBindMaterial(state.material);
  OpalRenderBindInputSet(state.globalInputSet.set, 0);
  OpalRenderBindInputSet(state.materialInputSet.set, 1);
  OpalRenderMesh(state.meshes[state.meshIndex]);
  OpalRenderEndRenderpass(state.sceneRenderpass);

  // Ui
  OpalRenderBeginRenderpass(state.uiRenderpass, state.uiFramebuffer);
  MS_ATTEMPT(RenderUi());
  OpalRenderEndRenderpass(state.uiRenderpass);
  MS_ATTEMPT_OPAL(OpalRenderEnd());

  return Ms_Success;
}

MsResult UpdateSceneRenderComponents()
{
  if (state.sceneGuiExtentsPrevFrame.x <= 0 || state.sceneGuiExtentsPrevFrame.y <= 0)
  {
    return Ms_Success;
  }

  OpalExtent newExtents = { (uint32_t)state.sceneGuiExtentsPrevFrame.x, (uint32_t)state.sceneGuiExtentsPrevFrame.y, 1 };

  if (state.sceneImage->extents.width == newExtents.width && state.sceneImage->extents.height == newExtents.height)
  {
    return Ms_Success;
  }


  MS_ATTEMPT_OPAL(OpalImageResize(state.sceneImage, newExtents));
  MS_ATTEMPT_OPAL(OpalImageResize(state.depthImage, newExtents));
  MS_ATTEMPT_OPAL(OpalFramebufferReinit(state.sceneFramebuffer));

  state.globalInputValues.viewportExtents->width = newExtents.width;
  state.globalInputValues.viewportExtents->height = newExtents.height;

  *state.globalInputValues.camProj = ProjectionPerspectiveExtended(
    (float)newExtents.width / (float)newExtents.height,
    1.0f,    // 1:1 guaranteed ratio
    90.0f,   // VFov
    0.01f,   // near clip
    100.0f); // far clip
  UpdateCamera();

  OpalInputInfo sceneImageInputInfo = { 0 };
  sceneImageInputInfo.index = 0;
  sceneImageInputInfo.type = Opal_Input_Type_Samped_Image;
  sceneImageInputInfo.value.image = state.sceneImage;
  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.uiSceneImageInputSet, 1, &sceneImageInputInfo));

  return Ms_Success;
}

MsResult MsUpdate()
{
  const float microToSecond = 0.000001f;
  float deltaTime = 0.0f;
  auto frameStart = std::chrono::steady_clock::now();
  auto frameEnd = std::chrono::steady_clock::now();

  while (!LapisWindowGetShouldClose(state.window.lapis))
  {
    frameEnd = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() * microToSecond;
    *state.globalInputValues.time += deltaTime;

    MS_ATTEMPT_LAPIS(LapisWindowUpdate(state.window.lapis));

    if (state.serialLoadPath[0] != 0)
      MS_ATTEMPT(MsSerializeLoad(state.serialLoadPath));
    MS_ATTEMPT(MsCompileQueuedShaders());
    MS_ATTEMPT(MsReimportQueuedImages());

    HandleInput();

    MS_ATTEMPT(UpdateSceneRenderComponents());
    MS_ATTEMPT(MsInputSetPushBuffers(&state.globalInputSet));
    MS_ATTEMPT(MsInputSetPushBuffers(&state.materialInputSet));
    MS_ATTEMPT(Render());

    if (state.serialLoadPath[0] != 0)
    {
      MS_ATTEMPT(MsSerializeLoad(state.serialLoadPath));
      state.serialLoadPath[0] = 0;
    }

    frameStart = frameEnd;
  }

  return Ms_Success;
}
