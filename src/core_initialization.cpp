
#include "src/common.h"

MsResult InitOpalBoilerplate();
MsResult InitMaterial();
MsResult InitMeshes();
MsResult InitImgui();

void ImguiVkResultCheck(VkResult error) {}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void LapisInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

MsResult MsInit()
{
  MS_ATTEMPT(InitOpalBoilerplate());
  MS_ATTEMPT(InitMaterial());
  MS_ATTEMPT(InitMeshes());
  MS_ATTEMPT(InitImgui());

  state.globalInputValues.camProj = ProjectionPerspective(1280.0f / 720.0f, 90.0f, 0.01f, 100.0f);
  state.globalInputValues.camProj.y.y *= -1;

  UpdateCamera();

  return Ms_Success;
}

MsResult InitOpalBoilerplate()
{
  LapisWindowInitInfo lapisWindowInfo = {};
  lapisWindowInfo.fnPlatformInputCallback = LapisInputCallback;
  lapisWindowInfo.extents = { 1280, 720 };
  lapisWindowInfo.position = { 1800, 100 };
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
  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&state.globalInputLayout, globalLayoutInfo));

  OpalMaterialInputValue globalBufferValue = {};
  globalBufferValue.buffer = state.globalInputBuffer;

  OpalInputSetInitInfo globalSetInfo = {};
  globalSetInfo.layout = state.globalInputLayout;
  globalSetInfo.pInputValues = &globalBufferValue;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&state.globalInputSet, globalSetInfo));

  state.shaderCount = 2;
  state.pShaders = LapisMemAllocZeroArray(OpalShader, state.shaderCount);
  MsCompileShader(Opal_Shader_Vertex, MATSANDBOX_VERT_DEFAULT_SOURCE);
  MsCompileShader(Opal_Shader_Fragment, MATSANDBOX_FRAG_DEFAULT_SOURCE);
  MsUpdateShader(Opal_Shader_Vertex);
  MsUpdateShader(Opal_Shader_Fragment);

  // TODO : !!! Remove this. Temp input buffer argument information for testing
  state.materialInfo.inputArgumentCount = 1;
  state.materialInfo.pInputArguements = LapisMemAllocZeroArray(MsInputArgument, 1);
  state.materialInfo.pInputArguements[0].type = Ms_Input_Buffer;
  state.materialInfo.pInputArguements[0].data.buffer.elementCount = 1;
  state.materialInfo.pInputArguements[0].data.buffer.pElements = LapisMemAllocZeroSingle(MsBufferElement);
  state.materialInfo.pInputArguements[0].data.buffer.pElements[0].type = Ms_Buffer_Float3;
  state.materialInfo.pInputArguements[0].data.buffer.pElements[0].data = LapisMemAllocZero(sizeof(Vec3));
  ((Vec3*)state.materialInfo.pInputArguements[0].data.buffer.pElements[0].data)->r = 0.5f;
  ((Vec3*)state.materialInfo.pInputArguements[0].data.buffer.pElements[0].data)->g = 0.2f;
  ((Vec3*)state.materialInfo.pInputArguements[0].data.buffer.pElements[0].data)->b = 0.9f;

  MS_ATTEMPT(MsUpdateMaterialInputLayoutAndSet());

  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = state.shaderCount;
  matInfo.pShaders = state.pShaders;
  matInfo.inputLayoutCount = 2;
  OpalInputLayout layouts[2] = { state.globalInputLayout, state.materialInfo.inputLayout };
  matInfo.pInputLayouts = layouts;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = state.renderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  return Ms_Success;
}

MsResult InitMeshes()
{
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Sphere.obj", &state.meshes[0]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/SphereSmooth.obj", &state.meshes[1]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Cube.obj", &state.meshes[2]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Cyborg_Weapon.obj", &state.meshes[3]));

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
