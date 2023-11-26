
#include "src/common.h"

#include <stdio.h>

void WriteBuffer(FILE* outFile, MsInputArgumentBuffer* buffer)
{
  fwrite(&buffer->elementCount, sizeof(uint32_t), 1, outFile);

  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    MsBufferElement* e = &buffer->pElements[i];
    fwrite(&e->type, sizeof(e->type), 1, outFile);
  }

  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    MsBufferElement* e = &buffer->pElements[i];
    fwrite(e->data, MsGetBufferElementSize(e->type), 1, outFile);
  }
}

void WriteImage(FILE* outFile, MsInputArgumentImage* image)
{
  void* imageData;
  uint32_t imageSize = OpalImageDumpData(image->image, &imageData);

  uint32_t pixelCount = image->image->extents.width * image->image->extents.height * image->image->extents.depth;
  fwrite(&image->image->extents, sizeof(OpalExtent), 1, outFile);
  fwrite(&image->image->format, sizeof(OpalFormat), 1, outFile);

  if (imageSize == 0 || pixelCount == 0)
  {
    if (imageData)
      LapisMemFree(imageData);
    return;
  }

  fwrite(imageData, OpalFormatToSize(image->image->format), pixelCount, outFile);

  LapisMemFree(imageData);
}

void WriteInputSet(FILE* outFile, MsInputSet* set)
{
  fwrite(&set->count, sizeof(uint32_t), 1, outFile);

  for (uint32_t i = 0; i < set->count; i++)
  {
    MsInputArgument* arg = &set->pArguments[i];
    fwrite(&arg->type, sizeof(arg->type), 1, outFile);
    switch (arg->type)
    {
    case Ms_Input_Buffer: WriteBuffer(outFile, &arg->data.buffer); break;
    case Ms_Input_Image: WriteImage(outFile, &arg->data.image); break;
    }
  }
}

MsResult MsSerializeSave(const char* path)
{
  FILE* outFile = fopen(path, "wb");

  WriteInputSet(outFile, &state.materialInputSet);

  fclose(outFile);

  return Ms_Success;
}

// ======
// Load
// ======

void ReadBuffer(FILE* inFile, MsInputSet* set)
{
  MsInputArgumentInitInfo info;
  info.type = Ms_Input_Buffer;
  fread(&info.bufferInfo.elementCount, sizeof(uint32_t), 1, inFile);
  info.bufferInfo.pElementTypes = LapisMemAllocArray(MsBufferElementType, info.bufferInfo.elementCount);
  fread(info.bufferInfo.pElementTypes, sizeof(MsBufferElementType), info.bufferInfo.elementCount, inFile);
  MsInputSetAddArgument(set, info);

  MsInputArgumentBuffer* buffer = &set->pArguments[set->count - 1].data.buffer;

  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    MsBufferElement* e = &buffer->pElements[i];
    fread(e->data, MsGetBufferElementSize(e->type), 1, inFile);
  }
}

void ReadImage(FILE* inFile, MsInputSet* set)
{
  OpalExtent extents;
  OpalFormat format;

  fread(&extents, sizeof(OpalExtent), 1, inFile);
  fread(&format, sizeof(OpalFormat), 1, inFile);

  uint32_t pixelCount = extents.width * extents.height * extents.depth;
  uint32_t imageSize = pixelCount * OpalFormatToSize(format);
  void* imageData = LapisMemAlloc(imageSize);
  fread(imageData, OpalFormatToSize(format), pixelCount, inFile);

  MsInputArgumentInitInfo info;
  info.type = Ms_Input_Image;
  info.imageInfo.imagePath = NULL;
  info.imageInfo.extents = extents;
  info.imageInfo.imageData = imageData;
  MsInputSetAddArgument(set, info);
}

void ReadInputSet(FILE* inFile, MsInputSet* set)
{
  uint32_t count;
  fread(&count, sizeof(uint32_t), 1, inFile);

  for (uint32_t i = 0; i < count; i++)
  {
    MsInputType type;
    fread(&type, sizeof(MsInputType), 1, inFile);

    switch (type)
    {
    case Ms_Input_Buffer: ReadBuffer(inFile, set); break;
    case Ms_Input_Image: ReadImage(inFile, set); break;
    }
  }
}

MsResult MsSerializeLoad(const char* path)
{
  FILE* inFile = fopen(path, "rb");

  MsInputSetShutdown(&state.materialInputSet);
  ReadInputSet(inFile, &state.materialInputSet);

  fclose(inFile);
  return Ms_Success;
}
