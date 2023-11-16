
#include "src/common.h"

void ShowInputSetArguments(MsInputSet* set, const char* title);
void ShowArgumentBuffer(MsInputArgumentBuffer* buffer);
void ShowBufferElement(MsBufferElement* element);
MsBufferElementType ShowBufferAddElementMenu();
void ShowArgumentImage(MsInputArgumentImage* image);

MsResult MsUiShowArgumentsPanel()
{
  ImGui::Begin("Arguments", NULL, ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("Add custom argument"))
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

      if (ImGui::MenuItem("Image"))
      {
        state.uiImageImportOrReload = 1;
      }

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  ShowInputSetArguments(&state.globalInputSet, "Global set");
  ShowInputSetArguments(&state.materialInputSet, "Custom set");

  static char imagePathBuffer[1024] = { 0 };
  if (state.uiImageImportOrReload)
  {
    printf("Should show modal \n");
    ImGui::OpenPopup("Import image##AddImageModal");
    //LapisMemSet(imagePathBuffer, 0, 1024);
  }

  if (ImGui::BeginPopupModal("Import image##AddImageModal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::InputText("Image path", imagePathBuffer, 1024);

    if (ImGui::Button("Done", ImVec2(120, 0)))
    {
      state.uiImageImportOrReload |= 4;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (state.uiImageImportOrReload & 4)
  {
    if ((state.uiImageImportOrReload & 3) == 1)
    {
      // load
      MsInputArgumentInitInfo argumentInfo;
      argumentInfo.type = Ms_Input_Image;
      argumentInfo.imageInfo.imagePath = imagePathBuffer;
      MsInputSetAddArgument(&state.materialInputSet, argumentInfo);
    }
    else if ((state.uiImageImportOrReload & 3) == 2)
    {
      // reimport
    }

    state.uiImageImportOrReload = 0;
  }

  ImGui::End();

  return Ms_Success;
}

void ShowInputSetArguments(MsInputSet* set, const char* title)
{
  if (!ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen))
    return;

  ImGui::PushID(set->pArguments);

  MsInputArgument* argument;
  for (uint32_t i = 0; i < set->count; i++)
  {
    ImGui::PushID(i);
    argument = &set->pArguments[i];

    ImGui::PushID(argument->id);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild(argument->name, ImVec2(0, 200), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      if (argument->type == Ms_Input_Buffer)
      {
        ImGui::MenuItem(argument->name, NULL, false, false);

        if (ImGui::BeginMenu("Add Element"))
        {
          MsBufferElementType newType = ShowBufferAddElementMenu();
          if (newType != Ms_Buffer_COUNT)
          {
            MsBufferAddElement(argument, newType);
          }

          ImGui::EndMenu();
        }
      }
      else // if (argument->type == Ms_Input_Image)
      {
        if (ImGui::MenuItem("Re-import"))
        {
          state.uiImageImportOrReload = 2;
        }
      }

      if (ImGui::MenuItem("Close"))
      {
        printf ("Remove argument %d", i);
      }
      ImGui::EndMenuBar();
    }

    // TODO Show some drag&drop element and handle that

    if (argument->type == Ms_Input_Buffer)
      ShowArgumentBuffer(&argument->data.buffer);
    else if (argument->type == Ms_Input_Image)
      ShowArgumentImage(&argument->data.image);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopID();

    ImGui::PopID();
  }

  ImGui::PopID();
}

void ShowArgumentBuffer(MsInputArgumentBuffer* buffer)
{
  MsBufferElement* element;
  for (uint32_t i = 0; i < buffer->elementCount; i++)
  {
    ImGui::PushID(i);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("element", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      ImGui::MenuItem(buffer->pElements[i].name, NULL, false, false);

      if (ImGui::MenuItem("Close"))
      {
        printf("Remove element %d", i);
      }
      ImGui::EndMenuBar();
    }

    ShowBufferElement(&buffer->pElements[i]);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopID();
  }
}

void ShowBufferElement(MsBufferElement* element)
{
  ImGui::SetNextItemWidth(-FLT_MIN);

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

        sprintf(titleBuffer, "##%s-%c%c", element->name, rowChar, colChar);

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

void ShowArgumentImage(MsInputArgumentImage* image)
{
  ImGui::Image(image->set->vk.descriptorSet, { 64.0f, 64.0f });
}
