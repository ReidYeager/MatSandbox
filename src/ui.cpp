
#include "src/common.h"

MsResult RenderImguiUi();

MsResult RenderUi()
{
  return RenderImguiUi();
}

void RenderImguiMatrix(const char* title, Mat4* data)
{
  static char titleBuffer[128] = { 0 };

  ImGui::Columns(4);
  for (uint32_t row = 0; row < 4; row++)
    for (uint32_t column = 0; column < 4; column++)
    {
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

      sprintf(titleBuffer, "##%s-%c%c", title, rowChar, colChar);

      {
        ImGui::DragFloat(titleBuffer, &data->elements[(4 * column) + row], 0.1f);
        ImGui::NextColumn();
      }
    }
  ImGui::Columns(1);
}

MsResult RenderImguiUi()
{
  static const uint32_t codeSize = 2048;
  static char vertCodeBuffer[codeSize] = MATSANDBOX_VERT_DEFAULT_SOURCE;
  static char fragCodeBuffer[codeSize] = MATSANDBOX_FRAG_DEFAULT_SOURCE;

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // =====
  // Vert
  // =====
  ImGui::Begin("This is a test thing");
  ImGui::Text("Vertex shader");
  if (ImGui::Button("Compile Vert"))
  {
    MS_ATTEMPT(MsCompileShader(Opal_Shader_Vertex, vertCodeBuffer));
    MS_ATTEMPT(MsUpdateShader(Opal_Shader_Vertex));
    MS_ATTEMPT(MsUpdateMaterial());
  }
  ImGui::InputTextMultiline(
    "##vertSource",
    vertCodeBuffer,
    codeSize,
    { -FLT_MIN, ImGui::GetTextLineHeight() * 20 },
    ImGuiInputTextFlags_AllowTabInput);

  // =====
  // Frag
  // =====
  ImGui::Text("Fragment shader");
  if (ImGui::Button("Compile Frag"))
  {
    MS_ATTEMPT(MsCompileShader(Opal_Shader_Fragment, fragCodeBuffer));
    MS_ATTEMPT(MsUpdateShader(Opal_Shader_Fragment));
    MS_ATTEMPT(MsUpdateMaterial());
  }
  ImGui::InputTextMultiline(
    "##fragSource",
    fragCodeBuffer,
    codeSize,
    { -FLT_MIN, ImGui::GetTextLineHeight() * 20 },
    ImGuiInputTextFlags_AllowTabInput);

  ImGui::End();

  ImGui::Begin("Properties");

  ImGui::Text("Camera view matrix");
  RenderImguiMatrix("Cam view matrix", &state.globalInputValues.camView);
  ImGui::Text("Camera projection matrix");
  RenderImguiMatrix("Cam proj matrix", &state.globalInputValues.camProj);
  ImGui::Text("Camera viewProjection matrix");
  RenderImguiMatrix("Cam viewproj matrix", &state.globalInputValues.camViewProj);
  ImGui::DragFloat3("Camera forward vector", state.globalInputValues.camForward.elements);
  state.globalInputValues.camForward = Vec3Normalize(state.globalInputValues.camForward);

  ImGui::End();

  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

  return Ms_Success;
}
