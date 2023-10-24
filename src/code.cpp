
#include "src/common.h"

FILE* newShaderHlslSource;

MsResult CompileShader(OpalShaderType type, const char* source);
uint32_t ParseCompilationErrors(FILE* pipe);


MsResult CompileShader(OpalShaderType type, const char* source)
{
  bool isFragment = type == Opal_Shader_Fragment;
  uint32_t size = strlen(source);

  char newFileName[64];
  char command[512];

  sprintf_s(newFileName, 64, "NewShaderSource.%s", isFragment ? "frag" : "vert");
  sprintf_s(command, 512, VULKAN_COMPILER " %s -o NewShaderCompiled.spv 2>&1", newFileName);

  // Save file
  fopen_s(&newShaderHlslSource, newFileName, "w");
  fwrite(source, sizeof(char), size, newShaderHlslSource);
  fclose(newShaderHlslSource);

  // Execute compilation command
  FILE* fp = _popen(command, "r");
  // Handle errors
  if (ParseCompilationErrors(fp) > 0)
  {
    _pclose(fp);
    return Ms_Fail;
  }
  _pclose(fp);

  return Ms_Success;
}

uint32_t ParseCompilationErrors(FILE* pipe)
{
  uint32_t errorCount = 0;

  char shellBuffer[2048];

  // TODO : Parse and handle errors
  while (fgets(shellBuffer, 1024, pipe) != NULL)
  {
    printf("%u : \"%s\"", errorCount, shellBuffer);
    errorCount++;
  }

  return errorCount;
}

MsResult RecreateShader(OpalShaderType type)
{
  bool isFragment = type == Opal_Shader_Fragment;

  OpalShader* shader = &shaders[isFragment];
  OpalShaderShutdown(shader);
  OpalShaderInitInfo initInfo = {};
  initInfo.type = isFragment ? Opal_Shader_Fragment : Opal_Shader_Vertex;
  initInfo.size = LapisFileRead("NewShaderCompiled.spv", &initInfo.pSource);
  if (OpalShaderInit(shader, initInfo) != Opal_Success)
  {
    printf("Failed to reinit the shader\n");
    return Ms_Fail;
  }
  LapisMemFree(initInfo.pSource);

  return Ms_Success;
}

MsResult MsUpdateShader(OpalShaderType type, const char* source)
{
  MS_ATTEMPT(CompileShader(type, source));

  // TODO : Wait for compilation to complete
  // Currently will crash if recreation happens before compilation finishes

  MS_ATTEMPT(RecreateShader(type));

  // Update the material
  OpalMaterialReinit(material);

  return Ms_Success;
}

