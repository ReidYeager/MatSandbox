
#ifndef MATSANDBOX_COMMON_H
#define MATSANDBOX_COMMON_H

#include "src/defines.h"

#include <lapis.h>
#include <opal.h>
#include <peridot.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>

#include <stdio.h>
#include <string>
#include <memory>

extern OpalShader shaders[2];
extern OpalMaterial material;

MsResult MsInit();
MsResult MsUpdate();
void MsShutdown();

MsResult UpdateCamera();

MsResult RenderUi();

const char* GetShaderTypeExtension(OpalShaderType type);
MsResult MsCompileShader(OpalShaderType type, const char* source);
MsResult MsUpdateShader(OpalShaderType type);
void MsUpdateMaterialValues();
MsResult MsUpdateMaterialInputLayoutAndSet();
MsResult MsUpdateMaterial();
uint32_t MsBufferElementSize(MsBufferElementType element);
MsResult Render();
MsResult MsCreateInputArgument(MsInputArgumentInitInfo info, uint32_t* outArgumentIndex);
MsResult MsUpdateInputArgument(MsInputArgument* argument);
uint32_t MsPadForUniform(uint32_t size);

MsResult LoadMesh(const char* path, OpalMesh* outMesh);

#endif // !MATSANDBOX_COMMON_H
