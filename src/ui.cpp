
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
        ImGui::DragFloat(titleBuffer, &data->elements[(4 * column) + row], 0.01f);
        ImGui::NextColumn();
      }
    }
  ImGui::Columns(1);
}

MsResult RenderCode()
{
  static const uint32_t codeSize = 2048;
  static char vertCodeBuffer[codeSize] = MATSANDBOX_VERT_DEFAULT_SOURCE;
  static char fragCodeBuffer[codeSize] = MATSANDBOX_FRAG_DEFAULT_SOURCE;

  ImGui::Begin("Code");

  // =====
  // Vert
  // =====
  ImGui::Text("Vertex shader");
  if (ImGui::Button("Compile Vert"))
  {
    if (MsCompileShader(Opal_Shader_Vertex, vertCodeBuffer) == Ms_Success)
    {
      MS_ATTEMPT(MsUpdateShader(Opal_Shader_Vertex));
      MS_ATTEMPT(MsUpdateMaterial());
    }
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
    if (MsCompileShader(Opal_Shader_Fragment, fragCodeBuffer) == Ms_Success)
    {
      MS_ATTEMPT(MsUpdateShader(Opal_Shader_Fragment));
      MS_ATTEMPT(MsUpdateMaterial());
    }
  }
  ImGui::InputTextMultiline(
    "##fragSource",
    fragCodeBuffer,
    codeSize,
    { -FLT_MIN, ImGui::GetTextLineHeight() * 20 },
    ImGuiInputTextFlags_AllowTabInput);

  ImGui::End();

  return Ms_Success;
}

void RenderGlobalArguments()
{
  ImGui::Text("Camera view matrix");
  RenderImguiMatrix("Cam view matrix", &state.globalInputValues.camView);
  ImGui::Text("Camera projection matrix");
  RenderImguiMatrix("Cam proj matrix", &state.globalInputValues.camProj);
  ImGui::DragFloat3("Camera forward vector", state.globalInputValues.camForward.elements);
  state.globalInputValues.camForward = Vec3Normalize(state.globalInputValues.camForward);
}

void RenderBufferArgumentElements(MsInputArgumentBuffer* buffer)
{
  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    MsBufferElement* element = &buffer->pElements[i];
    switch (element->type)
    {
    case Ms_Buffer_Float:
    {
      ImGui::DragFloat(element->name, (float*)element->data, 0.01f);
    } break;
    case Ms_Buffer_Float2:
    {
      ImGui::DragFloat2(element->name, (float*)element->data, 0.01f);
    } break;
    case Ms_Buffer_Float3:
    {
      ImGui::DragFloat3(element->name, (float*)element->data, 0.01f);
    } break;
    case Ms_Buffer_Float4:
    {
      ImGui::DragFloat4(element->name, (float*)element->data, 0.01f);
    } break;
    case Ms_Buffer_Int:
    {
      ImGui::DragInt(element->name, (int*)element->data);
    } break;
    case Ms_Buffer_Int2:
    {
      ImGui::DragInt2(element->name, (int*)element->data);
    } break;
    case Ms_Buffer_Int3:
    {
      ImGui::DragInt3(element->name, (int*)element->data);
    } break;
    case Ms_Buffer_Int4:
    {
      ImGui::DragInt4(element->name, (int*)element->data);
    } break;
    case Ms_Buffer_Uint:
    {
      ImGui::DragScalar(element->name, ImGuiDataType_U32, element->data);
    } break;
    case Ms_Buffer_Uint2:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_U32, element->data, 2);
    } break;
    case Ms_Buffer_Uint3:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_U32, element->data, 3);
    } break;
    case Ms_Buffer_Uint4:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_U32, element->data, 4);
    } break;
    case Ms_Buffer_Double:
    {
      ImGui::DragScalar(element->name, ImGuiDataType_Double, element->data);
    } break;
    case Ms_Buffer_Double2:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_Double, element->data, 2);
    } break;
    case Ms_Buffer_Double3:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_Double, element->data, 3);
    } break;
    case Ms_Buffer_Double4:
    {
      ImGui::DragScalarN(element->name, ImGuiDataType_Double, element->data, 4);
    } break;
    case Ms_Buffer_Mat4:
    {
      RenderImguiMatrix(element->name, (Mat4*)element->data);
    } break;
    default: break;
    }
  }
}

void RenderBufferAddElement(uint32_t argIndex)
{
  MsInputArgumentBuffer* buffer = &state.materialInfo.pInputArguements[argIndex].data.buffer;

  if (ImGui::Button("Add"))
    ImGui::OpenPopup("BufferAddPopup");

  if (ImGui::BeginPopup("BufferAddPopup"))
  {
    if (ImGui::BeginMenu("Int"))
    {
      if (ImGui::MenuItem("Int"))
        MsBufferAddElement(buffer, Ms_Buffer_Int);
      if (ImGui::MenuItem("Int2"))
        MsBufferAddElement(buffer, Ms_Buffer_Int2);
      if (ImGui::MenuItem("Int3"))
        MsBufferAddElement(buffer, Ms_Buffer_Int3);
      if (ImGui::MenuItem("Int4"))
        MsBufferAddElement(buffer, Ms_Buffer_Int4);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Uint"))
    {
      if (ImGui::MenuItem("Uint"))
        MsBufferAddElement(buffer, Ms_Buffer_Uint);
      if (ImGui::MenuItem("Uint2"))
        MsBufferAddElement(buffer, Ms_Buffer_Uint2);
      if (ImGui::MenuItem("Uint3"))
        MsBufferAddElement(buffer, Ms_Buffer_Uint3);
      if (ImGui::MenuItem("Uint4"))
        MsBufferAddElement(buffer, Ms_Buffer_Uint4);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Float"))
    {
      if (ImGui::MenuItem("Float"))
        MsBufferAddElement(buffer, Ms_Buffer_Float);
      if (ImGui::MenuItem("Float2"))
        MsBufferAddElement(buffer, Ms_Buffer_Float2);
      if (ImGui::MenuItem("Float3"))
        MsBufferAddElement(buffer, Ms_Buffer_Float3);
      if (ImGui::MenuItem("Float4"))
        MsBufferAddElement(buffer, Ms_Buffer_Float4);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Double"))
    {
      if (ImGui::MenuItem("Double"))
        MsBufferAddElement(buffer, Ms_Buffer_Double);
      if (ImGui::MenuItem("Double2"))
        MsBufferAddElement(buffer, Ms_Buffer_Double2);
      if (ImGui::MenuItem("Double3"))
        MsBufferAddElement(buffer, Ms_Buffer_Double3);
      if (ImGui::MenuItem("Double4"))
        MsBufferAddElement(buffer, Ms_Buffer_Double4);
      ImGui::EndMenu();
    }

    if (ImGui::Selectable("Mat4"))
    {
      MsBufferAddElement(buffer, Ms_Buffer_Mat4);
    }

    ImGui::EndPopup();
  }
}

void RenderCustomArguments()
{
  MsInputArgument* argument = NULL;
  for (uint32_t i = 0; i < state.materialInfo.inputArgumentCount; i++)
  {
    argument = state.materialInfo.pInputArguements + i;
    if (argument->type == Ms_Input_Buffer)
    {
      ImGui::SeparatorText("Test buffer");
      ImGui::SameLine();
      RenderBufferAddElement(i);
      RenderBufferArgumentElements(&argument->data.buffer);
    }
    else
    {
      // Do image stuff here
      ImGui::Text("This should be an image");
    }
  }
}

MsResult RenderArguments()
{
  ImGui::Begin("Arguments");

  if (ImGui::CollapsingHeader("Global", ImGuiTreeNodeFlags_DefaultOpen))
  {
    RenderGlobalArguments();
  }

  if (ImGui::CollapsingHeader("Custom", ImGuiTreeNodeFlags_DefaultOpen))
  {
    RenderCustomArguments();
  }

  ImGui::End();
  return Ms_Success;
}

MsResult RenderPreview()
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

MsResult RenderImguiUi()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  MS_ATTEMPT(RenderCode());
  MS_ATTEMPT(RenderArguments());
  MS_ATTEMPT(RenderPreview());

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
