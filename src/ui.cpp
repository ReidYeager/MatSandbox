
#include "src/common.h"

MsResult ShowPreview();
MsResult ShowCode();
MsResult ShowSaveLoadModals();

MsResult RenderUi()
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

  MS_ATTEMPT(ShowPreview());
  MS_ATTEMPT(ShowCode());
  MS_ATTEMPT(MsUiShowArgumentsPanel());

  ImGui::ShowDemoWindow();

  ImGui::EndFrame();

  ImGuiIO& io = ImGui::GetIO();
  if (!state.inputState.previewFocused && state.inputState.previewHovered && io.MouseClicked[1])
  {
    ImGui::SetWindowFocus("Preview");
  }

  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

  return Ms_Success;
}

MsResult ShowSaveLoadModals()
{
  static char pathBuffer[1024];

  if (ImGui::BeginPopupModal("Save path##SavePathModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::InputText("Save path", pathBuffer, 1024);

    if (ImGui::Button("Done", ImVec2(120, 0)))
    {
      MS_ATTEMPT(MsSerializeSave(pathBuffer));
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
      MS_ATTEMPT(MsSerializeLoad(pathBuffer));
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  return Ms_Success;
}

// =====
// Preview
// =====

MsResult ShowPreview()
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

  return Ms_Success;
}

// =====
// Code
// =====

MsResult ShowCodeBlock(const char* title, OpalShaderType type);

MsResult ShowCode()
{
  ImGui::Begin("Code");

  MS_ATTEMPT(ShowCodeBlock("Vertex", Opal_Shader_Vertex));
  ImGui::Spacing();
  MS_ATTEMPT(ShowCodeBlock("Fragment", Opal_Shader_Fragment));

  ImGui::End();

  return Ms_Success;
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
      codeInfo->capacity = codeInfo->capacity * 1.2;
      codeInfo->buffer = (char*)LapisMemRealloc((void*)(codeInfo->buffer), (uint64_t)codeInfo->capacity);
    }

  }

  return 0;
}

MsResult ShowCodeBlock(const char* title, OpalShaderType type)
{
  ImGui::PushID(title);

  if (ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen))
  {
    ShaderCodeInfo* codeInfo = &state.pShaderCodeInfos[type];

    if (ImGui::Button("Compile"))
    {
      if (MsCompileShader(type, codeInfo->buffer) == Ms_Success)
      {
        MS_ATTEMPT(MsUpdateShader(type));
        MS_ATTEMPT(MsUpdateMaterial());
        MS_ATTEMPT(MsInputSetPushBuffers(&state.materialInputSet));
      }
    }

    ImGui::SameLine();
    ImGui::Text("%u/%u", codeInfo->size, codeInfo->capacity);

    ImGui::InputTextMultiline(
      "##source",
      codeInfo->buffer,
      codeInfo->size,
      { -FLT_MIN, ImGui::GetTextLineHeight() * 20 },
      ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize,
      TextBoxResizeCallback,
      &type);
  }

  ImGui::PopID();
  return Ms_Success;
}
