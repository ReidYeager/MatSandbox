
#include "src/common.h"

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

  MSB_ATTEMPT(ShowSaveLoadModals());

  ImGui::EndMainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  MSB_ATTEMPT(ShowPreview());
  MSB_ATTEMPT(ShowCode());
  MSB_ATTEMPT(MsUiShowArgumentsPanel());


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
  static char pathBuffer[1024];
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

// =====
// Preview
// =====

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

// =====
// Code
// =====

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
