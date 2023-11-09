
#ifndef MATSANDBOX_COMMON_H
#define MATSANDBOX_COMMON_H

#include "src/defines.h"

#include <opal.h>

extern OpalShader shaders[2];
extern OpalMaterial material;

MsResult MsInit();
MsResult MsUpdate();
void MsShutdown();

MsResult MsUpdateShader(OpalShaderType type, const char* source);
MsResult MsCompileShader(OpalShaderType type, const char* source);
MsResult MsRecreateShader(OpalShaderType type);
MsResult LoadMesh(const char* path, OpalMesh* outMesh);

#endif // !MATSANDBOX_COMMON_H
