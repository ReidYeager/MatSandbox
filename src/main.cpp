
#include "src/common.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"

#include <lapis.h>
#include <opal.h>
#include <peridot.h>

#include <stdio.h>
#include <string>
#include <memory>

MatSandboxState state;

void ImguiVkResultCheck(VkResult error) {}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void HandleInput();

void LapisInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

MsResult InitOpalBoilerplate()
{
  LapisWindowInitInfo lapisWindowInfo = {};
  lapisWindowInfo.fnPlatformInputCallback = LapisInputCallback;
  lapisWindowInfo.extents = { 1280, 720 };
  lapisWindowInfo.position = { 100, 100 };
  lapisWindowInfo.title = "Material with lapis";
  lapisWindowInfo.resizable = false;
  LapisWindowInit(lapisWindowInfo, &state.window.lapis);

  LapisWindowPlatformData windowPlatformData = LapisWindowGetPlatformData(state.window.lapis);

  OpalInitInfo opalInfo = { 0 };
  opalInfo.windowPlatformInfo.hinstance = windowPlatformData.hinstance;
  opalInfo.windowPlatformInfo.hwnd = windowPlatformData.hwnd;
  opalInfo.debug = false;
  OpalFormat vertexInputFormats[3] = { Opal_Format_RGB32, Opal_Format_RGB32, Opal_Format_RG32 };
  opalInfo.vertexStruct.count = 3;
  opalInfo.vertexStruct.pFormats = vertexInputFormats;

  MS_ATTEMPT_OPAL(OpalInit(opalInfo));
  
  OpalWindowInitInfo owInfo = { 0 };
  owInfo.platformInfo.hinstance = windowPlatformData.hinstance;
  owInfo.platformInfo.hwnd = windowPlatformData.hwnd;
  owInfo.extents = lapisWindowInfo.extents;
  MS_ATTEMPT_OPAL(OpalWindowInit(&state.window.opal, owInfo));

  OpalWindowGetBufferImage(state.window.opal, &state.renderBufferImage);

  OpalImageInitInfo dimInfo = { 0 };
  dimInfo.extent = { 1280, 720, 1 };
  dimInfo.format = Opal_Format_D24_S8;
  dimInfo.sampleType = Opal_Sample_Bilinear;
  dimInfo.usage = Opal_Image_Usage_Depth;
  MS_ATTEMPT_OPAL(OpalImageInit(&state.depthImage, dimInfo));

  OpalAttachmentInfo attachments[2] = { 0 };
  attachments[0].clearValue.depthStencil = (OpalDepthStencilValue){ 1, 0 };
  attachments[0].format = Opal_Format_D24_S8;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = false;
  attachments[0].usage = Opal_Attachment_Usage_Depth;

  attachments[1].clearValue.color = (OpalColorValue){ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[1].format = Opal_Format_BGRA8;
  attachments[1].loadOp = Opal_Attachment_Op_Clear;
  attachments[1].shouldStore = true;
  attachments[1].usage = Opal_Attachment_Usage_Presented;

  uint32_t colorIndices[1] = { 1 };
  OpalSubpassInfo subpass = { 0 };
  subpass.depthAttachmentIndex = 0;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = colorIndices;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = NULL;

  OpalRenderpassInitInfo rpInfo = { 0 };
  rpInfo.dependencyCount = 0;
  rpInfo.pDependencies = NULL;
  rpInfo.imageCount = 2;
  rpInfo.pAttachments = attachments;
  rpInfo.subpassCount = 1;
  rpInfo.pSubpasses = &subpass;
  MS_ATTEMPT_OPAL(OpalRenderpassInit(&state.renderpass, rpInfo));

  OpalImage framebufferImages[2] = { state.depthImage, state.renderBufferImage };
  OpalFramebufferInitInfo fbInfo = { 0 };
  fbInfo.imageCount = 2;
  fbInfo.pImages = framebufferImages;
  fbInfo.renderpass = state.renderpass;
  MS_ATTEMPT_OPAL(OpalFramebufferInit(&state.framebuffer, fbInfo));

  return Ms_Success;
}

MsResult InitMeshes()
{
  MS_ATTEMPT(LoadMesh("C:/dev/MatSandbox/res/models/Sphere.obj", &state.meshes[0]));
  MS_ATTEMPT(LoadMesh("C:/dev/MatSandbox/res/models/SphereSmooth.obj", &state.meshes[1]));
  MS_ATTEMPT(LoadMesh("C:/dev/MatSandbox/res/models/Cube.obj", &state.meshes[2]));
  MS_ATTEMPT(LoadMesh("C:/dev/MatSandbox/res/models/Cyborg_Weapon.obj", &state.meshes[3]));

  return Ms_Success;
}

MsResult InitMaterial()
{
  OpalBufferInitInfo globalBufferInfo = {};
  globalBufferInfo.size = sizeof(state.globalInputValues);
  globalBufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MS_ATTEMPT_OPAL(OpalBufferInit(&state.globalInputBuffer, globalBufferInfo));

  // Global input set
  const uint32_t inputCount = 1;
  OpalInputType inputTypes[inputCount] = { Opal_Input_Type_Uniform_Buffer };

  OpalInputLayoutInitInfo globalLayoutInfo = {};
  globalLayoutInfo.count = inputCount;
  globalLayoutInfo.pTypes = inputTypes;
  OpalInputLayout globalLayout;
  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&globalLayout, globalLayoutInfo));

  OpalMaterialInputValue globalBufferValue = {};
  globalBufferValue.buffer = state.globalInputBuffer;

  OpalInputSetInitInfo globalSetInfo = {};
  globalSetInfo.layout = globalLayout;
  globalSetInfo.pInputValues = &globalBufferValue;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&state.globalInputSet, globalSetInfo));

  OpalShaderInitInfo shaderInfos[2] = { 0 };
  shaderInfos[0].type = Opal_Shader_Vertex;
  shaderInfos[0].size = LapisFileRead("C:/dev/MatSandbox/res/shaders/compiled/nothing.vert.spv", &shaderInfos[0].pSource);
  shaderInfos[1].type = Opal_Shader_Fragment;
  shaderInfos[1].size = LapisFileRead("C:/dev/MatSandbox/res/shaders/compiled/nothing.frag.spv", &shaderInfos[1].pSource);

  state.shaderCount = 2;
  state.pShaders = LapisMemAllocZeroArray(OpalShader, state.shaderCount);

  MS_ATTEMPT_OPAL(OpalShaderInit(&state.pShaders[0], shaderInfos[0]), LapisMemFree(shaderInfos[0].pSource));
  MS_ATTEMPT_OPAL(OpalShaderInit(&state.pShaders[1], shaderInfos[1]), LapisMemFree(shaderInfos[1].pSource));

  LapisMemFree(shaderInfos[0].pSource);
  LapisMemFree(shaderInfos[1].pSource);

  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = state.shaderCount;
  matInfo.pShaders = state.pShaders;
  matInfo.inputLayoutCount = 1;
  matInfo.pInputLayouts = &globalLayout;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = state.renderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  MsUpdateShader(Opal_Shader_Vertex, MATSANDBOX_VERT_DEFAULT_SOURCE);
  MsUpdateShader(Opal_Shader_Fragment, MATSANDBOX_FRAG_DEFAULT_SOURCE);

  return Ms_Success;
}

MsResult InitImgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(LapisWindowGetPlatformData(state.window.lapis).hwnd);

  ImGui_ImplVulkan_InitInfo imguiVulkanInfo = { 0 };
  imguiVulkanInfo.Allocator = NULL;
  imguiVulkanInfo.Instance = oState.vk.instance;
  imguiVulkanInfo.Device = oState.vk.device;
  imguiVulkanInfo.PhysicalDevice = oState.vk.gpu;
  imguiVulkanInfo.QueueFamily = oState.vk.gpuInfo.queueIndexGraphics;
  imguiVulkanInfo.Queue = oState.vk.queueGraphics;
  imguiVulkanInfo.PipelineCache = VK_NULL_HANDLE;
  imguiVulkanInfo.DescriptorPool = oState.vk.descriptorPool;
  imguiVulkanInfo.Subpass = 0;
  imguiVulkanInfo.MinImageCount = 2;
  imguiVulkanInfo.ImageCount = state.window.opal->imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  ImGui_ImplVulkan_Init(&imguiVulkanInfo, state.renderpass->vk.renderpass);

  VkCommandBuffer cmd;
  OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd);
  ImGui_ImplVulkan_CreateFontsTexture(cmd);
  OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd);
  ImGui_ImplVulkan_DestroyFontUploadObjects();

  return Ms_Success;
}

MsResult MsInit()
{
  MS_ATTEMPT(InitOpalBoilerplate());
  MS_ATTEMPT(InitMeshes());
  MS_ATTEMPT(InitMaterial());
  MS_ATTEMPT(InitImgui());

  state.globalInputValues.camProj = ProjectionPerspective(1280.0f / 720.0f, 90.0f, 0.01f, 100.0f);
  state.globalInputValues.camProj.y.y *= -1;
  HandleInput();

  return Ms_Success;
}

void RenderImguiMatrix(const char* title, Mat4* data)
{
  char titles[16][64] = {0};

  for (uint32_t i = 0; i < 16; i++)
  {
    char col, row;

    switch (i % 4)
    {
      case 0: col = 'x'; break;
      case 1: col = 'y'; break;
      case 2: col = 'z'; break;
      case 3: col = 'w'; break;
    }

    switch (i / 4)
    {
    case 0: row = 'x'; break;
    case 1: row = 'y'; break;
    case 2: row = 'z'; break;
    case 3: row = 'w'; break;
    }

    sprintf(titles[i], "##%s-%c%c", title, row, col);
  }

  ImGui::Columns(4);
  ImGui::DragFloat(titles[ 0], &data->x.x, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 1], &data->y.x, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 2], &data->z.x, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 3], &data->w.x, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 4], &data->x.y, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 5], &data->y.y, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 6], &data->z.y, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 7], &data->w.y, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 8], &data->x.z, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 9], &data->y.z, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[10], &data->z.z, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[11], &data->w.z, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[12], &data->x.w, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[13], &data->y.w, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[14], &data->z.w, 0.1f);
  ImGui::NextColumn();
  ImGui::DragFloat(titles[ 15], &data->w.w, 0.1f);

  ImGui::Columns(1);
}

void RenderImgui()
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
    MsUpdateShader(Opal_Shader_Vertex, vertCodeBuffer);
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
    MsUpdateShader(Opal_Shader_Fragment, fragCodeBuffer);
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
}

float camArmLength = 2.0f;
Quaternion camRotQuat;

void HandleInput()
{
  camArmLength -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Wheel) * 0.1f;

  if (LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Mouse_Right)
    || LapisInputGetValue(state.window.lapis, Lapis_Input_Button_R))
  {
    state.camera.transform.rotation.y -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_X);
    state.camera.transform.rotation.x -= LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_Y);
    camRotQuat = QuaternionFromEuler(state.camera.transform.rotation);
    state.globalInputValues.camForward = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, -1.0f });
  }
  else if (LapisInputGetValue(state.window.lapis, Lapis_Input_Button_Mouse_Middle)
    || LapisInputGetValue(state.window.lapis, Lapis_Input_Button_G))
  {
    Vec3 camRight = QuaternionMultiplyVec3(camRotQuat, {1.0f, 0.0f, 0.0f});
    Vec3 camUp = QuaternionMultiplyVec3(camRotQuat, {0.0f, 1.0f, 0.0f});

    camRight = Vec3MultiplyFloat(camRight, -LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_X) * 0.01f * camArmLength);
    camUp = Vec3MultiplyFloat(camUp, LapisInputGetValue(state.window.lapis, Lapis_Input_Axis_Mouse_Delta_Y) * 0.01f * camArmLength);

    state.camera.focusPosition = Vec3AddVec3(state.camera.focusPosition, camRight);
    state.camera.focusPosition = Vec3AddVec3(state.camera.focusPosition, camUp);
  }
  else if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_F))
  {
    state.camera.focusPosition = {0.0f, 0.0f, 0.0f};
  }

  state.camera.transform.position = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, camArmLength });
  state.camera.transform.position = Vec3AddVec3(state.camera.transform.position, state.camera.focusPosition);
  state.globalInputValues.camView = Mat4Invert(TransformToMat4(state.camera.transform));
  state.globalInputValues.camViewProj = Mat4MuliplyMat4(state.globalInputValues.camProj, state.globalInputValues.camView);

  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_1)) state.meshIndex = 0;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_2)) state.meshIndex = 1;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_3)) state.meshIndex = 2;
  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_4)) state.meshIndex  = 3;

  if (LapisInputOnPressed(state.window.lapis, Lapis_Input_Button_Escape)) LapisWindowMarkForClosure(state.window.lapis);

  OpalBufferPushData(state.globalInputBuffer, &state.globalInputValues);
}

MsResult MsUpdate()
{
  while (!LapisWindowGetShouldClose(state.window.lapis))
  {
    LapisWindowUpdate(state.window.lapis);

    HandleInput();

    OpalRenderBegin(state.window.opal);
    OpalRenderBeginRenderpass(state.renderpass, state.framebuffer);
    OpalRenderBindMaterial(state.material);
    OpalRenderBindInputSet(state.globalInputSet, 0);
    OpalRenderMesh(state.meshes[state.meshIndex]);

    RenderImgui();

    OpalRenderEndRenderpass(state.renderpass);
    OpalRenderEnd();
  }

  return Ms_Success;
}

void MsShutdown()
{
  OpalImageShutdown(&state.depthImage);
  OpalShaderShutdown(&state.pShaders[0]);
  OpalShaderShutdown(&state.pShaders[1]);
  OpalMaterialShutdown(&state.material);
  OpalFramebufferShutdown(&state.framebuffer);
  OpalRenderpassShutdown(&state.renderpass);

  OpalMeshShutdown(&state.meshes[0]);
  OpalMeshShutdown(&state.meshes[1]);
  OpalMeshShutdown(&state.meshes[2]);
  OpalMeshShutdown(&state.meshes[3]);

  OpalShutdown();
  LapisWindowShutdown(&state.window.lapis);
}

int main(void)
{
  MsInit();
  MsUpdate();
  MsShutdown();

  return 0;
}
