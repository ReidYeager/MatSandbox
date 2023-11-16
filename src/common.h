
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

MsResult MsInputSetAddArgument(MsInputSet* set, MsInputArgumentInitInfo info);
MsResult MsInputSetUpdateLayoutAndSet(MsInputSet* set);
MsResult MsInputSetPushBuffers(MsInputSet* set);
MsResult MsUpdateInputArgument(MsInputArgument* argument);

MsResult MsBufferAddElement(MsInputArgument* argument, MsBufferElementType type);

const char* MsGetShaderTypeExtension(OpalShaderType type);
MsResult MsCompileShader(OpalShaderType type, const char* source);
MsResult MsUpdateShader(OpalShaderType type);
MsResult MsUpdateMaterial();

uint32_t MsGetBufferElementSize(MsBufferElementType element);
uint32_t MsBufferOffsetToBaseAlignment(uint32_t offset, MsBufferElementType element); 

MsResult Render();

MsResult LoadMesh(const char* path, OpalMesh* outMesh);

#endif // !MATSANDBOX_COMMON_H
