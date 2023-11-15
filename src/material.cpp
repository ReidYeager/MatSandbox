
#include "src/common.h"

#include <lapis.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

uint32_t MsBufferElementSize(MsBufferElementType element)
{
  switch (element)
  {
  case Ms_Buffer_Float : case Ms_Buffer_Int : case Ms_Buffer_Uint : return 4;
  case Ms_Buffer_Float2: case Ms_Buffer_Int2: case Ms_Buffer_Uint2: return 8;
  case Ms_Buffer_Float3: case Ms_Buffer_Int3: case Ms_Buffer_Uint3: return 12;
  case Ms_Buffer_Float4: case Ms_Buffer_Int4: case Ms_Buffer_Uint4: return 16;

  case Ms_Buffer_Double : return 8;
  case Ms_Buffer_Double2: return 16;
  case Ms_Buffer_Double3: return 24;
  case Ms_Buffer_Double4: return 32;

  case Ms_Buffer_Mat4: return 64;

  default: return 0;
  }
}

uint32_t MsBufferOffsetToBaseAlignment(uint32_t offset, MsBufferElementType element)
{
  uint32_t alignment = 0;
  switch (element)
  {
  case Ms_Buffer_Float: case Ms_Buffer_Int: case Ms_Buffer_Uint: alignment = 4; break;
  case Ms_Buffer_Float2: case Ms_Buffer_Int2: case Ms_Buffer_Uint2: alignment = 8; break;

  case Ms_Buffer_Float3: case Ms_Buffer_Int3: case Ms_Buffer_Uint3:
  case Ms_Buffer_Float4: case Ms_Buffer_Int4: case Ms_Buffer_Uint4: alignment = 16; break;

  case Ms_Buffer_Double: alignment = 8; break;
  case Ms_Buffer_Double2: alignment = 16; break;
  case Ms_Buffer_Double3: case Ms_Buffer_Double4: alignment = 32; break;

  case Ms_Buffer_Mat4: alignment = 16; break;

  default: return offset;
  }

  alignment -= 1;
  return (offset + alignment) & ~alignment;
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

    inputInfo[i].index = i;

    switch (argument->type)
    {
    case Ms_Input_Buffer:
    {
      inputInfo[i].type = Opal_Input_Type_Uniform_Buffer;
      inputInfo[i].value.buffer = argument->data.buffer.buffer;
    } break;
    case Ms_Input_Image:
    {
      inputInfo[i].type = Opal_Input_Type_Samped_Image;
      inputInfo[i].value.image = argument->data.image.image;
    } break;
    default: return Ms_Fail;
    }
  }
  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.materialInfo.inputSet, state.materialInfo.inputArgumentCount, inputInfo));
  LapisMemFree(inputInfo);

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
  matInfo.renderpass = state.sceneRenderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  return Ms_Success;
}

void MsUpdateMaterialValues()
{
  for (uint32_t argi = 0; argi < state.materialInfo.inputArgumentCount; argi++)
  {
    MsUpdateInputArgument(state.materialInfo.pInputArguements + argi);
  }
}

MsResult MsBufferAddElement(MsInputArgument* argument, MsBufferElementType type)
{
  MsInputArgumentBuffer* buffer = &argument->data.buffer;

  // Add element
  uint32_t newElementIndex = buffer->elementCount;

  buffer->size = MsBufferOffsetToBaseAlignment(buffer->size, type);
  buffer->size += MsBufferElementSize(type);

  buffer->elementCount++;
  buffer->pElements = LapisMemReallocArray(buffer->pElements, MsBufferElement, buffer->elementCount);

  MsBufferElement* newElement = &buffer->pElements[newElementIndex];

  newElement->name = LapisMemAllocArray(char, 128);
  sprintf(newElement->name, "%u:%u %s", argument->id, buffer->elementCount - 1, MsBufferElementTypeNames[type]);
  newElement->type = type;
  newElement->data = LapisMemAllocZero(MsBufferElementSize(type));

  // Update Opal buffer
  OpalBufferShutdown(&buffer->buffer);

  OpalBufferInitInfo bufferInfo;
  bufferInfo.size = buffer->size;
  bufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MS_ATTEMPT_OPAL(OpalBufferInit(&buffer->buffer, bufferInfo));

  // Update material input set
  uint32_t bufferArgIndex = 0;
  while (&state.materialInfo.pInputArguements[bufferArgIndex].data.buffer != buffer)
  {
    bufferArgIndex++;
  }

  OpalInputInfo inputInfo;
  inputInfo.index = bufferArgIndex;
  inputInfo.type = Opal_Input_Type_Uniform_Buffer;
  inputInfo.value.buffer = buffer->buffer;

  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.materialInfo.inputSet, 1, &inputInfo));

  return Ms_Success;
}

MsResult InitBufferArgument(MsInputArgument* argument, MsInputArgumentInitInfo info)
{
  MsBufferElement* pElements = LapisMemAllocZeroArray(MsBufferElement, info.bufferInfo.elementCount);

  uint32_t bufferSize = 0;
  for (uint32_t i = 0; i < info.bufferInfo.elementCount; i++)
  {
    uint32_t elementSize = MsBufferElementSize(info.bufferInfo.pElementTypes[i]);
    bufferSize = MsBufferOffsetToBaseAlignment(bufferSize, pElements[i].type);
    bufferSize += elementSize;
    pElements[i].type = info.bufferInfo.pElementTypes[i];
    pElements[i].data = LapisMemAllocZero(elementSize);
    pElements[i].name = LapisMemAllocArray(char, 128);
    sprintf(pElements[i].name, "%u:%u %s", argument->id, i, MsBufferElementTypeNames[info.bufferInfo.pElementTypes[i]]);
  }

  OpalBufferInitInfo bufferInfo;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MS_ATTEMPT_OPAL(
    OpalBufferInit(&argument->data.buffer.buffer, bufferInfo),
    {
      for (uint32_t i = 0; i < info.bufferInfo.elementCount; i++)
      {
        LapisMemFree(pElements[i].data);
      }
      LapisMemFree(pElements);
    });

  argument->data.buffer.elementCount = info.bufferInfo.elementCount;
  argument->data.buffer.pElements = pElements;
  argument->data.buffer.size = bufferSize;

  return Ms_Success;
}

MsResult InitImageArgument(MsInputArgument* argument, MsInputArgumentInitInfo info)
{
  MsInputArgumentImage* image = &argument->data.image;

  int channels;
  int32_t imageWidth = 0, imageHeight = 0;
  stbi_uc* imageSource = stbi_load(
  info.imageInfo.imagePath,
  &imageWidth,
  &imageHeight,
  &channels,
  STBI_rgb_alpha);

  if (imageWidth <= 0 || imageWidth <= 0 || imageSource == NULL)
  {
    LapisConsolePrintMessage(Lapis_Console_Error, "Unable to load specified image file\n>> \"%s\"", info.imageInfo.imagePath);
    return Ms_Fail;
  }

  OpalImageInitInfo initInfo;
  initInfo.extent = { (uint32_t)imageWidth, (uint32_t)imageHeight, 1 };
  initInfo.sampleType = Opal_Sample_Bilinear;
  initInfo.usage = Opal_Image_Usage_Uniform;
  initInfo.format = Opal_Format_RGBA8;
  MS_ATTEMPT_OPAL(OpalImageInit(&image->image, initInfo));
  MS_ATTEMPT_OPAL(OpalImageFill(image->image, imageSource));

  stbi_image_free(imageSource);

  OpalMaterialInputValue inImage;
  inImage.image = image->image;

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = state.uiSingleImageInputLayout;
  setInfo.pInputValues = &inImage;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&image->set, setInfo));

  return Ms_Success;
}

MsResult MsCreateInputArgument(MsInputArgumentInitInfo info, uint32_t* outArgumentIndex)
{
  MsInputArgument newArgument = {};

  newArgument.type = info.type;
  newArgument.id = state.materialInfo.inputArgumentNextId++;
  *outArgumentIndex = state.materialInfo.inputArgumentCount;
  state.materialInfo.inputArgumentCount++;
  newArgument.name = LapisMemAllocArray(char, 128);
  sprintf(newArgument.name, "Input argument %u", newArgument.id);

  if (info.type == Ms_Input_Buffer)
  {
    MS_ATTEMPT(InitBufferArgument(&newArgument, info));
  }
  else
  {
    MS_ATTEMPT(InitImageArgument(&newArgument, info));
  }

  state.materialInfo.pInputArguements = (MsInputArgument*)LapisMemRealloc(state.materialInfo.pInputArguements, sizeof(MsInputArgument) * state.materialInfo.inputArgumentCount);
  state.materialInfo.pInputArguements[state.materialInfo.inputArgumentCount - 1] = newArgument;
  //LapisMemCopy(&newArgument, state.materialInfo.pInputArguements + *outArgumentIndex, sizeof(MsInputArgument));

  return Ms_Success;
}

MsResult MsUpdateInputArgument(MsInputArgument* argument)
{
  if (argument->type == Ms_Input_Buffer)
  {
    MsInputArgumentBuffer* buffer = &argument->data.buffer;
    MsBufferElement* element = NULL;
    uint32_t offset = 0, elementSize = 0;

    for (uint32_t i = 0; i < argument->data.buffer.elementCount; i++)
    {
      element = &buffer->pElements[i];

      offset = MsBufferOffsetToBaseAlignment(offset, element->type);

      elementSize = MsBufferElementSize(element->type);
      MS_ATTEMPT_OPAL(OpalBufferPushDataSegment(buffer->buffer, buffer->pElements[i].data, elementSize, offset));
      offset += elementSize;
    }
  }

  return Ms_Success;
}
