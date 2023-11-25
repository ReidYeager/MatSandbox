
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
  printf("Can not currently serialize images\n");
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
  FILE* outFile = fopen(path, "w");

  WriteInputSet(outFile, &state.globalInputSet);

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
  printf("Can not currently serialize images\n");
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
  FILE* inFile = fopen(path, "r");

  MsInputSetShutdown(&state.materialInputSet);
  ReadInputSet(inFile, &state.materialInputSet);

  fclose(inFile);
  return Ms_Success;
}
