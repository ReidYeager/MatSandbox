
#include "src/common.h"

#include <lapis.h>

uint32_t MsBufferElementSize(MsBufferElementType element)
{
  switch (element)
  {
  case Ms_Buffer_Float : case Ms_Buffer_Int : case Ms_Buffer_Uint : return 4;
  case Ms_Buffer_Float2: case Ms_Buffer_Int2: case Ms_Buffer_Uint2: return 8;
  case Ms_Buffer_Float3: case Ms_Buffer_Int3: case Ms_Buffer_Uint3: return 12;
  case Ms_Buffer_Float4: case Ms_Buffer_Int4: case Ms_Buffer_Uint4: return 16;

  case Ms_Buffer_double : return 8;
  case Ms_Buffer_double2: return 16;
  case Ms_Buffer_double3: return 24;
  case Ms_Buffer_double4: return 32;

  case Ms_Buffer_Mat4: return 64;

  default: return 0;
  }
}

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

MsResult CreateBuffer(MsInputArgumentBuffer* argument)
{
  if (argument == NULL)
    return Ms_Fail;

  if (argument->buffer != NULL)
    OpalBufferShutdown(&argument->buffer);

  argument->size = 0;

  for (uint32_t i = 0, elementSize = 0; i < argument->elementCount; i++)
  {
    elementSize = MsBufferElementSize(argument->pElements[i].type);
    if (argument->pElements[i].data == NULL)
    {
      argument->pElements[i].data = LapisMemAllocZero(elementSize);
    }

    argument->size += elementSize;
  }

  if (argument->size == 0)
    return Ms_Fail;

  OpalBufferInitInfo initInfo = { 0 };
  initInfo.usage = Opal_Buffer_Usage_Uniform;
  initInfo.size = argument->size;
  MS_ATTEMPT_OPAL(OpalBufferInit(&argument->buffer, initInfo));

  for (uint32_t i = 0; i < argument->elementCount; i++)
  {
    MS_ATTEMPT_OPAL(OpalBufferPushDataSegment(argument->buffer, argument->pElements[i].data, MsBufferElementSize(argument->pElements[i].type), argument->size));
    LapisLogDebug("Pushed data for %u\n", i);
  }

  return Ms_Success;
}

MsResult MsUpdateMaterialInputLayoutAndSet()
{
  if (state.materialInfo.inputSet != NULL)
    OpalInputSetShutdown(&state.materialInfo.inputSet);
  if (state.materialInfo.inputLayout != NULL)
    OpalInputLayoutShutdown(&state.materialInfo.inputLayout);

  OpalInputLayoutInitInfo layoutInfo = { 0 };
  layoutInfo.count = state.materialInfo.inputArgumentCount;
  layoutInfo.pTypes = LapisMemAllocZeroArray(OpalInputType, layoutInfo.count);

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.pInputValues = LapisMemAllocZeroArray(OpalMaterialInputValue, layoutInfo.count);

  for (uint32_t i = 0; i < state.materialInfo.inputArgumentCount; i++)
  {
    if (state.materialInfo.pInputArguements[i].type == Ms_Input_Buffer)
    {
      MS_ATTEMPT(CreateBuffer(&state.materialInfo.pInputArguements[i].data.buffer));
      layoutInfo.pTypes[i] = Opal_Input_Type_Uniform_Buffer;
      setInfo.pInputValues[i].buffer = state.materialInfo.pInputArguements[i].data.buffer.buffer;
    }
    else
    {
      layoutInfo.pTypes[i] = Opal_Input_Type_Samped_Image;
      setInfo.pInputValues[i].image = state.materialInfo.pInputArguements[i].data.image.image;
    }
  }

  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&state.materialInfo.inputLayout, layoutInfo));
  setInfo.layout = state.materialInfo.inputLayout;

  MS_ATTEMPT_OPAL(OpalInputSetInit(&state.materialInfo.inputSet, setInfo));

  // Send arguments to the input set
  OpalInputInfo* inputInfo = LapisMemAllocZeroArray(OpalInputInfo, state.materialInfo.inputArgumentCount);
  for (uint32_t i = 0; i < state.materialInfo.inputArgumentCount; i++)
  {
    MsInputArgument* argument = &state.materialInfo.pInputArguements[i];

    inputInfo->index = i;

    switch (argument->type)
    {
    case Ms_Input_Buffer:
    {
      inputInfo->type = Opal_Input_Type_Uniform_Buffer;
      inputInfo->value.buffer = argument->data.buffer.buffer;
    } break;
    case Ms_Input_Image:
    {
      inputInfo->type = Opal_Input_Type_Samped_Image;
      inputInfo->value.image = argument->data.image.image;
    } break;
    default: return Ms_Fail;
    }
  }
  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.materialInfo.inputSet, state.materialInfo.inputArgumentCount, inputInfo));
  LapisMemFree(inputInfo);

  LapisLogDebug("SDLFKJSDLKFJSDLKFJ\n");
  return Ms_Success;
}

MsResult MsUpdateMaterial()
{
  MS_ATTEMPT(MsUpdateMaterialInputLayoutAndSet());

  //MS_ATTEMPT_OPAL(OpalMaterialReinit(state.material));
  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = state.shaderCount;
  matInfo.pShaders = state.pShaders;
  matInfo.inputLayoutCount = 2;
  OpalInputLayout layouts[2] = { state.globalInputLayout, state.materialInfo.inputLayout };
  matInfo.pInputLayouts = layouts;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = state.renderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  return Ms_Success;
}


