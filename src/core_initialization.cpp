
#include "src/common.h"

MsResult InitWindow();
MsResult InitSceneRenderResources();
MsResult InitUiRenderResources();
MsResult InitGlobalSet();
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
  state.shaderCount = 2;
  state.pShaderCodeInfos = LapisMemAllocZeroArray(ShaderCodeInfo, 2);

  ShaderCodeInfo* ci = &state.pShaderCodeInfos[0];
  ci->type = Opal_Shader_Vertex;
  ci->capacity = sizeof(MATSANDBOX_VERT_DEFAULT_SOURCE);
  ci->size = ci->capacity;
  ci->buffer = LapisMemAllocArray(char, ci->size);
  LapisMemCopy((void*)MATSANDBOX_VERT_DEFAULT_SOURCE, (void*)ci->buffer, (uint64_t)ci->size);

  ci = &state.pShaderCodeInfos[1];
  ci->type = Opal_Shader_Fragment;
  ci->capacity = sizeof(MATSANDBOX_FRAG_DEFAULT_SOURCE);
  ci->size = ci->capacity;
  ci->buffer = LapisMemAllocArray(char, ci->size);
  LapisMemCopy((void*)MATSANDBOX_FRAG_DEFAULT_SOURCE, (void*)ci->buffer, (uint64_t)ci->size);

  MS_ATTEMPT(InitWindow());
  MS_ATTEMPT(InitSceneRenderResources());
  MS_ATTEMPT(InitUiRenderResources());
  MS_ATTEMPT(InitGlobalSet());
  MS_ATTEMPT(InitMaterial());
  MS_ATTEMPT(InitMeshes());
  MS_ATTEMPT(InitImgui());

  uint32_t windowWidth, windowHeight;
  LapisWindowGetExtents(state.window.lapis, &windowWidth, &windowHeight);

  *state.globalInputValues.camProj = ProjectionPerspectiveExtended(
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
  lapisWindowInfo.position = { 1800, 200 };
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
  MS_ATTEMPT_OPAL(OpalInputLayoutInit(&state.uiSingleImageInputLayout, layoutInfo));

  OpalMaterialInputValue inputValue = { 0 };
  inputValue.image = state.sceneImage;

  OpalInputSetInitInfo setInfo = { 0 };
  setInfo.layout = state.uiSingleImageInputLayout;
  setInfo.pInputValues = &inputValue;
  MS_ATTEMPT_OPAL(OpalInputSetInit(&state.uiSceneImageInputSet, setInfo));

  return Ms_Success;
}

MsResult InitGlobalSet()
{
  MsBufferElementType bufferContentTypes[3] = { Ms_Buffer_Mat4, Ms_Buffer_Mat4, Ms_Buffer_Float3 };

  MsInputArgumentInitInfo bufferInfo;
  bufferInfo.type = Ms_Input_Buffer;
  bufferInfo.bufferInfo.elementCount = 3;
  bufferInfo.bufferInfo.pElementTypes = bufferContentTypes;
  MS_ATTEMPT(MsInputSetAddArgument(&state.globalInputSet, bufferInfo))

  state.globalInputValues.camView = (Mat4*)state.globalInputSet.pArguments[0].data.buffer.pElements[0].data;
  state.globalInputValues.camProj = (Mat4*)state.globalInputSet.pArguments[0].data.buffer.pElements[1].data;
  state.globalInputValues.camForward = (Vec3*)state.globalInputSet.pArguments[0].data.buffer.pElements[2].data;

  MS_ATTEMPT(MsInputSetUpdateLayoutAndSet(&state.globalInputSet));

  return Ms_Success;
}

MsResult InitMaterial()
{
  MS_ATTEMPT(MsCompileShader(&state.pShaderCodeInfos[0], MATSANDBOX_VERT_DEFAULT_SOURCE));
  MS_ATTEMPT(MsCompileShader(&state.pShaderCodeInfos[1], MATSANDBOX_FRAG_DEFAULT_SOURCE));
  MS_ATTEMPT(MsUpdateShader(&state.pShaderCodeInfos[0]));
  MS_ATTEMPT(MsUpdateShader(&state.pShaderCodeInfos[1]));

  MS_ATTEMPT(MsInputSetUpdateLayoutAndSet(&state.materialInputSet));

  OpalShader* pShaders = LapisMemAllocArray(OpalShader, state.shaderCount);
  for (uint32_t i = 0; i < state.shaderCount; i++)
  {
    pShaders[i] = state.pShaderCodeInfos[i].shader;
  }

  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = state.shaderCount;
  matInfo.pShaders = pShaders;
  matInfo.inputLayoutCount = 2;
  OpalInputLayout layouts[2] = { state.globalInputSet.layout, state.materialInputSet.layout };
  matInfo.pInputLayouts = layouts;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = state.sceneRenderpass;
  matInfo.subpassIndex = 0;
  MS_ATTEMPT_OPAL(OpalMaterialInit(&state.material, matInfo));

  LapisMemFree(pShaders);

  return Ms_Success;
}

MsResult InitMeshes()
{
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Shaderball.obj", &state.meshes[0]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Sphere.obj", &state.meshes[1]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/SphereSmooth.obj", &state.meshes[2]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Cube.obj", &state.meshes[3]));
  MS_ATTEMPT(LoadMesh(GAME_RESOURCE_PATH "models/Cyborg_Weapon.obj", &state.meshes[4]));

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
