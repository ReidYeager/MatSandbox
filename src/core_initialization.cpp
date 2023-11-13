
#include "src/common.h"

MsResult InitWindow();
MsResult InitSceneRenderResources();
MsResult InitUiRenderResources();
MsResult InitMaterial();
MsResult InitMeshes();
MsResult InitImgui();

void ImguiVkResultCheck(VkResult error) {}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void LapisInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

void WindowResizeCallback(LapisWindow window, uint32_t width, uint32_t height)
{
  if (width == 0 || height == 0 || LapisWindowGetMinimized(window))
    return;

  if (OpalWindowReinit(state.window.opal) != Opal_Success)
  {
    LapisWindowMarkForClosure(window);
    return;
  }

  if (OpalFramebufferReinit(state.uiFramebuffer) != Opal_Success)
  {
    LapisWindowMarkForClosure(window);
    return;
  }

  Render();
}

MsResult MsInit()
{
  MS_ATTEMPT(InitWindow());
  MS_ATTEMPT(InitSceneRenderResources());
  MS_ATTEMPT(InitUiRenderResources());
  MS_ATTEMPT(InitMaterial());
  MS_ATTEMPT(InitMeshes());
  MS_ATTEMPT(InitImgui());

  uint32_t windowWidth, windowHeight;
  LapisWindowGetExtents(state.window.lapis, &windowWidth, &windowHeight);

  state.globalInputValues.camProj = ProjectionPerspectiveExtended(
    (float)windowWidth / (float)windowHeight,
    1.0f,    // 1:1 guaranteed ratio
    90.0f,   // VFov
    0.01f,   // near clip
    100.0f); // far clip

  UpdateCamera();

  return Ms_Success;
}

MsResult InitWindow()
{
  LapisWindowInitInfo lapisWindowInfo = {};
  lapisWindowInfo.fnPlatformInputCallback = LapisInputCallback;
  lapisWindowInfo.fnResizeCallback = WindowResizeCallback;
  lapisWindowInfo.resizable = true;
  lapisWindowInfo.extents = { 1280, 720 };
  lapisWindowInfo.position = { 100, 100 };
  lapisWindowInfo.title = "Material with lapis";
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

  return Ms_Success;
}

MsResult InitSceneRenderResources()
{
  OpalImageInitInfo depthImageInfo = { 0 };
  depthImageInfo.extent = { 1280, 720, 1 };
  depthImageInfo.format = Opal_Format_D24_S8;
  depthImageInfo.sampleType = Opal_Sample_Bilinear;
  depthImageInfo.usage = Opal_Image_Usage_Depth;
  MS_ATTEMPT_OPAL(OpalImageInit(&state.depthImage, depthImageInfo));

  OpalImageInitInfo sceneImageInfo = { 0 };
  sceneImageInfo.extent = { 1280, 720, 1 };
  sceneImageInfo.format = Opal_Format_RGBA8;
  sceneImageInfo.sampleType = Opal_Sample_Bilinear;
  sceneImageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Uniform;
  MS_ATTEMPT_OPAL(OpalImageInit(&state.sceneImage, sceneImageInfo));

  // Renderpass init
  OpalAttachmentInfo attachments[2] = { 0 };
  attachments[0].clearValue.color = (OpalColorValue){ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[0].format = Opal_Format_BGRA8;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].usage = Opal_Attachment_Usage_Presented;

  attachments[1].clearValue.depthStencil = (OpalDepthStencilValue){ 1, 0 };
  attachments[1].format = Opal_Format_D24_S8;
  attachments[1].loadOp = Opal_Attachment_Op_Clear;
  attachments[1].shouldStore = false;
  attachments[1].usage = Opal_Attachment_Usage_Depth;

  uint32_t colorIndices[1] = { 0 };
  OpalSubpassInfo subpass = { 0 };
  subpass.depthAttachmentIndex = 1;
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
  MS_ATTEMPT_OPAL(OpalRenderpassInit(&state.sceneRenderpass, rpInfo));

  // Framebuffer init
  OpalImage framebufferImages[2] = { state.sceneImage, state.depthImage };
  OpalFramebufferInitInfo fbInfo = { 0 };
  fbInfo.imageCount = 2;
  fbInfo.pImages = framebufferImages;
  fbInfo.renderpass = state.sceneRenderpass;
  MS_ATTEMPT_OPAL(OpalFramebufferInit(&state.sceneFramebuffer, fbInfo));

  return Ms_Success;
}

MsResult InitUiRenderResources()
{
  // Renderpass init
  OpalAttachmentInfo attachment;
  attachment.clearValue.color = (OpalColorValue){ 0.1f, 0.1f, 0.4f, 1.0f };
  attachment.format = Opal_Format_BGRA8;
  attachment.loadOp = Opal_Attachment_Op_Clear;
  attachment.shouldStore = true;
  attachment.usage = Opal_Attachment_Usage_Presented;

  uint32_t colorIndices[1] = { 0 };
  OpalSubpassInfo subpass = { 0 };
  subpass.depthAttachmentIndex = OPAL_DEPTH_ATTACHMENT_NONE;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = colorIndices;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = NULL;

  OpalRenderpassInitInfo rpInfo = { 0 };
  rpInfo.dependencyCount = 0;
  rpInfo.pDependencies = NULL;
  rpInfo.imageCount = 1;
  rpInfo.pAttachments = &attachment;
  rpInfo.subpassCount = 1;
  rpInfo.pSubpasses = &subpass;
  MS_ATTEMPT_OPAL(OpalRenderpassInit(&state.uiRenderpass, rpInfo));

  // Framebuffer init
  OpalFramebufferInitInfo fbInfo = { 0 };
  fbInfo.imageCount = 1;
  fbInfo.pImages = &state.renderBufferImage;
  fbInfo.renderpass = state.uiRenderpass;
  MS_ATTEMPT_OPAL(OpalFramebufferInit(&state.uiFramebuffer, fbInfo));

  OpalInputLayoutInitInfo layoutInfo = { 0 };
  OpalInputType type = Opal_Input_Type_Samped_Image;
  layoutInfo.count = 1;
  layoutInfo.pTypes = &type;
  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&state.uiSceneImageInputLayout, layoutInfo));

  OpalMaterialInputValue inputValue = { 0 };
  inputValue.image = state.sceneImage;

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = state.uiSceneImageInputLayout;
  setInfo.pInputValues = &inputValue;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&state.uiSceneImageInputSet, setInfo));

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
  matInfo.renderpass = state.sceneRenderpass;
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
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
  ImGui_ImplVulkan_Init(&imguiVulkanInfo, state.uiRenderpass->vk.renderpass);

  VkCommandBuffer cmd;
  OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd);
  ImGui_ImplVulkan_CreateFontsTexture();
  OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd);

  return Ms_Success;
}
