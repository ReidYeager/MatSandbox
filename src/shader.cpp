
#include "src/definesNew.h"
#include "src/shader.h"

uint32_t MsbParseCompilationErrors(FILE* pipe);

MsbResult MsbShader::Init(MsbShaderInitInfo initInfo)
{
  type = initInfo.type;

  MSB_ATTEMPT(UpdateSource(initInfo.sourceSize, initInfo.pSource));
  
  MSB_LOG("Shader init success\n");
  return Msb_Success;
}

void MsbShader::Shutdown()
{
  LapisMemFree(pCompiledSource);
  OpalShaderShutdown(&opal);
}

MsbResult MsbShader::UpdateSource(uint32_t size, char* pPlainTextSource)
{
  MSB_ATTEMPT(Compile(size, pPlainTextSource));
  MSB_ATTEMPT(Update());

  return Msb_Success;
}

MsbResult MsbShader::Compile(uint32_t size, char* pSource)
{
  static const uint32_t nameMaxLength = 64;
  static const uint32_t commandMaxLength = 512;
  static const uint32_t extensionMaxLength = 5;

  char tmpFileName[nameMaxLength] = { 0 };
  char tmpCompiledFileName[nameMaxLength] = { 0 };
  char command[commandMaxLength] = { 0 };
  char extension[extensionMaxLength] = { 0 };

  // Build compilation command
  // ===============
  switch (type)
  {
  case Msb_Shader_Vertex: LapisMemCopy((char*)"vert", extension, extensionMaxLength); break;
  case Msb_Shader_Fragment: LapisMemCopy((char*)"frag", extension, extensionMaxLength); break;
  default: MSB_ERR("Attempting to compile unsupported shader type : %u\n", type); return Msb_Fail;
  }

  sprintf_s(tmpFileName, nameMaxLength, "NewShaderSource.%s", extension);
  sprintf_s(tmpCompiledFileName, nameMaxLength, "NewShaderCompiled.%s.spv", extension);
  sprintf_s(command, commandMaxLength, VULKAN_COMPILER " %s -o %s 2>&1", tmpFileName, tmpCompiledFileName, extension);

  // Compile
  // ===============
 
  // Save source file (compilation requires a file input)
  FILE* newShaderSource;
  fopen_s(&newShaderSource, tmpFileName, "w");
  fwrite(pSource, sizeof(char), size, newShaderSource);
  fclose(newShaderSource);

  // Execute compilation command
  FILE* fp = _popen(command, "r");

  // TODO : Remove temporary source file

  // Handle compilatin errors
  // ===============
  if (MsbParseCompilationErrors(fp) > 0)
  {
    _pclose(fp);
    return Msb_Fail;
  }
  _pclose(fp);

  // Update compiled source
  // ===============
  FILE* compiledFile;
  fopen_s(&compiledFile, tmpCompiledFileName, "rb");
  // Get size
  fseek(compiledFile, 0L, SEEK_END);
  compiledSourceSize = (uint32_t)ftell(compiledFile);
  rewind(compiledFile);
  // Read source
  if (pCompiledSource != NULL)
    LapisMemFree(pCompiledSource);
  pCompiledSource = LapisMemAllocZeroArray(char, compiledSourceSize);
  fread(pCompiledSource, 1, (size_t)compiledSourceSize, compiledFile);
  // Close
  fclose(compiledFile);

  return Msb_Success;
}

uint32_t MsbParseCompilationErrors(FILE* pipe)
{
  uint32_t errorCount = 0;

  char shellBuffer[2048];

  // TODO : Parse and present errors within the app itself
  while (fgets(shellBuffer, 1024, pipe) != NULL)
  {
    printf("%u : %s", errorCount, shellBuffer);
    errorCount++;
  }

  return errorCount;
}

MsbResult MsbShader::Update()
{
  if (opal != OPAL_NULL_HANDLE)
    OpalShaderShutdown(&opal);

  OpalShaderInitInfo initInfo = { 0 };
  initInfo.pSource = pCompiledSource;
  initInfo.size = compiledSourceSize;

  switch (type)
  {
  case Msb_Shader_Vertex: initInfo.type = Opal_Shader_Vertex; break;
  case Msb_Shader_Fragment: initInfo.type = Opal_Shader_Fragment; break;
  default: MSB_ERR("Attempting to create shader with unsupported type : %u\n", type); return Msb_Fail;
  }

  MSB_ATTEMPT_OPAL(OpalShaderInit(&opal, initInfo));

  return Msb_Success;
}

