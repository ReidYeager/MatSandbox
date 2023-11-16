
#include "src/common.h"

MsResult ShowPreview();
MsResult ShowCode();
MsResult ShowArguments();

MsResult RenderUi()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

  MS_ATTEMPT(ShowPreview());
  MS_ATTEMPT(ShowCode());
  MS_ATTEMPT(ShowArguments());

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

// =====
// Arguments
// =====

void RenderGlobalArguments();
void RenderAddArgument();
void RenderCustomArguments();

MsResult ShowArguments()
{
  ImGui::Begin("Arguments");

  if (ImGui::CollapsingHeader("Global", ImGuiTreeNodeFlags_DefaultOpen))
  {
    RenderGlobalArguments();
  }

  if (ImGui::CollapsingHeader("Custom", ImGuiTreeNodeFlags_DefaultOpen))
  {
    RenderAddArgument();
    RenderCustomArguments();
  }

  ImGui::End();
  return Ms_Success;
}


void RenderImguiMatrix(const char* title, Mat4* data)
{
  static char titleBuffer[128];

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

      sprintf(titleBuffer, "##%s-%c%c", title, rowChar, colChar);

      ImGui::DragFloat(titleBuffer, &data->elements[(4 * row) + column], 0.01f);

      ImGui::PopID();
    }

    ImGui::NextColumn();
  }
  ImGui::Columns(1);
}

void RenderGlobalArguments()
{
  ImGui::Text("Camera view matrix");
  RenderImguiMatrix("Cam view matrix", state.globalInputValues.camView);
  ImGui::Text("Camera projection matrix");
  RenderImguiMatrix("Cam proj matrix", state.globalInputValues.camProj);
  ImGui::DragFloat3("Camera forward vector", state.globalInputValues.camForward->elements);
  *state.globalInputValues.camForward = Vec3Normalize(*state.globalInputValues.camForward);
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

MsBufferElementType ShowBufferAddElementMenu()
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

void RenderBufferAddElement(uint32_t argIndex)
{
  ImGui::PushID(argIndex);

  if (ImGui::Button("Add element"))
    ImGui::OpenPopup("BufferElementAddPopup");

  if (ImGui::BeginPopup("BufferElementAddPopup"))
  {
    MsBufferElementType newType = ShowBufferAddElementMenu();

    if (newType != Ms_Buffer_COUNT)
    {
      MsBufferAddElement(&state.materialInputSet.pArguments[argIndex], newType);
      MsInputSetPushBuffers(&state.materialInputSet);
    }

    ImGui::EndPopup();
  }

  ImGui::PopID();
}

void RenderCustomArguments()
{
  MsInputArgument* argument = NULL;
  static char nameBuffer[128];

  for (uint32_t i = 0; i < state.materialInputSet.count; i++)
  {
    argument = state.materialInputSet.pArguments + i;
    sprintf(nameBuffer, "%u : %s", i, argument->name);

    if (argument->type == Ms_Input_Buffer)
    {
      ImGui::SeparatorText(nameBuffer);
      RenderBufferAddElement(i);
      RenderBufferArgumentElements(&argument->data.buffer);
    }
    else
    {
      ImGui::Image(argument->data.image.set->vk.descriptorSet, { 64.0f, 64.0f });
      ImGui::SameLine();
      sprintf(nameBuffer, "%u : %s", i, argument->name);
      ImGui::Text("%s", nameBuffer);
    }
  }
}

void RenderAddArgument()
{
  if (ImGui::Button("Add argument"))
    ImGui::OpenPopup("ArgumentAddPopup");

  if (ImGui::BeginPopup("ArgumentAddPopup"))
  {
    if (ImGui::BeginMenu("Buffer"))
    {
      MsBufferElementType firstType = ShowBufferAddElementMenu();
      if (firstType != Ms_Buffer_COUNT)
      {
        MsInputArgumentInitInfo argumentInfo;
        argumentInfo.type = Ms_Input_Buffer;
        argumentInfo.bufferInfo.elementCount = 1;
        argumentInfo.bufferInfo.pElementTypes = &firstType;
        MsInputSetAddArgument(&state.materialInputSet, argumentInfo);
      }

      ImGui::EndMenu();
    }

    if (ImGui::Selectable("Image"))
    {
      printf("This button doesn't work. Figure out why\n");
      ImGui::OpenPopup("AddImageModal");
    }

    ImGui::EndPopup();
  }

  if (ImGui::Button("Temporary Add image"))
    ImGui::OpenPopup("AddImageModal");

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("AddImageModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    static char pathBuffer[1024] = { 0 };
    ImGui::InputText("Image path", pathBuffer, 1024);

    if (ImGui::Button("Done", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();

      MsInputArgumentInitInfo argumentInfo;
      argumentInfo.type = Ms_Input_Image;
      argumentInfo.imageInfo.imagePath = pathBuffer;
      MsInputSetAddArgument(&state.materialInputSet, argumentInfo);
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

}
