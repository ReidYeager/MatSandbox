
#include "graphics.h"

#include <peridot.h>
#include <windows.h>
#include <windowsx.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <vector>

MsgState gState;

LRESULT CALLBACK Win32MessageProcess(HWND _hwnd, uint32_t _message, WPARAM _wparam, LPARAM _lparam)
{
  LRESULT result = 0;

  switch (_message)
  {
  case WM_CLOSE:
  case WM_QUIT:
  {
    gState.window.shouldClose = true;
  } break;
  default: result = DefWindowProcA(_hwnd, _message, _wparam, _lparam);
  }

  return result;
}

MsResult WindowInit()
{
  bool isResizable = false;
  Vec2U extents = { 800, 600 };
  Vec2I position = { 100, 100 };
  const char* title = "Material Sandbox";
  const char* windowClassName = "MaterialSandboxWindowClass";

  gState.window.hinstance = GetModuleHandleA(0);

  WNDCLASSA wc = {
    /* style;         */ CS_DBLCLKS,
    /* lpfnWndProc;   */ Win32MessageProcess,
    /* cbClsExtra;    */ 0,
    /* cbWndExtra;    */ 0,
    /* hInstance;     */ gState.window.hinstance,
    /* hIcon;         */ LoadIcon(gState.window.hinstance, IDI_APPLICATION),
    /* hCursor;       */ LoadCursor(NULL, IDC_ARROW),
    /* hbrBackground; */ NULL,
    /* lpszMenuName;  */ NULL,
    /* lpszClassName; */ windowClassName
  };

  int x = RegisterClassA(&wc);

  if (x == 0)
  {
    return MS_Fail_External;
  }

  uint32_t resizability = (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX) * isResizable;

  uint32_t windowStyle = resizability | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  uint32_t windowExStyle = WS_EX_APPWINDOW;

  // Adjust extents so the canvas matches the input extents
  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);
  uint32_t adjustedWidth = extents.width + borderRect.right - borderRect.left;
  uint32_t adjustedHeight = extents.height + borderRect.bottom - borderRect.top;

  gState.window.hwnd = CreateWindowExA(
    /* dwExStyle    */ windowExStyle,
    /* lpClassName  */ windowClassName,
    /* lpWindowName */ title,
    /* dwStyle      */ windowStyle,
    /* X            */ position.x, // X screen position
    /* Y            */ position.y, // Y screen position
    /* nWidth       */ adjustedWidth,
    /* nHeight      */ adjustedHeight,
    /* hWndParent   */ 0,
    /* hMenu        */ 0,
    /* hInstance    */ gState.window.hinstance,
    /* lpParam      */ 0
  );

  if (gState.window.hwnd == 0)
  {
    printf("Failed to create window\n");
    return MS_Fail_External;
  }

  ShowWindow(gState.window.hwnd, SW_SHOW);
  gState.window.windowStyle = windowStyle;
  gState.window.windowExStyle = windowExStyle;

  return Ms_Success;
}

MsResult WindowUpdate()
{
  MSG message;

  while (PeekMessageA(&message, gState.window.hwnd, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return Ms_Success;
}

void WindowShutdown()
{
  DestroyWindow(gState.window.hwnd);
}

MsResult InstanceInit()
{
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.apiVersion = VK_API_VERSION_1_3;
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 1);
  appInfo.pEngineName = "MaterialSandboxEngine";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 1);
  appInfo.pApplicationName = "MaterialSandbox";

  uint32_t extensionCount = 2;
  const char* extensionNames[3];
  extensionNames[0] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
  extensionNames[1] = VK_KHR_SURFACE_EXTENSION_NAME;

  if (gState.useDebug)
  {
    extensionCount++;
    extensionNames[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  uint32_t layerCount = 0;
  const char* layerNames[1];

  if (gState.useDebug)
  {
    layerCount++;
    layerNames[0] = "VK_LAYER_KHRONOS_validation";
  }

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensionNames;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layerNames;

  MSG_ATTEMPT_VK(vkCreateInstance(&createInfo, gState.allocator, &gState.instance));

  return Ms_Success;
}

MsResult SurfaceInit()
{
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.hinstance = gState.window.hinstance;
  createInfo.hwnd = gState.window.hwnd;

  MSG_ATTEMPT_VK(vkCreateWin32SurfaceKHR(gState.instance, &createInfo, NULL, &gState.surface));

  return Ms_Success;
}

uint32_t GetFamilyIndexForQueue_Ovk(const MsgGpuInfo* const gpu, VkQueueFlags flags)
{
  uint32_t bestFit = ~0U;

  for (uint32_t i = 0; i < gpu->queueFamilyPropertiesCount; i++)
  {
    uint32_t queueFlag = gpu->queueFamilyProperties[i].queueFlags;

    if ((queueFlag & flags) == flags)
    {
      // Try to avoid choosing overlapping queue family indices
      if (flags & VK_QUEUE_GRAPHICS_BIT || i != gpu->queueIndexGraphics)
      {
        return i;
      }
      else
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0U)
  {
    printf("Failed to find a queue family with the flag %u\n", flags);
  }
  return bestFit;
}

uint32_t GetFamilyIndexForPresent_Ovk(VkPhysicalDevice gpu, const MsgGpuInfo* const info)
{
  VkBool32 canPresent = VK_FALSE;
  uint32_t bestFit = ~0u;

  for (uint32_t i = 0; i < info->queueFamilyPropertiesCount; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, gState.surface, &canPresent);

    if (canPresent == VK_TRUE)
    {
      if (i != info->queueIndexGraphics && i != info->queueIndexTransfer)
      {
        return i;
      }
      else if (i != info->queueIndexTransfer)
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0u)
  {
    printf("Failed to find a queue family for presentation\n");
  }

  return bestFit;
}

MsgGpuInfo CreateGpuInfo(VkPhysicalDevice device)
{
  MsgGpuInfo gpu = { 0 };

  vkGetPhysicalDeviceProperties(device, &gpu.properties);
  vkGetPhysicalDeviceFeatures(device, &gpu.features);
  vkGetPhysicalDeviceMemoryProperties(device, &gpu.memoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &gpu.queueFamilyPropertiesCount, NULL);
  gpu.queueFamilyProperties.resize(gpu.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &gpu.queueFamilyPropertiesCount, gpu.queueFamilyProperties.data());

  gpu.queueIndexGraphics = GetFamilyIndexForQueue_Ovk(&gpu, VK_QUEUE_GRAPHICS_BIT);
  gpu.queueIndexTransfer = GetFamilyIndexForQueue_Ovk(&gpu, VK_QUEUE_TRANSFER_BIT);
  gpu.queueIndexPresent = GetFamilyIndexForPresent_Ovk(device, &gpu);

  return gpu;
}

bool DetermineDeviceSuitability(VkPhysicalDevice device)
{
  MsgGpuInfo gpuInfo = CreateGpuInfo(device);
  bool isSuitable = true;

  isSuitable &= gpuInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  isSuitable &= gpuInfo.queueIndexPresent != ~0u;

  return isSuitable;
}

MsResult ChoosePhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(gState.instance, &deviceCount, NULL);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(gState.instance, &deviceCount, devices.data());

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    if (DetermineDeviceSuitability(devices[i]))
    {
      gState.gpu = devices[i];
      gState.gpuInfo = CreateGpuInfo(gState.gpu);

      return Ms_Success;
    }
  }

  return Ms_Fail;
}

MsResult DeviceInit()
{
  VkPhysicalDeviceFeatures enabledFeatures = { 0 };

  // Queues =====
  const float queuePriority = 1.0f;
  const uint32_t queueCount = 3;
  uint32_t queueIndices[queueCount] = {};
  queueIndices[0] = gState.gpuInfo.queueIndexGraphics;
  queueIndices[1] = gState.gpuInfo.queueIndexTransfer;
  queueIndices[2] = gState.gpuInfo.queueIndexPresent;
  VkDeviceQueueCreateInfo queueCreateInfos[queueCount] = {};

  for (uint32_t i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  // Extensions =====
  const uint32_t extensionCount = 1;
  const char* extensionNames[extensionCount] = {};
  extensionNames[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // Layers =====
  uint32_t layerCount = 0;
  const char* layers[1] = {};

  if (gState.useDebug)
  {
    layerCount++;
    layers[0] = "VK_LAYER_KHRONOS_validation";
  }

  // Creation =====
  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensionNames;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  MSG_ATTEMPT_VK(vkCreateDevice(gState.gpu, &createInfo, NULL, &gState.device));

  vkGetDeviceQueue(gState.device, gState.gpuInfo.queueIndexGraphics, 0, &gState.queueGraphics);
  vkGetDeviceQueue(gState.device, gState.gpuInfo.queueIndexTransfer, 0, &gState.queueTransfer);
  vkGetDeviceQueue(gState.device, gState.gpuInfo.queueIndexPresent, 0, &gState.queuePresent);

  return Ms_Success;
}

MsResult CommandPoolInit(bool isTransient)
{
  VkCommandPoolCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = NULL;

  VkCommandPool* outPool = NULL;

  if (isTransient)
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = gState.gpuInfo.queueIndexTransfer;
    outPool = &gState.cmdPoolTransient;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = gState.gpuInfo.queueIndexGraphics;
    outPool = &gState.cmdPoolGraphics;
  }

  MSG_ATTEMPT_VK(vkCreateCommandPool(gState.device, &createInfo, NULL, outPool));

  return Ms_Success;
}

MsResult DescriptorPoolInit()
{
  VkDescriptorPoolSize sizes[2] = {};
  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 1024;
  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 1024;

  VkDescriptorPoolCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1024;
  createInfo.poolSizeCount = 2;
  createInfo.pPoolSizes = sizes;

  MSG_ATTEMPT_VK(vkCreateDescriptorPool(gState.device, &createInfo, NULL, &gState.descriptorPool));

  return Ms_Success;
}

MsResult MsgInit(bool debug)
{
  gState.useDebug = debug;

  MS_ATTEMPT(WindowInit());
  MS_ATTEMPT(InstanceInit());
  MS_ATTEMPT(SurfaceInit());

  MS_ATTEMPT(ChoosePhysicalDevice());
  MS_ATTEMPT(DeviceInit());

  MS_ATTEMPT(CommandPoolInit(false));
  MS_ATTEMPT(CommandPoolInit(true));
  MS_ATTEMPT(DescriptorPoolInit());

  printf("Graphics initialized\n");


  while (!gState.window.shouldClose)
  {
    WindowUpdate();
  }


  return Ms_Success;
}

void MsgShutdown()
{
  vkDeviceWaitIdle(gState.device);
  vkDestroyDescriptorPool(gState.device, gState.descriptorPool, gState.allocator);
  vkDestroyCommandPool(gState.device, gState.cmdPoolGraphics, gState.allocator);
  vkDestroyCommandPool(gState.device, gState.cmdPoolTransient, gState.allocator);

  vkDestroyDevice(gState.device, gState.allocator);
  vkDestroySurfaceKHR(gState.instance, gState.surface, gState.allocator);
  vkDestroyInstance(gState.instance, gState.allocator);

  printf("Graphics shutdown\n");
}
