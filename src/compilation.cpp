
#include "src/common.h"

FILE* newShaderHlslSource;

uint32_t ParseCompilationErrors(FILE* pipe)
{
  uint32_t errorCount = 0;

  char shellBuffer[2048];

  // TODO : Parse and present errors
  while (fgets(shellBuffer, 1024, pipe) != NULL)
  {
    printf("%u : \"%s\"", errorCount, shellBuffer);
    errorCount++;
  }

  return errorCount;
}

MsResult MsCompileShader(OpalShaderType type, const char* source)
{
  bool isFragment = type == Opal_Shader_Fragment;
  uint32_t size = strlen(source);

  char newFileName[SHADER_NAME_MAX_LENGTH];
  char command[512];

  sprintf_s(newFileName, SHADER_NAME_MAX_LENGTH, "NewShaderSource.%s", isFragment ? "frag" : "vert");
  sprintf_s(command, 512, VULKAN_COMPILER " %s -o NewShaderCompiled.%s.spv 2>&1", newFileName, GetShaderTypeExtension(type));

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