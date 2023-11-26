
#include "src/common.h"

#include <lapis.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

uint32_t MsGetBufferElementSize(MsBufferElementType element)
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

const char* MsGetShaderTypeExtension(OpalShaderType type)
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

  char shaderNameBuffer[MS_SHADER_NAME_MAX_LENGTH];
  sprintf_s(shaderNameBuffer, MS_SHADER_NAME_MAX_LENGTH, "NewShaderSource.%s", MsGetShaderTypeExtension(type));

  OpalShader* shader = &state.pShaders[isFragment];
  OpalShaderInitInfo initInfo = {};
  initInfo.type = type;
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
    elementSize = MsGetBufferElementSize(argument->pElements[i].type);
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
    MS_ATTEMPT_OPAL(OpalBufferPushDataSegment(argument->buffer, argument->pElements[i].data, MsGetBufferElementSize(argument->pElements[i].type), argument->size));
  }

  return Ms_Success;
}

MsResult MsInputSetUpdateLayoutAndSet(MsInputSet* set)
{
  if (set->set != NULL)
    OpalInputSetShutdown(&set->set);
  if (set->layout != NULL)
    OpalInputLayoutShutdown(&set->layout);

  OpalInputLayoutInitInfo layoutInfo = { 0 };
  layoutInfo.count = set->count;
  layoutInfo.pTypes = LapisMemAllocZeroArray(OpalInputType, layoutInfo.count);

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.pInputValues = LapisMemAllocZeroArray(OpalMaterialInputValue, layoutInfo.count);

  for (uint32_t i = 0; i < set->count; i++)
  {
    if (set->pArguments[i].type == Ms_Input_Buffer)
    {
      layoutInfo.pTypes[i] = Opal_Input_Type_Uniform_Buffer;
      setInfo.pInputValues[i].buffer = set->pArguments[i].data.buffer.buffer;
    }
    else
    {
      layoutInfo.pTypes[i] = Opal_Input_Type_Samped_Image;
      setInfo.pInputValues[i].image = set->pArguments[i].data.image.image;
    }
  }

  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&set->layout, layoutInfo));
  setInfo.layout = set->layout;

  MS_ATTEMPT_OPAL(OpalInputSetInit(&set->set, setInfo));

  // Send arguments to the input set
  OpalInputInfo* inputInfo = LapisMemAllocZeroArray(OpalInputInfo, set->count);
  for (uint32_t i = 0; i < set->count; i++)
  {
    MsInputArgument* argument = &set->pArguments[i];

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
  MS_ATTEMPT_OPAL(OpalInputSetUpdate(set->set, set->count, inputInfo));
  LapisMemFree(inputInfo);

  return Ms_Success;
}

uint32_t tmp = 0;

MsResult MsUpdateMaterial()
{
  MS_ATTEMPT(MsInputSetUpdateLayoutAndSet(&state.materialInputSet));

  printf("%u\n", tmp);

  //MS_ATTEMPT_OPAL(OpalMaterialReinit(state.material));
  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = state.shaderCount;
  matInfo.pShaders = state.pShaders;
  matInfo.inputLayoutCount = 2;
  OpalInputLayout layouts[2] = { state.globalInputSet.layout, state.materialInputSet.layout };
  matInfo.pInputLayouts = layouts;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = state.sceneRenderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  return Ms_Success;
}

MsResult MsInputSetPushBuffers(MsInputSet* set)
{
  for (uint32_t argi = 0; argi < set->count; argi++)
  {
    MsInputArgument* argument = &set->pArguments[argi];

    if (argument->type == Ms_Input_Buffer)
    {
      MsInputArgumentBuffer* buffer = &argument->data.buffer;
      MsBufferElement* element = NULL;
      uint32_t offset = 0, elementSize = 0;

      for (uint32_t i = 0; i < argument->data.buffer.elementCount; i++)
      {
        element = &buffer->pElements[i];

        offset = MsBufferOffsetToBaseAlignment(offset, element->type);

        elementSize = MsGetBufferElementSize(element->type);
        MS_ATTEMPT_OPAL(OpalBufferPushDataSegment(buffer->buffer, buffer->pElements[i].data, elementSize, offset));
        offset += elementSize;
      }
    }
  }

  return Ms_Success;
}

MsResult MsBufferAddElement(MsInputArgument* argument, MsBufferElementType type)
{
  MsInputArgumentBuffer* buffer = &argument->data.buffer;

  // Add element
  uint32_t newElementIndex = buffer->elementCount;

  buffer->size = MsBufferOffsetToBaseAlignment(buffer->size, type);
  buffer->size += MsGetBufferElementSize(type);

  buffer->elementCount++;
  buffer->pElements = LapisMemReallocArray(buffer->pElements, MsBufferElement, buffer->elementCount);

  MsBufferElement* newElement = &buffer->pElements[newElementIndex];

  newElement->name = LapisMemAllocArray(char, 128);
  sprintf(newElement->name, "%u:%u %s", argument->id, buffer->elementCount - 1, MsBufferElementTypeNames[type]);
  newElement->type = type;
  newElement->data = LapisMemAllocZero(MsGetBufferElementSize(type));

  // Update Opal buffer
  OpalBufferShutdown(&buffer->buffer);

  OpalBufferInitInfo bufferInfo;
  bufferInfo.size = buffer->size;
  bufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MS_ATTEMPT_OPAL(OpalBufferInit(&buffer->buffer, bufferInfo));

  // Update material input set
  uint32_t bufferArgIndex = 0;
  while (&state.materialInputSet.pArguments[bufferArgIndex].data.buffer != buffer)
  {
    bufferArgIndex++;
  }

  OpalInputInfo inputInfo;
  inputInfo.index = bufferArgIndex;
  inputInfo.type = Opal_Input_Type_Uniform_Buffer;
  inputInfo.value.buffer = buffer->buffer;

  MS_ATTEMPT_OPAL(OpalInputSetUpdate(state.materialInputSet.set, 1, &inputInfo));

  return Ms_Success;
}

void MsBufferRemoveElement(MsInputArgumentBuffer* buffer, uint32_t index)
{
  LapisMemFree(buffer->pElements[index].name);
  LapisMemFree(buffer->pElements[index].data);

  for (uint32_t i = index; i < buffer->elementCount - 1; i ++)
  {
    buffer->pElements[i] = buffer->pElements[i + 1];
  }
  buffer->elementCount--;
}

MsResult InitBufferArgument(MsInputArgument* argument, MsInputArgumentInitInfo info)
{
  MsBufferElement* pElements = LapisMemAllocZeroArray(MsBufferElement, info.bufferInfo.elementCount);

  uint32_t bufferSize = 0;
  for (uint32_t i = 0; i < info.bufferInfo.elementCount; i++)
  {
    uint32_t elementSize = MsGetBufferElementSize(info.bufferInfo.pElementTypes[i]);
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

MsResult ImageArgumentLoadFromFile(const char* path, void** imageData, OpalExtent* extents)
{
  int channels;
  int32_t imageWidth = 0, imageHeight = 0;
  stbi_uc* imageSource = stbi_load(path, &imageWidth, &imageHeight, &channels, STBI_rgb_alpha);

  if (imageWidth <= 0 || imageWidth <= 0 || imageSource == NULL)
  {
    LapisConsolePrintMessage(Lapis_Console_Error, "Unable to load specified image file\n>> \"%s\"", path);
    return Ms_Fail;
  }

  *imageData = imageSource;
  *extents = { (uint32_t)imageWidth, (uint32_t)imageHeight, 1 };

  return Ms_Success;
}

MsResult InitImageArgument(MsInputArgument* argument, MsInputArgumentInitInfo info)
{
  MsInputArgumentImage* image = &argument->data.image;

  void* imageData = info.imageInfo.imageData;
  OpalExtent extent = info.imageInfo.extents;

  if (info.imageInfo.imagePath != NULL)
  {
    MS_ATTEMPT(ImageArgumentLoadFromFile(info.imageInfo.imagePath, &imageData, &extent));
  }

  OpalImageInitInfo initInfo;
  initInfo.extent = extent;
  initInfo.sampleType = Opal_Sample_Bilinear;
  initInfo.usage = Opal_Image_Usage_Uniform;
  initInfo.format = Opal_Format_RGBA8;
  MS_ATTEMPT_OPAL(OpalImageInit(&image->image, initInfo));
  MS_ATTEMPT_OPAL(OpalImageFill(image->image, imageData));

  if (info.imageInfo.imagePath != NULL)
  {
    stbi_image_free(imageData);
  }

  OpalMaterialInputValue inImage;
  inImage.image = image->image;

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = state.uiSingleImageInputLayout;
  setInfo.pInputValues = &inImage;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&image->set, setInfo));

  return Ms_Success;
}

MsResult MsInputSetAddArgument(MsInputSet* set, MsInputArgumentInitInfo info)
{
  MsInputArgument newArgument = {};

  newArgument.type = info.type;
  newArgument.id = set->nextId++;
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

  set->count++;
  if (set->count > 1)
    set->pArguments = LapisMemReallocArray(set->pArguments, MsInputArgument, set->count);
  else
    set->pArguments = LapisMemAllocArray(MsInputArgument, set->count);
  set->pArguments[set->count - 1] = newArgument;

  MS_ATTEMPT(MsInputSetUpdateLayoutAndSet(set));

  return Ms_Success;
}

void MsInputSetRemoveArgument(MsInputSet* set, uint32_t index)
{
  MsInputArgument* argument = &set->pArguments[index];
  LapisMemFree(argument->name);
  if (argument->type == Ms_Input_Buffer)
  {
    OpalBufferShutdown(&argument->data.buffer.buffer);
    for (uint32_t i = 0; i < argument->data.buffer.elementCount; i++)
    {
      LapisMemFree(argument->data.buffer.pElements[i].name);
      LapisMemFree(argument->data.buffer.pElements[i].data);
    }
    LapisMemFree(argument->data.buffer.pElements);
    argument->data.buffer.elementCount = 0;
    argument->data.buffer.size = 0;
  }
  else
  {
    OpalImageShutdown(&argument->data.image.image);
    OpalInputSetShutdown(&argument->data.image.set);
  }

  for (uint32_t i = index; i < set->count - 1; i++)
  {
    set->pArguments[i] = set->pArguments[i + 1];
  }

  set->count--;
}

MsResult MsInputSetReloadImage(MsInputSet* set, uint32_t imageIndex, char* path)
{
  MsInputArgumentImage* image = &set->pArguments[imageIndex].data.image;

  int channels;
  int32_t imageWidth = 0, imageHeight = 0;
  stbi_uc* imageSource = stbi_load(
    path,
    &imageWidth,
    &imageHeight,
    &channels,
    STBI_rgb_alpha);

  if (imageWidth <= 0 || imageWidth <= 0 || imageSource == NULL)
  {
    LapisConsolePrintMessage(Lapis_Console_Error, "Unable to load specified image file\n>> \"%s\"", path);
    return Ms_Fail;
  }

  OpalImageShutdown(&image->image);

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
