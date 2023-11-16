
#include "src/common.h"

MsResult UpdateCamera()
{
  Quaternion camRotQuat = QuaternionFromEuler(state.camera.transform.rotation);

  *state.globalInputValues.camForward = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, -1.0f });

  state.camera.transform.position = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, state.camera.armLength });
  state.camera.transform.position = Vec3AddVec3(state.camera.transform.position, state.camera.focusPosition);
  *state.globalInputValues.camView = Mat4Invert(TransformToMat4(state.camera.transform));

  state.camera.rotationQuat = QuaternionFromEuler(state.camera.transform.rotation);

  return Ms_Success;
}
