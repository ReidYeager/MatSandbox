
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

MsResult MsUpdate()
{
  while (!LapisWindowGetShouldClose(state.window.lapis))
  {
    MS_ATTEMPT_LAPIS(LapisWindowUpdate(state.window.lapis));

    HandleInput();

    MS_ATTEMPT(UpdateCamera());

    MS_ATTEMPT_OPAL(OpalRenderBegin(state.window.opal));
    OpalRenderBeginRenderpass(state.renderpass, state.framebuffer);
    OpalRenderBindMaterial(state.material);
    OpalRenderBindInputSet(state.globalInputSet, 0);
    OpalRenderMesh(state.meshes[state.meshIndex]);

    MS_ATTEMPT(RenderUi());

    OpalRenderEndRenderpass(state.renderpass);
    MS_ATTEMPT_OPAL(OpalRenderEnd());
  }

  return Ms_Success;
}
