
#ifndef MAT_SANDBOX_CAMERA_H
#define MAT_SANDBOX_CAMERA_H

#include "src/definesNew.h"

class MsbCamera
{
private:
  Mat4* viewMatrix;
  Mat4* projectionMatrix;
  Vec3* forwardVector;

  float armLength = 2.0f;
  Transform transform;

public:
  void SetBufferPointers(void* viewMatrix, void* projMatrix, void* fwdVector);
  void SetProjection(float screenRatio);
  MsbResult Update();

};

#endif // MAT_SANDBOX_CAMERA_H
