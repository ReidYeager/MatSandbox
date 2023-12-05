
#include "src/common.h"

#include "src/definesNew.h"
#include "src/ui.h"

MsbResult ShowPreview();
MsbResult ShowCode();
MsbResult ShowSaveLoadModals();

MsbResult RenderUi()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::BeginMainMenuBar();
  if (ImGui::MenuItem("Save"))
  {
    ImGui::OpenPopup("Save path##SavePathModal");
  }

  if (ImGui::MenuItem("Load"))
  {
    ImGui::OpenPopup("Load path##LoadPathModal");
  }

  ShowSaveLoadModals();

  ImGui::EndMainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  MSB_ATTEMPT(ShowPreview());
  MSB_ATTEMPT(ShowCode());
  MSB_ATTEMPT(MsUiShowArgumentsPanel());

  //ImGui::ShowDemoWindow();

  ImGui::EndFrame();

  ImGuiIO& io = ImGui::GetIO();
  if (!state.inputState.previewFocused && state.inputState.previewHovered && io.MouseClicked[1])
  {
    ImGui::SetWindowFocus("Preview");
  }

  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

  return Msb_Success;
}

MsbResult ShowSaveLoadModals()
{
  static char pathBuffer[1024] = "D:/Dev/MatSandbox/testout.dat";
  static bool embedImages = false;

  if (ImGui::BeginPopupModal("Save path##SavePathModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::InputText("Save path", pathBuffer, 1024);
    ImGui::Checkbox("Embed images", &embedImages);

    if (ImGui::Button("Done", ImVec2(120, 0)))
    {
      MSB_ATTEMPT(MsSerializeSave(pathBuffer, embedImages));
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupModal("Load path##LoadPathModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::InputText("Load path", pathBuffer, 1024);

    if (ImGui::Button("Open", ImVec2(120, 0)))
    {
      strcpy(state.serialLoadPath, pathBuffer);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  return Msb_Success;
}

// ===============
// Preview
// ===============

MsbResult ShowPreview()
{
  ImVec2 sceneExtents = { (float)state.sceneImage->extents.width, (float)state.sceneImage->extents.height };
  ImGui::Begin("Preview");
  ImGui::Image(state.uiSceneImageInputSet->vk.descriptorSet, sceneExtents);
  ImVec2 min = ImGui::GetWindowContentRegionMin();
  ImVec2 max = ImGui::GetWindowContentRegionMax();
  state.sceneGuiExtentsPrevFrame = { (int32_t)(max.x - min.x), (int32_t)(max.y - min.y) };

  state.inputState.previewFocused = ImGui::IsWindowFocused();
  state.inputState.previewHovered = ImGui::IsWindowHovered();

  ImGui::End();

  return Msb_Success;
}

// ===============
// Code
// ===============

MsbResult ShowCodeBlock(ShaderCodeInfo* codeInfo);

MsbResult ShowCode()
{
  ImGui::Begin("Code");

  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    if (i > 0)
      ImGui::Spacing();

    MSB_ATTEMPT(ShowCodeBlock(&state.pShaderCodeInfos[i]));
  }

  ImGui::End();

  return Msb_Success;
}

int TextBoxResizeCallback(ImGuiInputTextCallbackData* callbackData)
{
  OpalShaderType type = *(OpalShaderType*)callbackData->UserData;
  ShaderCodeInfo* codeInfo = &state.pShaderCodeInfos[type];

  codeInfo->size = callbackData->BufTextLen;

  if (callbackData->EventFlag == ImGuiInputTextFlags_CallbackResize)
  {
    if (codeInfo->size >= codeInfo->capacity)
    {
      while (codeInfo->size >= codeInfo->capacity)
      {
        codeInfo->capacity = codeInfo->capacity * 1.2;
      }

      codeInfo->buffer = (char*)LapisMemRealloc((void*)(codeInfo->buffer), (uint64_t)codeInfo->capacity);
    }

  }

  return 0;
}

static const char* shaderTypeNames[2] = {
  "Vertex",
  "Fragment"
};

void MsShaderAddToCompileQueue(ShaderCodeInfo* codeInfo)
{
  if (state.shaderCompileQueueLength >= Opal_Shader_COUNT)
    return;

  uint32_t index = state.shaderCompileQueueLength;
  state.shaderCompileQueueLength++;

  state.pShaderCompileQueue[index] = codeInfo;
}

MsbResult ShowCodeBlock(ShaderCodeInfo* codeInfo)
{
  const char* title = shaderTypeNames[codeInfo->type];

  ImGui::PushID(title);

  if (ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen))
  {
    ImGui::BeginChild(title, { -FLT_MIN, 0 }, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      if (ImGui::MenuItem("Compile"))
      {
        MsShaderAddToCompileQueue(codeInfo);
      }

      ImGui::EndMenuBar();
    }

    ImGui::InputTextMultiline(
      "##source",
      codeInfo->buffer,
      codeInfo->size + 1,
      { -FLT_MIN, -FLT_MIN },
      ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize,
      TextBoxResizeCallback,
      &codeInfo->type);

    ImGui::EndChild();
  }

  ImGui::PopID();
  return Msb_Success;
}

// ===============
// New ui
// ===============

void ImguiVkResultCheck(VkResult error) {}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void LapisInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

MsBufferElementType MsbShowBufferAddElementMenu();
void MsbShowBufferElement(MsbBufferElement* element);

MsbResult MsbUi::Init(MsbUiInitInfo initInfo)
{
  resources = initInfo.resources;
  inSets = initInfo.inSets;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();

  if (!ImGui_ImplWin32_Init(initInfo.hwnd))
  {
    MSB_ERR("Failed to initialize Imgui\n");
    return Msb_Fail;
  }

  ImGui_ImplVulkan_InitInfo imguiVulkanInfo = { 0 };
  imguiVulkanInfo.Allocator = NULL;
  imguiVulkanInfo.Instance = initInfo.vk.instance;
  imguiVulkanInfo.Device = initInfo.vk.device;
  imguiVulkanInfo.PhysicalDevice = initInfo.vk.physcialDevice;
  imguiVulkanInfo.QueueFamily = initInfo.vk.queueFamily;
  imguiVulkanInfo.Queue = initInfo.vk.queue;
  imguiVulkanInfo.PipelineCache = VK_NULL_HANDLE;
  imguiVulkanInfo.DescriptorPool = initInfo.vk.descriptorPool;
  imguiVulkanInfo.Subpass = 0;
  imguiVulkanInfo.MinImageCount = 2;
  imguiVulkanInfo.ImageCount = initInfo.vk.imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  if (!ImGui_ImplVulkan_Init(&imguiVulkanInfo, initInfo.vk.renderpass))
  {
    MSB_ERR("Failed to initialize Imgui vulkan\n");
    return Msb_Fail;
  }

  VkCommandBuffer cmd;
  MSB_ATTEMPT_OPAL(OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd));
  ImGui_ImplVulkan_CreateFontsTexture();
  MSB_ATTEMPT_OPAL(OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd));

  MSB_LOG("Ui init complete\n");

  return Msb_Success;
}

MsbResult MsbUi::Render()
{
  MSB_ATTEMPT(StartFrame());

  MSB_ATTEMPT(BuildArguments());

  MSB_ATTEMPT(EndFrame());

  return Msb_Success;
}

MsbResult MsbUi::StartFrame()
{
  OpalRenderBeginRenderpass(resources->renderpass, resources->framebuffer);

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::BeginMainMenuBar();
  if (ImGui::MenuItem("Save"))
  {
    ImGui::OpenPopup("Save path##SavePathModal");
  }

  if (ImGui::MenuItem("Load"))
  {
    ImGui::OpenPopup("Load path##LoadPathModal");
  }

  //ShowSaveLoadModals();

  ImGui::EndMainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  return Msb_Success;
}

MsbResult MsbUi::EndFrame()
{
  ImGui::EndFrame();

  ImGuiIO& io = ImGui::GetIO();
  if (!state.inputState.previewFocused && state.inputState.previewHovered && io.MouseClicked[1])
  {
    ImGui::SetWindowFocus("Preview");
  }

  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

  OpalRenderEndRenderpass(resources->renderpass);

  return Msb_Success;
}

MsbResult MsbUi::BuildPreview()
{
  return Msb_Success;
}

MsbResult MsbUi::BuildCode()
{
  return Msb_Success;
}

MsbResult MsbUi::BuildArguments()
{
  // Intitialization
  // ===============
  ImGui::Begin("Arguments", NULL, ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("Add custom argument"))
    {
      if (ImGui::BeginMenu("Buffer"))
      {
        MsBufferElementType firstType = MsbShowBufferAddElementMenu();
        if (firstType != Ms_Buffer_COUNT)
        {
          // Add new buffer argumnet
          // Add `firstType` element to buffer
          MSB_LOG("Ui :: Add new buffer\n");
        }
        ImGui::EndMenu();
      }

      if (ImGui::MenuItem("Image"))
      {
        MSB_LOG("Ui :: Add new image\n");
      }

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Input sets
  // ===============
  uint32_t setCount = inSets.size();
  for (uint32_t i = 0; i < setCount; i++)
  {
    MsbInputSet* set = inSets[i];
    if (!ImGui::CollapsingHeader(set->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
      continue;

    ImGui::PushID(set->GetSet()->vk.descriptorSet);

    uint32_t argCount = set->GetArgumentCount();
    for (uint32_t j = 0; j < argCount; j++)
    {
      ImGui::PushID(j);

      MsbInputArgument* arg = set->GetArgument(j);
      BuildSingleArgument(arg, i > 0);

      ImGui::PopID();
    }

    ImGui::PopID();
  }

  // End
  // ===============
  ImGui::End();

  return Msb_Success;
}

MsbResult MsbUi::BuildSingleArgument(MsbInputArgument* arg, bool showMenuOptions)
{
  ImGui::PushID(arg->id);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

  ImGui::BeginChild(arg->name.c_str(), ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar())
  {
    ImGui::MenuItem(arg->name.c_str(), NULL, false, false);

    if (arg->type == Msb_Argument_Buffer)
    {
      if (showMenuOptions && ImGui::BeginMenu("Add Element"))
      {
        MsBufferElementType newType = MsbShowBufferAddElementMenu();
        if (newType != Ms_Buffer_COUNT)
        {
          MSB_LOG("Buffer add element\n");
          //MsBufferAddElement(argument, newType);
        }

        ImGui::EndMenu();
      }
    }
    else if (arg->type == Msb_Argument_Image)
    {
      if (ImGui::MenuItem("Re-import"))
      {
        MSB_LOG("Image reimport\n");
      }
    }

    if (showMenuOptions && ImGui::MenuItem("Remove"))
    {
      MSB_LOG("Remove argument from set\n");

      //ImGui::EndMenuBar();
      //ImGui::EndChild();
      //ImGui::PopStyleVar();
      //ImGui::PopID();
      //ImGui::PopID();
      //return Msb_Success;
    }
    ImGui::EndMenuBar();
  }

  switch (arg->type)
  {
  case Msb_Argument_Buffer: MSB_ATTEMPT(BuildArgumentBuffer(arg, showMenuOptions)); break;
  case Msb_Argument_Image: MSB_ATTEMPT(BuildArgumentImage(arg)); break;
  default: break;
  }

  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::PopID();

  return Msb_Success;
}

MsbResult MsbUi::BuildArgumentBuffer(MsbInputArgument* arg, bool canEditElements)
{
  MsBufferElement* element;
  MsbInputArgumentBuffer* buffer = &arg->data.buffer;

  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    ImGui::PushID(i);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::BeginChild("element", ImVec2(-FLT_MIN, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      ImGui::MenuItem(buffer->pElements[i].name.c_str(), NULL, false, false);

      if (canEditElements && ImGui::MenuItem("Remove"))
      {
        MSB_LOG("Remove buffer element\n");
        //MsBufferRemoveElement(buffer, i);
        //i--;

        //ImGui::EndMenuBar();
        //ImGui::EndChild();
        //ImGui::PopStyleVar();
        //ImGui::PopID();
        //continue;
      }
      ImGui::EndMenuBar();
    }

    MsbShowBufferElement(&buffer->pElements[i]);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopID();
  }

  return Msb_Success;
}

MsbResult MsbUi::BuildArgumentImage(MsbInputArgument* arg)
{
  return Msb_Success;
}

MsbResult MsbUi::BuildFileModal()
{
  
  return Msb_Success;
}

MsBufferElementType MsbShowBufferAddElementMenu()
{
  MsBufferElementType newType = Ms_Buffer_COUNT;

  if (ImGui::BeginMenu("Int"))
  {
    if (ImGui::MenuItem("Int"))
      newType = Ms_Buffer_Int;
    if (ImGui::MenuItem("Int2"))
      newType = Ms_Buffer_Int2;
    if (ImGui::MenuItem("Int3"))
      newType = Ms_Buffer_Int3;
    if (ImGui::MenuItem("Int4"))
      newType = Ms_Buffer_Int4;
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Uint"))
  {
    if (ImGui::MenuItem("Uint"))
      newType = Ms_Buffer_Uint;
    if (ImGui::MenuItem("Uint2"))
      newType = Ms_Buffer_Uint2;
    if (ImGui::MenuItem("Uint3"))
      newType = Ms_Buffer_Uint3;
    if (ImGui::MenuItem("Uint4"))
      newType = Ms_Buffer_Uint4;
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Float"))
  {
    if (ImGui::MenuItem("Float"))
      newType = Ms_Buffer_Float;
    if (ImGui::MenuItem("Float2"))
      newType = Ms_Buffer_Float2;
    if (ImGui::MenuItem("Float3"))
      newType = Ms_Buffer_Float3;
    if (ImGui::MenuItem("Float4"))
      newType = Ms_Buffer_Float4;
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Double"))
  {
    if (ImGui::MenuItem("Double"))
      newType = Ms_Buffer_Double;
    if (ImGui::MenuItem("Double2"))
      newType = Ms_Buffer_Double2;
    if (ImGui::MenuItem("Double3"))
      newType = Ms_Buffer_Double3;
    if (ImGui::MenuItem("Double4"))
      newType = Ms_Buffer_Double4;
    ImGui::EndMenu();
  }

  if (ImGui::Selectable("Mat4"))
  {
    newType = Ms_Buffer_Mat4;
  }

  return newType;
}

void MsbShowBufferElement(MsbBufferElement* element)
{
  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::PushID(element->data);

  switch (element->type)
  {
  case Ms_Buffer_Int:     ImGui::DragInt    (element->name.c_str(), (int*  )element->data                 ); break;
  case Ms_Buffer_Int2:    ImGui::DragInt2   (element->name.c_str(), (int*  )element->data                 ); break;
  case Ms_Buffer_Int3:    ImGui::DragInt3   (element->name.c_str(), (int*  )element->data                 ); break;
  case Ms_Buffer_Int4:    ImGui::DragInt4   (element->name.c_str(), (int*  )element->data                 ); break;
  case Ms_Buffer_Uint:    ImGui::DragScalar (element->name.c_str(), ImGuiDataType_U32,    element->data   ); break;
  case Ms_Buffer_Uint2:   ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_U32,    element->data, 2); break;
  case Ms_Buffer_Uint3:   ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_U32,    element->data, 3); break;
  case Ms_Buffer_Uint4:   ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_U32,    element->data, 4); break;
  case Ms_Buffer_Float:   ImGui::DragFloat  (element->name.c_str(), (float*)element->data, 0.01f          ); break;
  case Ms_Buffer_Float2:  ImGui::DragFloat2 (element->name.c_str(), (float*)element->data, 0.01f          ); break;
  case Ms_Buffer_Float3:  ImGui::DragFloat3 (element->name.c_str(), (float*)element->data, 0.01f          ); break;
  case Ms_Buffer_Float4:  ImGui::DragFloat4 (element->name.c_str(), (float*)element->data, 0.01f          ); break;
  case Ms_Buffer_Double:  ImGui::DragScalar (element->name.c_str(), ImGuiDataType_Double, element->data   ); break;
  case Ms_Buffer_Double2: ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_Double, element->data, 2); break;
  case Ms_Buffer_Double3: ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_Double, element->data, 3); break;
  case Ms_Buffer_Double4: ImGui::DragScalarN(element->name.c_str(), ImGuiDataType_Double, element->data, 4); break;
  case Ms_Buffer_Mat4:
  {
    static char titleBuffer[128];
    Mat4* mat = (Mat4*)element->data;

    ImGui::Columns(4);
    for (uint32_t row = 0; row < 4; row++)
    {
      for (uint32_t column = 0; column < 4; column++)
      {
        ImGui::PushID((4 * row) + column);

        char colChar, rowChar;

        switch (column)
        {
        case 0: colChar = 'x'; break;
        case 1: colChar = 'y'; break;
        case 2: colChar = 'z'; break;
        case 3: colChar = 'w'; break;
        }

        switch (row)
        {
        case 0: rowChar = 'x'; break;
        case 1: rowChar = 'y'; break;
        case 2: rowChar = 'z'; break;
        case 3: rowChar = 'w'; break;
        }

        sprintf(titleBuffer, "##%s-%c%c", element->name.c_str(), rowChar, colChar);

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::DragFloat(titleBuffer, &mat->elements[(4 * row) + column], 0.01f);

        ImGui::PopID();
      }

      ImGui::NextColumn();
    }
    ImGui::Columns(1);
  } break;
  default: break;
  }

  ImGui::PopID();
}
