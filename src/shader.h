
#ifndef MAT_SANDBOX_SHADER_H
#define MAT_SANDBOX_SHADER_H

#include "src/definesNew.h"

enum MsbShaderType
{
  Msb_Shader_Vertex,
  Msb_Shader_Fragment,
  // Msb_Shader_Compute,  // TODO : Extend shader type support
  // Msb_Shader_Geometry, // TODO : Extend shader type support
  // Msb_Shader_Mesh,     // TODO : Extend shader type support
  Msb_Shader_COUNT
};

struct MsbShaderInitInfo
{
  MsbShaderType type;
  uint32_t sourceSize;
  char* pSource;
};

class MsbShader
{
private:
  OpalShader m_opal = OPAL_NULL_HANDLE;
  MsbShaderType m_type;

  uint32_t m_sourceCapacity = 0;
  uint32_t m_sourceSize = 0;
  char* m_sourceBuffer = NULL; // Plain-text glsl source code

  uint32_t m_compiledSourceSize = 0;
  char* m_compiledSource = NULL;

public:
  MsbResult Init(MsbShaderInitInfo initInfo);
  void Shutdown();

  constexpr OpalShader GetOpal() { return m_opal; }
  MsbResult UpdateSource(uint32_t size, char* pPlainTextSource);

private:
  MsbResult Compile(uint32_t size, char* pSource);
  MsbResult Update();


};

#endif // !MAT_SANDBOX_SHADER_H
