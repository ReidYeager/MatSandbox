
#include "src/common.h"

#include <lapis.h>

const char* GetShaderTypeExtension(OpalShaderType type)
{
  const char* shaderTypeExtension;
  switch (type)
  {
  case Opal_Shader_Vertex: shaderTypeExtension = "vert"; break;
  case Opal_Shader_Fragment: shaderTypeExtension = "frag"; break;
  default: shaderTypeExtension = "unknown"; break;
  }

  return shaderTypeExtension;
}

MsResult MsUpdateShader(OpalShaderType type)
{
  bool isFragment = type == Opal_Shader_Fragment;

  char shaderNameBuffer[SHADER_NAME_MAX_LENGTH];
  sprintf_s(shaderNameBuffer, SHADER_NAME_MAX_LENGTH, "NewShaderSource.%s", GetShaderTypeExtension(type));

  OpalShader* shader = &state.pShaders[isFragment];
  OpalShaderInitInfo initInfo = {};
  initInfo.type = isFragment ? Opal_Shader_Fragment : Opal_Shader_Vertex;
  initInfo.size = LapisFileRead(shaderNameBuffer, &initInfo.pSource);
  if (OpalShaderInit(shader, initInfo) != Opal_Success)
  {
    printf("Failed to reinit the shader\n");
    return Ms_Fail;
  }
  LapisMemFree(initInfo.pSource);

  return Ms_Success;
}

MsResult MsUpdateMaterial()
{
  MS_ATTEMPT_OPAL(OpalMaterialReinit(state.material));

  return Ms_Success;
}
