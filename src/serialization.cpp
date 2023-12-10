
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

void WriteImage(FILE* outFile, MsInputArgumentImage* image, MsbSerializeSaveFlags flags)
{
  if (! (flags & Msb_Save_Images_Embed))
  {
    uint32_t pathLength = strlen(image->sourcePath); // Does not include '\0'
    const char* resPath = GAME_RESOURCE_PATH;

    // Try to ignore the resource path
    uint32_t i = 0;
    while (i < pathLength && image->sourcePath[i] != '\0')
    {
      if (resPath[i] == '\0')
        break;

      if (image->sourcePath[i] != resPath[i])
      {
        i = 0;
        break;
      }

      i++;
    }

    pathLength -= i;
    char path[1024];
    
    if (i > 0)
    {
      sprintf(path, "%s%s", MSB_RES_IMAGE_KEY, image->sourcePath + i);
      pathLength += strlen(MSB_RES_IMAGE_KEY);
    }
    else
    {
      sprintf(path, "%s", image->sourcePath);
    }

    fwrite(&pathLength, sizeof(uint32_t), 1, outFile);
    fwrite(path, sizeof(char), pathLength, outFile);
  }
  else
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
}

void WriteInputSet(FILE* outFile, MsInputSet* set, MsbSerializeSaveFlags flags)
{
  fwrite(&set->count, sizeof(uint32_t), 1, outFile);

  for (uint32_t i = 0; i < set->count; i++)
  {
    MsInputArgument* arg = &set->pArguments[i];
    fwrite(&arg->type, sizeof(arg->type), 1, outFile);
    switch (arg->type)
    {
    case Ms_Input_Buffer: WriteBuffer(outFile, &arg->data.buffer); break;
    case Ms_Input_Image: WriteImage(outFile, &arg->data.image, flags); break;
    }
  }
}

void WriteShaders(FILE* outFile)
{
  fwrite(&state.shaderCount, sizeof(uint32_t), 1, outFile);

  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    ShaderCodeInfo* shader = &state.pShaderCodeInfos[i];
    fwrite(&shader->type, sizeof(OpalShaderType), 1, outFile);
    fwrite(&shader->size, sizeof(uint32_t), 1, outFile);
    fwrite(shader->buffer, sizeof(char), shader->size, outFile);
  }
}

MsbResult MsSerializeSave(const char* path, MsbSerializeSaveFlags flags)
{
  FILE* outFile = fopen(path, "wb");

  uint32_t fileFingerprint = MSB_SAVEFILE_FINGERPRINT | MSB_SAVEFILE_VERSION;
  fwrite(&fileFingerprint, sizeof(uint32_t), 1, outFile);
  fwrite(&flags, sizeof(uint32_t), 1, outFile);

  WriteShaders(outFile);
  WriteInputSet(outFile, &state.materialInputSet, flags);

  fclose(outFile);

  return Msb_Success;
}

// ======
// Load
// ======

MsbResult ReadBuffer(FILE* inFile, MsInputSet* set, MsbSerializeSaveFlags flags)
{
  MsInputArgumentInitInfo info;
  info.type = Ms_Input_Buffer;
  fread(&info.bufferInfo.elementCount, sizeof(uint32_t), 1, inFile);
  info.bufferInfo.pElementTypes = LapisMemAllocArray(MsBufferElementType, info.bufferInfo.elementCount);
  fread(info.bufferInfo.pElementTypes, sizeof(MsBufferElementType), info.bufferInfo.elementCount, inFile);
  MSB_ATTEMPT(MsInputSetAddArgument(set, info));

  MsInputArgumentBuffer* buffer = &set->pArguments[set->count - 1].data.buffer;

  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    MsBufferElement* e = &buffer->pElements[i];
    fread(e->data, MsGetBufferElementSize(e->type), 1, inFile);
  }

  return Msb_Success;
}

MsbResult ReadImage(FILE* inFile, MsInputSet* set, MsbSerializeSaveFlags flags)
{
  MsInputArgumentInitInfo newImageInfo;
  newImageInfo.type = Ms_Input_Image;
  newImageInfo.imageInfo.imagePath = NULL;

  if ((flags & Msb_Save_Images_Embed) == 0)
  {
    uint32_t pathLength;
    fread(&pathLength, sizeof(uint32_t), 1, inFile); // Does not include '\0'
    char pathBuffer[1024];
    fread(pathBuffer, sizeof(char), pathLength, inFile);
    pathBuffer[pathLength] = 0;

    char* resKey = MSB_RES_IMAGE_KEY;

    uint32_t i = 0;
    while (i < pathLength && pathBuffer[i] != '\0')
    {
      if (resKey[i] == '\0')
        break;

      if (pathBuffer[i] != resKey[i])
      {
        i = 0;
        break;
      }

      i++;
    }

    if (i > 0)
    {
      char resPath[1024] = {0};
      sprintf(resPath, "%s%s", GAME_RESOURCE_PATH, pathBuffer + i);
      pathLength = strlen(resPath);
      LapisMemCopy(resPath, pathBuffer, pathLength);
    }

    pathBuffer[pathLength] = '\0';
    newImageInfo.imageInfo.imagePath = LapisMemAllocZeroArray(char, pathLength + 1);
    LapisMemCopy(pathBuffer, newImageInfo.imageInfo.imagePath, pathLength + 1);
  }
  else
  {
    OpalExtent extents;
    OpalFormat format;

    fread(&extents, sizeof(OpalExtent), 1, inFile);
    fread(&format, sizeof(OpalFormat), 1, inFile);

    uint32_t pixelCount = extents.width * extents.height * extents.depth;
    uint32_t imageSize = pixelCount * OpalFormatToSize(format);
    void* imageData = LapisMemAlloc(imageSize);
    fread(imageData, OpalFormatToSize(format), pixelCount, inFile);

    newImageInfo.imageInfo.extents = extents;
    newImageInfo.imageInfo.imageData = imageData;
  }

  MSB_ATTEMPT(MsInputSetAddArgument(set, newImageInfo));

  if (newImageInfo.imageInfo.imagePath != NULL)
  {
    LapisMemFree(newImageInfo.imageInfo.imagePath);
  }

  return Msb_Success;
}

MsbResult ReadInputSet(FILE* inFile, MsInputSet* set, MsbSerializeSaveFlags flags)
{
  uint32_t count;
  fread(&count, sizeof(uint32_t), 1, inFile);

  for (uint32_t i = 0; i < count; i++)
  {
    MsInputType type;
    fread(&type, sizeof(MsInputType), 1, inFile);

    switch (type)
    {
    case Ms_Input_Buffer: MSB_ATTEMPT(ReadBuffer(inFile, set, flags)); break;
    case Ms_Input_Image: MSB_ATTEMPT(ReadImage(inFile, set, flags)); break;
    }
  }

  return Msb_Success;
}

MsbResult ReadShaders(FILE* inFile)
{
  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    LapisMemFree(state.pShaderCodeInfos[i].buffer);
  }
  LapisMemFree(state.pShaderCodeInfos);

  fread(&state.shaderCount, sizeof(uint32_t), 1, inFile);
  state.pShaderCodeInfos = LapisMemAllocZeroArray(ShaderCodeInfo, state.shaderCount);

  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    ShaderCodeInfo* shader = &state.pShaderCodeInfos[i];
    fread(&shader->type, sizeof(OpalShaderType), 1, inFile);
    fread(&shader->size, sizeof(uint32_t), 1, inFile);
    shader->capacity = shader->size;
    shader->buffer = LapisMemAllocArray(char, shader->size + 1);
    shader->buffer[shader->size] = 0;
    fread(shader->buffer, sizeof(char), shader->size, inFile);
  }

  return Msb_Success;
}

MsbResult MsSerializeLoad(const char* path)
{
  OpalWaitIdle();

  FILE* inFile = fopen(path, "rb");

  uint32_t fileFingerprint;
  fread(&fileFingerprint, sizeof(uint32_t), 1, inFile);

  if ((fileFingerprint & MSB_SAVEFILE_FINGERPRINT_BITS) != MSB_SAVEFILE_FINGERPRINT)
  {
    fclose(inFile);
    MSB_ERR("Invalid file. Can not load '%s'\n", path);
    return Msb_Fail;
  }

  uint32_t fileVersion = fileFingerprint & ~MSB_SAVEFILE_FINGERPRINT_BITS;
  if (fileVersion != MSB_SAVEFILE_VERSION)
  {
    fclose(inFile);
    MSB_ERR("Mismatch serialization version (%u should be %u) for file '%s'\n", fileVersion, MSB_SAVEFILE_VERSION, path);
    return Msb_Fail;
  }


  MsbSerializeSaveFlags flags;
  fread(&flags, sizeof(MsbSerializeSaveFlags), 1, inFile);

  MSB_ATTEMPT(ReadShaders(inFile));
  MsInputSetShutdown(&state.materialInputSet);
  MSB_ATTEMPT(ReadInputSet(inFile, &state.materialInputSet, flags));

  fclose(inFile);

  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    if (MsCompileShader(&state.pShaderCodeInfos[i], state.pShaderCodeInfos[i].buffer) == Msb_Success)
    {
      MSB_ATTEMPT(MsUpdateShader(&state.pShaderCodeInfos[i]));
    }
  }

  MSB_ATTEMPT(MsUpdateMaterial());
  MSB_ATTEMPT(MsInputSetPushBuffers(&state.materialInputSet));

  state.serialLoadPath[0] = 0;

  return Msb_Success;
}
