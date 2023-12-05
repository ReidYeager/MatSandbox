
#include "src/common.h"
#include "src/camera.h"

MsbResult UpdateCamera()
{
  Quaternion camRotQuat = QuaternionFromEuler(state.camera.transform.rotation);

  *state.globalInputValues.camForward = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, -1.0f });

  state.camera.transform.position = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, state.camera.armLength });
  state.camera.transform.position = Vec3AddVec3(state.camera.transform.position, state.camera.focusPosition);
  *state.globalInputValues.camView = Mat4Invert(TransformToMat4(state.camera.transform));

  state.camera.rotationQuat = QuaternionFromEuler(state.camera.transform.rotation);

  return Msb_Success;
}

void MsbCamera::SetBufferPointers(void* inViewMatrix, void* inProjMatrix, void* inFwdVector)
{
  viewMatrix = (Mat4*)inViewMatrix;
  projectionMatrix = (Mat4*)inProjMatrix;
  forwardVector = (Vec3*)inFwdVector;
}

void MsbCamera::SetProjection(float screenRatio)
{
  *projectionMatrix = ProjectionPerspectiveExtended(
    screenRatio,
    1.0f,    // 1:1 guaranteed ratio
    90.0f,   // VFov
    0.01f,   // near clip
    100.0f); // far clip
}

MsbResult MsbCamera::Update()
{
  Quaternion rotationQuat = QuaternionFromEuler(state.camera.transform.rotation);

  *forwardVector = QuaternionMultiplyVec3(rotationQuat, { 0.0f, 0.0f, -1.0f });

  transform.position = QuaternionMultiplyVec3(rotationQuat, { 0.0f, 0.0f, armLength });
  transform.position = Vec3AddVec3(state.camera.transform.position, state.camera.focusPosition);
  *viewMatrix = Mat4Invert(TransformToMat4(state.camera.transform));

  return Msb_Success;
}
