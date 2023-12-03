
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

MsbResult MsInit();
MsbResult MsUpdate();
void MsShutdown();

MsbResult UpdateCamera();

MsbResult RenderUi();

MsbResult MsInputSetAddArgument(MsInputSet* set, MsInputArgumentInitInfo info);
void MsInputSetRemoveArgument(MsInputSet* set, uint32_t);
MsbResult MsInputSetUpdateLayoutAndSet(MsInputSet* set);
MsbResult MsInputSetPushBuffers(MsInputSet* set);
MsbResult MsUpdateInputArgument(MsInputArgument* argument);
MsbResult MsInputSetReloadImage(MsInputSet* set, uint32_t imageIndex, char* path);

MsbResult MsBufferAddElement(MsInputArgument* argument, MsBufferElementType type);
void MsBufferRemoveElement(MsInputArgumentBuffer* buffer, uint32_t index);

const char* MsGetShaderTypeExtension(OpalShaderType type);
MsbResult MsCompileShader(ShaderCodeInfo* codeInfo, const char* source);
MsbResult MsUpdateShader(ShaderCodeInfo* codeInfo);
MsbResult MsUpdateMaterial();

void MsShaderAddToCompileQueue(ShaderCodeInfo* codeInfo);
MsbResult MsCompileQueuedShaders();
MsbResult MsReimportQueuedImages();

uint32_t MsGetBufferElementSize(MsBufferElementType element);
uint32_t MsBufferOffsetToBaseAlignment(uint32_t offset, MsBufferElementType element); 

MsbResult MsUiShowArgumentsPanel();
MsbResult Render();

MsbResult LoadMesh(const char* path, OpalMesh* outMesh);

void MsInputSetShutdown(MsInputSet* set);
MsbResult MsSerializeSave(const char* path, bool embedImages);
MsbResult MsSerializeLoad(const char* path);

#endif // !MATSANDBOX_COMMON_H
