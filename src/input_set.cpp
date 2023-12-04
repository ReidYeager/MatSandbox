
#include "src/definesNew.h"
#include "src/input_set.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma region Helpers
uint32_t MsbBufferElementSize(MsbBufferElementType type)
{
  switch (type)
  {
  case Msb_Buffer_Float : case Msb_Buffer_Int : case Msb_Buffer_Uint : return 4;
  case Msb_Buffer_Float2: case Msb_Buffer_Int2: case Msb_Buffer_Uint2: return 8;
  case Msb_Buffer_Float3: case Msb_Buffer_Int3: case Msb_Buffer_Uint3: return 12;
  case Msb_Buffer_Float4: case Msb_Buffer_Int4: case Msb_Buffer_Uint4: return 16;

  case Msb_Buffer_Double: return 8;
  case Msb_Buffer_Double2: return 16;
  case Msb_Buffer_Double3: return 24;
  case Msb_Buffer_Double4: return 32;

  case Msb_Buffer_Mat4: return 64;

  default: return 0;
  }
}

uint32_t MsbBufferOffsetToBaseAlignment(uint32_t offset, MsbBufferElementType nextType)
{
  uint32_t alignment = 0;
  switch (nextType)
  {
  case Msb_Buffer_Float : case Msb_Buffer_Int : case Msb_Buffer_Uint : alignment = 4; break;
  case Msb_Buffer_Float2: case Msb_Buffer_Int2: case Msb_Buffer_Uint2: alignment = 8; break;

  case Msb_Buffer_Float3: case Msb_Buffer_Int3: case Msb_Buffer_Uint3:
  case Msb_Buffer_Float4: case Msb_Buffer_Int4: case Msb_Buffer_Uint4: alignment = 16; break;

  case Msb_Buffer_Double: alignment = 8; break;
  case Msb_Buffer_Double2: alignment = 16; break;
  case Msb_Buffer_Double3: case Msb_Buffer_Double4: alignment = 32; break;

  case Msb_Buffer_Mat4: alignment = 16; break;

  default: return offset;
  }

  alignment -= 1;
  return (offset + alignment) & ~alignment;
}

OpalInputType MsbInputTypeToOpalInputType(MsbInputArgumentType type)
{
  switch (type)
  {
  case Msb_Argument_Buffer: return Opal_Input_Type_Uniform_Buffer;
  case Msb_Argument_Image: return Opal_Input_Type_Samped_Image;
  default: MSB_ERR("Translating an unsupported MsbInputArgumentType : %d", type); return Opal_Input_Type_COUNT;
  }
}

MsbResult MsbLoadImageFile(const char* path, void** outImageData, Vec2U* outExtents)
{
  int channels;
  int32_t imageWidth = 0, imageHeight = 0;
  stbi_uc* imageSource = stbi_load(path, &imageWidth, &imageHeight, &channels, STBI_rgb_alpha);

  if (imageWidth <= 0 || imageWidth <= 0 || imageSource == NULL)
  {
    MSB_ERR("Unable to load image file '%s'\n", path);
    return Msb_Fail;
  }

  *outImageData = imageSource;
  *outExtents = { (uint32_t)imageWidth, (uint32_t)imageHeight };

  return Msb_Success;
}
#pragma endregion

MsbResult MsbInputSet::Init(std::vector<MsbInputArgumentInitInfo>& pInitInfo)
{
  for (uint32_t i = 0; i < pInitInfo.size(); i++)
  {
    MSB_ATTEMPT(AddArgument(&pInitInfo[i]));
  }

  MSB_ATTEMPT(UpdateLayoutAndSet());
  MSB_ATTEMPT(PushBuffersData());

  MSB_LOG("Input set init complete\n");
  return Msb_Success;
}

void MsbInputSet::Shutdown()
{
  if (set != OPAL_NULL_HANDLE)
    OpalInputSetShutdown(&set);
  if (layout != OPAL_NULL_HANDLE)
    OpalInputLayoutShutdown(&layout);

  for (uint32_t i = 0; i < pArguments.size(); i++)
  {
    switch (pArguments[i].type)
    {
    case Msb_Argument_Buffer: ShutdownBufferArgument(&pArguments[i]); break;
    case Msb_Argument_Image: ShutdownImageArgument(&pArguments[i]); break;
    default: continue;
    }
  }
  pArguments.clear();
}

MsbResult MsbInputSet::UpdateLayoutAndSet()
{
  if (set != OPAL_NULL_HANDLE)
    OpalInputSetShutdown(&set);
  if (layout != OPAL_NULL_HANDLE)
    OpalInputLayoutShutdown(&layout);

  uint32_t count = pArguments.size();
  std::vector<OpalMaterialInputValue> pInputValues(count);
  std::vector<OpalInputInfo> inputInfo(count);

  OpalInputLayoutInitInfo layoutInfo = { 0 };
  layoutInfo.count = count;
  layoutInfo.pTypes = LapisMemAllocZeroArray(OpalInputType, count);

  for (uint32_t i = 0; i < count; i++)
  {
    MsbInputArgument* arg = &pArguments[i];

    // Add to input layout
    layoutInfo.pTypes[i] = MsbInputTypeToOpalInputType(arg->type);

    // Add to input set
    OpalMaterialInputValue inValue = { 0 };
    switch (arg->type)
    {
    case Msb_Argument_Buffer: inValue.buffer = arg->data.buffer.opal; break;
    case Msb_Argument_Image: inValue.image = arg->data.image.opal; break;
    default: return Msb_Fail;
    }
    pInputValues[i] = inValue;

    inputInfo[i].index = i;
    inputInfo[i].type = MsbInputTypeToOpalInputType(arg->type);
    inputInfo[i].value = inValue;
  }

  MSB_ATTEMPT_OPAL(OpalInputLayoutInit(&layout, layoutInfo));

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = layout;
  setInfo.pInputValues = pInputValues.data();
  MSB_ATTEMPT_OPAL(OpalInputSetInit(&set, setInfo));

  MSB_ATTEMPT_OPAL(OpalInputSetUpdate(set, inputInfo.size(), inputInfo.data()));

  return Msb_Success;
}

MsbResult MsbInputSet::PushBuffersData()
{
  for (uint32_t i = 0; i < pArguments.size(); i++)
  {
    if (pArguments[i].type != Msb_Argument_Buffer)
      continue;

    MsbInputArgumentBuffer* barg = &pArguments[i].data.buffer;

    uint32_t offset = 0, elementSize = 0;
    for (uint32_t j = 0; j < barg->elementCount; j++)
    {
      elementSize = MsbBufferElementSize(barg->pElements[j].type);
      offset = MsbBufferOffsetToBaseAlignment(offset, barg->pElements[j].type);
      MSB_ATTEMPT_OPAL(OpalBufferPushDataSegment(barg->opal, barg->pElements[j].data, elementSize, offset));
      offset += elementSize;
    }
  }

  return Msb_Success;
}

// ===============
// Argument addition
// ===============
MsbResult MsbInputSet::AddArgument(MsbInputArgumentInitInfo* initInfo)
{
  MsbInputArgument newArg = { 0 };
  newArg.type = initInfo->type;

  switch (initInfo->type)
  {
  case Msb_Argument_Buffer: MSB_ATTEMPT(InitBufferArgument(initInfo, &newArg)); break;
  case Msb_Argument_Image: MSB_ATTEMPT(InitImageArgument(initInfo, &newArg)); break;
  default: MSB_ERR("Attempted addition of invalid input argument type : %u\n", initInfo->type); return Msb_Fail;
  }

  newArg.id = nextArgumentId++;
  newArg.name = "New argument";

  pArguments.push_back(newArg);

  return Msb_Success;
}

MsbResult MsbInputSet::InitBufferArgument(MsbInputArgumentInitInfo* initInfo, MsbInputArgument* outArg)
{
  MsbInputArgumentBuffer* barg = &outArg->data.buffer;
  uint32_t count = initInfo->buffer.elementCount;

  barg->elementCount = count;
  barg->pElements = LapisMemAllocZeroArray(MsbBufferElement, count);

  for (uint32_t i = 0; i < count; i++)
  {
    uint32_t elementSize = MsbBufferElementSize(initInfo->buffer.pElementTypes[i]);
    barg->size = MsbBufferOffsetToBaseAlignment(barg->size, initInfo->buffer.pElementTypes[i]);
    barg->size += elementSize;

    barg->pElements[i].type = initInfo->buffer.pElementTypes[i];
    barg->pElements[i].name = "test";
    barg->pElements[i].data = LapisMemAllocZero(elementSize);
  }

  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = barg->size;
  bufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MSB_ATTEMPT_OPAL(
    OpalBufferInit(&barg->opal, bufferInfo),
    {
        for (uint32_t i = 0; i < count; i++)
        {
          LapisMemFree(barg->pElements[i].data);
        }
        LapisMemFree(barg->pElements);
    });

  return Msb_Success;
}

MsbResult MsbInputSet::InitImageArgument(MsbInputArgumentInitInfo* initInfo, MsbInputArgument* outArg)
{
  MsbInputArgumentImage* iarg = &outArg->data.image;

  void* imageData = initInfo->image.imageData;
  Vec2U extents = initInfo->image.extents;
  iarg->sourcePath = NULL;

  if (initInfo->image.imagePath != NULL)
  {
    // Load image
    MSB_ATTEMPT(MsbLoadImageFile(initInfo->image.imagePath, &imageData, &extents));
    iarg->sourcePath = (char*)LapisMemAllocZero(strlen(initInfo->image.imagePath));
    LapisMemCopy(initInfo->image.imagePath, iarg->sourcePath, strlen(initInfo->image.imagePath));
  }

  // Create opal image
  // ===============
  OpalImageInitInfo imageInfo = { 0 };
  imageInfo.extent = { extents.x, extents.y, 1 };
  imageInfo.format = Opal_Format_RGBA8;
  imageInfo.usage = Opal_Image_Usage_Uniform;
  imageInfo.sampleType = Opal_Sample_Bilinear;
  MSB_ATTEMPT_OPAL(OpalImageInit(&iarg->opal, imageInfo));
  MSB_ATTEMPT_OPAL(OpalImageFill(iarg->opal, imageData));

  if (initInfo->image.imagePath != NULL)
  {
    stbi_image_free(imageData);
  }

  // Create single-image input set
  // ===============
  OpalMaterialInputValue inputImage;
  inputImage.image = iarg->opal;

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = msbSingleImageLayout;
  setInfo.pInputValues = &inputImage;
  MSB_ATTEMPT_OPAL(OpalInputSetInit(&iarg->set, setInfo));

  return Msb_Success;
}

// ===============
// Argument removal
// ===============
MsbResult MsbInputSet::RemoveArgument(uint32_t index)
{
  MsbInputArgument* arg = &pArguments[index];

  switch (arg->type)
  {
  case Msb_Argument_Buffer: MSB_ATTEMPT(ShutdownBufferArgument(arg)); break;
  case Msb_Argument_Image: MSB_ATTEMPT(ShutdownImageArgument(arg)); break;
  default: return Msb_Fail;
  }

  for (uint32_t i = index; i < pArguments.size() - 1; i++)
  {
    pArguments[i] = pArguments[i + 1];
  }
  pArguments.pop_back();

  return Msb_Success;
}

MsbResult MsbInputSet::ShutdownBufferArgument(MsbInputArgument* argument)
{
  MsbInputArgumentBuffer* barg = &argument->data.buffer;

  for (uint32_t i = 0; i < barg->elementCount; i++)
  {
    LapisMemFree(barg->pElements[i].data);
  }
  LapisMemFree(barg->pElements);

  OpalBufferShutdown(&barg->opal);

  barg->size = 0;
  barg->elementCount = 0;

  return Msb_Success;
}

MsbResult MsbInputSet::ShutdownImageArgument(MsbInputArgument* argument)
{
  MsbInputArgumentImage* iarg = &argument->data.image;

  OpalImageShutdown(&iarg->opal);
  OpalInputSetShutdown(&iarg->set);
  LapisMemFree(iarg->sourcePath);
  iarg->sourcePath = NULL;

  return Msb_Success;
}
