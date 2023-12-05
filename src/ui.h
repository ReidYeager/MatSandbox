
#ifndef MAT_SANDBOX_UI_H
#define MAT_SANDBOX_UI_H

#include "src/definesNew.h"
#include "src/input_set.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>

#include <vulkan/vulkan.h>

struct MsbUiInitInfo
{
  HWND hwnd;
  MsbUiRenderResources* resources;

  std::vector<MsbInputSet*> inSets;

  struct
  {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physcialDevice;
    uint32_t queueFamily;
    VkQueue queue;
    VkDescriptorPool descriptorPool;
    uint32_t imageCount;
    VkRenderPass renderpass;
  } vk;
};

class MsbUi
{
private:
  MsbUiRenderResources* resources;
  std::vector<MsbInputSet*> inSets;

  bool shouldShowFileModal = false;

  enum FileModalOkFunctions
  {
    Ui_File_Modal_Load_Image,
    Ui_File_Modal_Reload_Image,
    Ui_File_Modal_Load_Serialized,
    Ui_File_Modal_Save_Serialized
  };

  struct
  {
    std::string title;
    std::string path;
    FileModalOkFunctions okFunction;
  } fileModalSettings;

public:
  MsbResult Init(MsbUiInitInfo initInfo);
  MsbResult Render();

private:
  MsbResult StartFrame();
  MsbResult BuildPreview();
  MsbResult BuildCode();
  MsbResult BuildArguments();
  MsbResult EndFrame();

  MsbResult BuildSingleArgument(MsbInputArgument* arg, bool showBufferOptions);
  MsbResult BuildArgumentBuffer(MsbInputArgument* arg, bool canEditElements);
  MsbResult BuildArgumentImage(MsbInputArgument* arg);

  MsbResult BuildFileModal();

};

#endif // !MAT_SANDBOX_UI_H
