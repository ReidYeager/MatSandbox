
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

MsbWindow msWindow;
OpalImage depthImage;
OpalRenderpass renderpass;
OpalFramebuffer framebuffer;
OpalShader shaders[2] = { 0 };
OpalMaterial material;
uint32_t selectedMesh = 0;
uint32_t renderedModelIndex = 0;
OpalMesh meshes[4];

OpalInputSet globalInputSet;
OpalBuffer globalInputBuffer;
Transform camTransform = transformIdentity;
struct GlobalInputStruct
{
  Mat4 cameraView;
  Mat4 cameraProjection;
  Mat4 viewProj;
  Vec3 cameraForward;
} globalStruct;

void ImguiVkResultCheck(VkResult error) {}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
  LapisWindowInit(lapisWindowInfo, &msWindow.lapis);

  LapisWindowPlatformData windowPlatformData = LapisWindowGetPlatformData(msWindow.lapis);

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
  MS_ATTEMPT_OPAL(OpalWindowInit(&msWindow.opal, owInfo));

  OpalImage windowRenderImage;
  OpalWindowGetBufferImage(msWindow.opal, &windowRenderImage);

  OpalImageInitInfo dimInfo = { 0 };
  dimInfo.extent = { 1280, 720, 1 };
  dimInfo.format = Opal_Format_D24_S8;
  dimInfo.sampleType = Opal_Sample_Bilinear;
  dimInfo.usage = Opal_Image_Usage_Depth;
  MS_ATTEMPT_OPAL(OpalImageInit(&depthImage, dimInfo));

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
  MS_ATTEMPT_OPAL(OpalRenderpassInit(&renderpass, rpInfo));

  OpalImage framebufferImages[2] = { depthImage, windowRenderImage };
  OpalFramebufferInitInfo fbInfo = { 0 };
  fbInfo.imageCount = 2;
  fbInfo.pImages = framebufferImages;
  fbInfo.renderpass = renderpass;
  MS_ATTEMPT_OPAL(OpalFramebufferInit(&framebuffer, fbInfo));

  return Ms_Success;
}

MsResult InitMeshes()
{
  MS_ATTEMPT(LoadMesh("D:/Dev/Library/res/models/Sphere.obj", &meshes[0]));
  MS_ATTEMPT(LoadMesh("D:/Dev/Library/res/models/SphereSmooth.obj", &meshes[1]));
  MS_ATTEMPT(LoadMesh("D:/Dev/Library/res/models/Cube.obj", &meshes[2]));
  MS_ATTEMPT(LoadMesh("D:/Dev/Library/res/models/Cyborg_Weapon.obj", &meshes[3]));

  return Ms_Success;
}

MsResult InitMaterial()
{
  OpalBufferInitInfo globalBufferInfo = {};
  globalBufferInfo.size = sizeof(GlobalInputStruct);
  globalBufferInfo.usage = Opal_Buffer_Usage_Uniform;
  MS_ATTEMPT_OPAL(OpalBufferInit(&globalInputBuffer, globalBufferInfo));

  // Global input set
  const uint32_t inputCount = 1;
  OpalInputType inputTypes[inputCount] = { Opal_Input_Type_Uniform_Buffer };

  OpalInputLayoutInitInfo globalLayoutInfo = {};
  globalLayoutInfo.count = inputCount;
  globalLayoutInfo.pTypes = inputTypes;
  OpalInputLayout globalLayout;
  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&globalLayout, globalLayoutInfo));

  OpalMaterialInputValue globalBufferValue = {};
  globalBufferValue.buffer = globalInputBuffer;

  OpalInputSetInitInfo globalSetInfo = {};
  globalSetInfo.layout = globalLayout;
  globalSetInfo.pInputValues = &globalBufferValue;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&globalInputSet, globalSetInfo));

  OpalShaderInitInfo shaderInfos[2] = { 0 };
  shaderInfos[0].type = Opal_Shader_Vertex;
  shaderInfos[0].size = LapisFileRead("D:/Dev/MatSandbox/res/shaders/compiled/nothing.vert.spv", &shaderInfos[0].pSource);
  shaderInfos[1].type = Opal_Shader_Fragment;
  shaderInfos[1].size = LapisFileRead("D:/Dev/MatSandbox/res/shaders/compiled/nothing.frag.spv", &shaderInfos[1].pSource);

  MS_ATTEMPT_OPAL(OpalShaderInit(&shaders[0], shaderInfos[0]), LapisMemFree(shaderInfos[0].pSource));
  MS_ATTEMPT_OPAL(OpalShaderInit(&shaders[1], shaderInfos[1]), LapisMemFree(shaderInfos[1].pSource));

  LapisMemFree(shaderInfos[0].pSource);
  LapisMemFree(shaderInfos[1].pSource);

  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = 2;
  matInfo.pShaders = shaders;
  matInfo.inputLayoutCount = 1;
  matInfo.pInputLayouts = &globalLayout;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = renderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&material, matInfo));

  return Ms_Success;
}

MsResult InitImgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(LapisWindowGetPlatformData(msWindow.lapis).hwnd);

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
  imguiVulkanInfo.ImageCount = msWindow.opal->imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  ImGui_ImplVulkan_Init(&imguiVulkanInfo, renderpass->vk.renderpass);

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

  return Ms_Success;
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
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());
}

float camArmLength = 2.0f;
Quaternion camRotQuat;

void HandleInput()
{
  camArmLength += (LapisInputGetValue(msWindow.lapis, Lapis_Input_Button_K) - LapisInputGetValue(msWindow.lapis, Lapis_Input_Button_I)) * 0.0006f;

  if (LapisInputGetValue(msWindow.lapis, Lapis_Input_Button_Mouse_Right))
  {
    camTransform.rotation.y -= LapisInputGetValue(msWindow.lapis, Lapis_Input_Axis_Mouse_Delta_X);
    camTransform.rotation.x -= LapisInputGetValue(msWindow.lapis, Lapis_Input_Axis_Mouse_Delta_Y);
    camRotQuat = QuaternionFromEuler(camTransform.rotation);
    globalStruct.cameraForward = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, -1.0f });
  }

  camTransform.position = QuaternionMultiplyVec3(camRotQuat, { 0.0f, 0.0f, camArmLength });

  globalStruct.cameraView = Mat4Invert(TransformToMat4(camTransform));

  globalStruct.cameraProjection = ProjectionPerspective(1280.0f / 720.0f, 90.0f, 0.01f, 100.0f);
  globalStruct.cameraProjection.y.y *= -1;

  globalStruct.viewProj = Mat4MuliplyMat4(globalStruct.cameraProjection, globalStruct.cameraView);

  if (LapisInputOnPressed(msWindow.lapis, Lapis_Input_Button_1)) renderedModelIndex = 0;
  if (LapisInputOnPressed(msWindow.lapis, Lapis_Input_Button_2)) renderedModelIndex = 1;
  if (LapisInputOnPressed(msWindow.lapis, Lapis_Input_Button_3)) renderedModelIndex = 2;
  if (LapisInputOnPressed(msWindow.lapis, Lapis_Input_Button_4)) renderedModelIndex = 3;

  OpalBufferPushData(globalInputBuffer, &globalStruct);
}

MsResult MsUpdate()
{
  while (!LapisWindowGetShouldClose(msWindow.lapis))
  {
    LapisWindowUpdate(msWindow.lapis);

    HandleInput();

    OpalRenderBegin(msWindow.opal);
    OpalRenderBeginRenderpass(renderpass, framebuffer);
    OpalRenderBindMaterial(material);
    OpalRenderBindInputSet(globalInputSet, 0);
    OpalRenderMesh(meshes[renderedModelIndex]);

    RenderImgui();

    OpalRenderEndRenderpass(renderpass);
    OpalRenderEnd();
  }

  return Ms_Success;
}

void MsShutdown()
{
  OpalImageShutdown(&depthImage);
  OpalShaderShutdown(&shaders[0]);
  OpalShaderShutdown(&shaders[1]);
  OpalMaterialShutdown(&material);
  OpalFramebufferShutdown(&framebuffer);
  OpalRenderpassShutdown(&renderpass);

  OpalMeshShutdown(&meshes[0]);
  OpalMeshShutdown(&meshes[1]);

  OpalShutdown();
  LapisWindowShutdown(&msWindow.lapis);
}

int main(void)
{
  MsInit();
  MsUpdate();
  MsShutdown();

  return 0;
}
