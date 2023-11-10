
#include "src/common.h"

void UpdateCameraValues()
{
  Quaternion camRotQuat = QuaternionFromEuler(state.camera.transform.rotation);

  state.globalInputValues.camForward = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, -1.0f });

  state.camera.transform.position = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, state.camera.armLength });
  state.camera.transform.position = Vec3AddVec3(state.camera.transform.position, state.camera.focusPosition);
  state.globalInputValues.camView = Mat4Invert(TransformToMat4(state.camera.transform));

  state.globalInputValues.camViewProj = Mat4MuliplyMat4(state.globalInputValues.camProj, state.globalInputValues.camView);

  state.camera.rotationQuat = QuaternionFromEuler(state.camera.transform.rotation);
}

MsResult UpdateCamera()
{
  UpdateCameraValues();

  MS_ATTEMPT_OPAL(OpalBufferPushData(state.globalInputBuffer, &state.globalInputValues));

  return Ms_Success;
}
