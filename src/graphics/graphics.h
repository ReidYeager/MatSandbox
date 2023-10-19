
#ifndef MATSANDBOX_GRAPHICS_H
#define MATSANDBOX_GRAPHICS_H

#include "src/defines.h"
#include "src/graphics/graphics_defines.h"

#include <vulkan/vulkan.h>

#define MSG_ATTEMPT_VK(fn)   \
{                            \
  VkResult result = (fn);    \
  if (result != VK_SUCCESS)  \
  {                          \
    return MS_Fail_External; \
  }                          \
}

extern MsgState gState;

MsResult MsgInit(bool debug);
void MsgShutdown();

#endif // !MATSANDBOX_GRAPHICS_H
