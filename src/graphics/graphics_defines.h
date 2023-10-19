
#ifndef MATSANDBOX_GRAPHICS_DEFINES_H
#define MATSANDBOX_GRAPHICS_DEFINES_H

#include "src/defines.h"
#include "src/graphics/graphics_define_types.h"

#include <stdint.h>
#include <windows.h>
#include <windowsx.h>
#include <vulkan/vulkan.h>

#include <vector>

struct MsgGpuInfo
{
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
  uint32_t queueIndexPresent;
};

struct MsgVertexLayout
{
  uint32_t elementCount;
  MsgFormat formats[8]; // Max 8 inputs per vertex
};

struct MsgWindow
{
  bool shouldClose;
  HWND hwnd;
  HINSTANCE hinstance;

  uint32_t windowStyle;
  uint32_t windowExStyle;
};

struct MsgState
{
  bool useDebug;

  MsgWindow window;

  const VkAllocationCallbacks* allocator;

  VkInstance instance;
  VkPhysicalDevice gpu;
  MsgGpuInfo gpuInfo;
  VkDevice device;
  VkSurfaceKHR surface;

  VkQueue queueGraphics;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool cmdPoolTransient;
  VkCommandPool cmdPoolGraphics;

  VkDescriptorPool descriptorPool;
};


#endif // !MATSANDBOX_GRAPHICS_DEFINES_H
