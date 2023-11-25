
#include "src/common.h"

void MsShutdown()
{
  MsInputSetShutdown(&state.materialInputSet);

  OpalImageShutdown(&state.depthImage);
  OpalShaderShutdown(&state.pShaders[0]);
  OpalShaderShutdown(&state.pShaders[1]);
  OpalMaterialShutdown(&state.material);
  OpalFramebufferShutdown(&state.uiFramebuffer);
  OpalRenderpassShutdown(&state.uiRenderpass);
  OpalFramebufferShutdown(&state.sceneFramebuffer);
  OpalRenderpassShutdown(&state.sceneRenderpass);

  OpalMeshShutdown(&state.meshes[0]);
  OpalMeshShutdown(&state.meshes[1]);
  OpalMeshShutdown(&state.meshes[2]);
  OpalMeshShutdown(&state.meshes[3]);

  OpalShutdown();
  LapisWindowShutdown(&state.window.lapis);
}

void MsInputSetShutdown(MsInputSet* set)
{
  for (uint32_t i = 0; i < set->count; i++)
  {
    switch (set->pArguments[i].type)
    {
    case Ms_Input_Buffer:
    {
      MsInputArgumentBuffer* buffer = &set->pArguments[i].data.buffer;
      for (uint32_t e = 0; e < buffer->elementCount; e++)
      {
        LapisMemFree(buffer->pElements[e].data);
      }

      LapisMemFree(buffer->pElements);
      buffer->elementCount = 0;

      OpalBufferShutdown(&buffer->buffer);
    } break;
    case Ms_Input_Image:
    {
      OpalImageShutdown(&set->pArguments[i].data.image.image);
    } break;
    default: break;
    }
  }
  LapisMemFree(set->pArguments);

  set->count = 0;
  OpalInputLayoutShutdown(&set->layout);
  OpalInputSetShutdown(&set->set);
  set->nextId = 0;
}
