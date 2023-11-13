
#include "src/common.h"

void HandleInput()
{
  state.camera.armLength -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Wheel) * 0.1f;

  if (LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Mouse_Right)
    || LapisInputGetValue(state.window.lapis, Lapis_Input_Button_R))
  {
    state.camera.transform.rotation.y -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_X);
    state.camera.transform.rotation.x -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_Y);
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
  }
  else if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_F))
  {
    state.camera.focusPosition = { 0.0f, 0.0f, 0.0f };
  }

  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_1)) state.meshIndex = 0;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_2)) state.meshIndex = 1;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_3)) state.meshIndex = 2;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_4)) state.meshIndex = 3;

  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_Escape)) LapisWindowMarkForClosure(state.window.lapis);
}

void UpdateMaterialValues()
{
  for (uint32_t argi = 0; argi < state.materialInfo.inputArgumentCount; argi++)
  {
    if (state.materialInfo.pInputArguements[argi].type != Ms_Input_Buffer) continue;

    MsInputArgumentBuffer* buffer = &state.materialInfo.pInputArguements[argi].data.buffer;
    uint32_t currentOffset = 0;
    for (uint32_t i = 0, elementSize = 0; i < buffer->elementCount; i++)
    {
      MsBufferElement* element = &buffer->pElements[i];
      elementSize = MsBufferElementSize(element->type);
      OpalBufferPushDataSegment(buffer->buffer, element->data, elementSize, currentOffset);
      currentOffset += elementSize;
    }
  }
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

  state.globalInputValues.camProj =
    ProjectionPerspective((float)newExtents.width / (float)newExtents.height, 90.0f, 0.01f, 100.0f);

  OpalInputInfo sceneImageInputInfo = { 0 };
  sceneImageInputInfo.index = 0;
  sceneImageInputInfo.type = Opal_Input_Type_Samped_Image;
  sceneImageInputInfo.value.image = state.sceneImage;
  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.uiSceneImageInputSet, 1, &sceneImageInputInfo));

  return Ms_Success;
}

MsResult Render()
{
  if (LapisWindowGetMinimized(state.window.lapis) || !LapisWindowGetVisible(state.window.lapis))
    return Ms_Success;

  MS_ATTEMPT_OPAL(OpalRenderBegin(state.window.opal));

  // Scene
  OpalRenderBeginRenderpass(state.sceneRenderpass, state.sceneFramebuffer);
  OpalRenderBindMaterial(state.material);
  UpdateMaterialValues();
  OpalRenderBindInputSet(state.globalInputSet, 0);
  OpalRenderBindInputSet(state.materialInfo.inputSet, 1);
  OpalRenderMesh(state.meshes[state.meshIndex]);
  OpalRenderEndRenderpass(state.sceneRenderpass);

  // Ui
  OpalRenderBeginRenderpass(state.uiRenderpass, state.uiFramebuffer);
  MS_ATTEMPT(RenderUi());
  OpalRenderEndRenderpass(state.uiRenderpass);
  MS_ATTEMPT_OPAL(OpalRenderEnd());

  return Ms_Success;
}

MsResult MsUpdate()
{
  while (!LapisWindowGetShouldClose(state.window.lapis))
  {
    MS_ATTEMPT_LAPIS(LapisWindowUpdate(state.window.lapis));

    HandleInput();

    MS_ATTEMPT(UpdateSceneRenderComponents());
    MS_ATTEMPT(UpdateCamera());
    MS_ATTEMPT(Render());
  }

  return Ms_Success;
}
