
#include "src/common.h"

MsResult ShowPreview();
MsResult ShowCode();

MsResult RenderUi()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  MS_ATTEMPT(ShowPreview());
  MS_ATTEMPT(ShowCode());
  MS_ATTEMPT(MsUiShowArgumentsPanel());

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

MsResult ShowCodeBlock(const char* title, OpalShaderType type, char* codeBuffer);

MsResult ShowCode()
{
  static char vertCodeBuffer[MS_CODE_MAX_LENGTH] = MATSANDBOX_VERT_DEFAULT_SOURCE;
  static char fragCodeBuffer[MS_CODE_MAX_LENGTH] = MATSANDBOX_FRAG_DEFAULT_SOURCE;

  ImGui::Begin("Code");

  MS_ATTEMPT(ShowCodeBlock("Vertex", Opal_Shader_Vertex, vertCodeBuffer));
  ImGui::Spacing();
  MS_ATTEMPT(ShowCodeBlock("Fragment", Opal_Shader_Fragment, fragCodeBuffer));

  ImGui::End();

  return Ms_Success;
}

MsResult ShowCodeBlock(const char* title, OpalShaderType type, char* codeBuffer)
{
  ImGui::PushID(title);

  if (ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen))
  {
    if (ImGui::Button("Compile"))
    {
      if (MsCompileShader(type, codeBuffer) == Ms_Success)
      {
        MS_ATTEMPT(MsUpdateShader(type));
        MS_ATTEMPT(MsUpdateMaterial());
        MS_ATTEMPT(MsInputSetPushBuffers(&state.materialInputSet));
      }
    }

    ImGui::InputTextMultiline(
      "##source",
      codeBuffer,
      MS_CODE_MAX_LENGTH,
      { -FLT_MIN, ImGui::GetTextLineHeight() * 20 },
      ImGuiInputTextFlags_AllowTabInput);
  }

  ImGui::PopID();
  return Ms_Success;
}
