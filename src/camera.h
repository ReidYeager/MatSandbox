
#ifndef MAT_SANDBOX_CAMERA_H
#define MAT_SANDBOX_CAMERA_H

#include "src/definesNew.h"

class MsbCamera
{
private:
  Mat4* m_viewMatrix;
  Mat4* m_projectionMatrix;
  Vec3* m_forwardVector;

  float m_armLength = 2.0f;
  Transform m_transform;

public:
  void SetBufferPointers(void* viewMatrix, void* projMatrix, void* fwdVector);
  void SetProjection(float screenRatio);
  MsbResult Update();

};

#endif // MAT_SANDBOX_CAMERA_H
