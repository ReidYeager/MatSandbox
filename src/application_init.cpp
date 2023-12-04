
#include "src/application.h"

MsbResult MsbApplication::Run()
{
  MSB_ATTEMPT(Init());
  MSB_ATTEMPT(Update());
  Shutdown();

  return Msb_Success;
}

#undef CreateWindow

MsbResult CreateWindow(MsbWindow* window);
MsbResult InitGraphics(MsbWindow* window);
MsbResult InitSceneRenderResources(MsbSceneRenderResources* resources);
MsbResult InitUiRenderResources(MsbUiRenderResources* resources, OpalImage sceneImage, OpalImage renderBuffer);
MsbResult InitCustomMaterial(MsbMaterial* material, MsbInputSet* globalSet, MsbInputSet* customSet, OpalRenderpass renderpass);

MsbResult MsbApplication::Init()
{
  // Create window
  // ===============
  MSB_ATTEMPT(CreateWindow(&window));

  // Initialize graphics
  // ===============
  MSB_ATTEMPT(InitGraphics(&window));

  // Single image input layout
  OpalInputLayoutInitInfo singleImageInfo = { 0 };
  OpalInputType type = Opal_Input_Type_Samped_Image;
  singleImageInfo.count = 1;
  singleImageInfo.pTypes = &type;
  MSB_ATTEMPT_OPAL(OpalInputLayoutInit(&msbSingleImageLayout, singleImageInfo));

  // Framebuffers, renderpasses
  MSB_ATTEMPT(InitSceneRenderResources(&sceneRenderResources));
  MSB_ATTEMPT(InitUiRenderResources(&uiRenderResources, sceneRenderResources.sceneImage, window.renderBufferImage));

  // Initialize input sets
  // ===============

  // Global set
  const uint32_t bufferElementCount = 5;
  MsbBufferElementType pBufferElements[bufferElementCount] = {
    Msb_Buffer_Float,  // Time
    Msb_Buffer_Uint2,  // Viewport extents
    Msb_Buffer_Mat4,   // Camera view matrix
    Msb_Buffer_Mat4,   // Camera projection matrix
    Msb_Buffer_Float3, // Camera forward vector
  };

  std::vector<MsbInputArgumentInitInfo> args(1);
  args[0].buffer.elementCount = bufferElementCount;
  args[0].buffer.pElementTypes = pBufferElements;

  MSB_ATTEMPT(globalSet.Init(args));

  // Custom set
  std::vector<MsbInputArgumentInitInfo> emptyArgs;
  MSB_ATTEMPT(customSet.Init(emptyArgs));

  // Compile base material
  // ===============
  MSB_ATTEMPT(InitCustomMaterial(&customMaterial, &globalSet, &customSet, sceneRenderResources.renderpass));

  MSB_LOG("Init success\n");
  return Msb_Success;
}

MsbResult CreateWindow(MsbWindow* window)
{
  LapisWindowInitInfo lapisWindowInfo = {};
  lapisWindowInfo.fnPlatformInputCallback = NULL;
  lapisWindowInfo.fnResizeCallback = NULL;
  lapisWindowInfo.resizable = true;
  lapisWindowInfo.extents = window->extents;
  lapisWindowInfo.position = window->screenPosition;
  lapisWindowInfo.title = window->title;
  MSB_ATTEMPT_LAPIS(LapisWindowInit(lapisWindowInfo, &window->lapis));

  return Msb_Success;
}

MsbResult InitGraphics(MsbWindow* window)
{
  LapisWindowPlatformData windowPlatformData = LapisWindowGetPlatformData(window->lapis);

  OpalInitInfo opalInfo = { 0 };
  opalInfo.windowPlatformInfo.hinstance = windowPlatformData.hinstance;
  opalInfo.windowPlatformInfo.hwnd = windowPlatformData.hwnd;
  opalInfo.debug = true;
  OpalFormat vertexInputFormats[3] = { Opal_Format_RGB32, Opal_Format_RGB32, Opal_Format_RG32 };
  opalInfo.vertexStruct.count = 3;
  opalInfo.vertexStruct.pFormats = vertexInputFormats;

  MSB_ATTEMPT_OPAL(OpalInit(opalInfo));

  OpalWindowInitInfo owInfo = { 0 };
  owInfo.platformInfo.hinstance = windowPlatformData.hinstance;
  owInfo.platformInfo.hwnd = windowPlatformData.hwnd;
  owInfo.extents = window->extents;
  MSB_ATTEMPT_OPAL(OpalWindowInit(&window->opal, owInfo));

  OpalWindowGetBufferImage(window->opal, &window->renderBufferImage);

  return Msb_Success;
}

MsbResult InitSceneRenderResources(MsbSceneRenderResources* resources)
{
  // Images
  // ===============
  OpalImageInitInfo sceneDepthImageInfo = { 0 };
  sceneDepthImageInfo.extent = { 256, 256, 1 };
  sceneDepthImageInfo.format = Opal_Format_D24_S8;
  sceneDepthImageInfo.sampleType = Opal_Sample_Bilinear;
  sceneDepthImageInfo.usage = Opal_Image_Usage_Depth;
  MSB_ATTEMPT_OPAL(OpalImageInit(&resources->depthImage, sceneDepthImageInfo));

  OpalImageInitInfo sceneImageInfo = { 0 };
  sceneImageInfo.extent = { 256, 256, 1 };
  sceneImageInfo.format = Opal_Format_RGBA8;
  sceneImageInfo.sampleType = Opal_Sample_Bilinear;
  sceneImageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Uniform;
  MSB_ATTEMPT_OPAL(OpalImageInit(&resources->sceneImage, sceneImageInfo));

  // Renderpass
  // ===============
  const uint32_t sceneAttachmentCount = 2;
  OpalAttachmentInfo sceneAttachments[sceneAttachmentCount] = { 0 };
  sceneAttachments[0].clearValue.color = (OpalColorValue){ 0.5f, 0.5f, 0.5f, 1.0f };
  sceneAttachments[0].format = Opal_Format_RGBA8;
  sceneAttachments[0].loadOp = Opal_Attachment_Op_Clear;
  sceneAttachments[0].shouldStore = true;
  sceneAttachments[0].usage = Opal_Attachment_Usage_Presented;

  sceneAttachments[1].clearValue.depthStencil = (OpalDepthStencilValue){ 1, 0 };
  sceneAttachments[1].format = Opal_Format_D24_S8;
  sceneAttachments[1].loadOp = Opal_Attachment_Op_Clear;
  sceneAttachments[1].shouldStore = false;
  sceneAttachments[1].usage = Opal_Attachment_Usage_Depth;

  uint32_t sceneColorIndices[1] = { 0 };
  OpalSubpassInfo sceneSubpass = { 0 };
  sceneSubpass.depthAttachmentIndex = 1;
  sceneSubpass.colorAttachmentCount = 1;
  sceneSubpass.pColorAttachmentIndices = sceneColorIndices;
  sceneSubpass.inputAttachmentCount = 0;
  sceneSubpass.pInputColorAttachmentIndices = NULL;

  OpalRenderpassInitInfo sceneRpInfo = { 0 };
  sceneRpInfo.dependencyCount = 0;
  sceneRpInfo.pDependencies = NULL;
  sceneRpInfo.imageCount = sceneAttachmentCount;
  sceneRpInfo.pAttachments = sceneAttachments;
  sceneRpInfo.subpassCount = 1;
  sceneRpInfo.pSubpasses = &sceneSubpass;
  MSB_ATTEMPT_OPAL(OpalRenderpassInit(&resources->renderpass, sceneRpInfo));

  // Framebuffer
  // ===============
  OpalImage sceneFramebufferImages[2] = { resources->sceneImage, resources->depthImage };
  OpalFramebufferInitInfo sceneFbInfo = { 0 };
  sceneFbInfo.imageCount = 2;
  sceneFbInfo.pImages = sceneFramebufferImages;
  sceneFbInfo.renderpass = resources->renderpass;
  MSB_ATTEMPT_OPAL(OpalFramebufferInit(&resources->framebuffer, sceneFbInfo));

  return Msb_Success;
}

MsbResult InitUiRenderResources(MsbUiRenderResources* resources, OpalImage sceneImage, OpalImage renderBuffer)
{
  // Renderpass
  // ===============
  OpalAttachmentInfo uiAttachment;
  uiAttachment.clearValue.color = (OpalColorValue){ 0.1f, 0.1f, 0.4f, 1.0f };
  uiAttachment.format = Opal_Format_BGRA8;
  uiAttachment.loadOp = Opal_Attachment_Op_Clear;
  uiAttachment.shouldStore = true;
  uiAttachment.usage = Opal_Attachment_Usage_Presented;

  uint32_t uiColorIndices[1] = { 0 };
  OpalSubpassInfo uiSubpass = { 0 };
  uiSubpass.depthAttachmentIndex = OPAL_DEPTH_ATTACHMENT_NONE;
  uiSubpass.colorAttachmentCount = 1;
  uiSubpass.pColorAttachmentIndices = uiColorIndices;
  uiSubpass.inputAttachmentCount = 0;
  uiSubpass.pInputColorAttachmentIndices = NULL;

  OpalRenderpassInitInfo uiRpInfo = { 0 };
  uiRpInfo.dependencyCount = 0;
  uiRpInfo.pDependencies = NULL;
  uiRpInfo.imageCount = 1;
  uiRpInfo.pAttachments = &uiAttachment;
  uiRpInfo.subpassCount = 1;
  uiRpInfo.pSubpasses = &uiSubpass;
  MSB_ATTEMPT_OPAL(OpalRenderpassInit(&resources->renderpass, uiRpInfo));

  // Framebuffer
  // ===============
  OpalFramebufferInitInfo uiFbInfo = { 0 };
  uiFbInfo.imageCount = 1;
  uiFbInfo.pImages = &renderBuffer;
  uiFbInfo.renderpass = resources->renderpass;
  MSB_ATTEMPT_OPAL(OpalFramebufferInit(&resources->framebuffer, uiFbInfo));

  return Msb_Success;
}

MsbResult InitCustomMaterial(MsbMaterial* material, MsbInputSet* globalSet, MsbInputSet* customSet, OpalRenderpass renderpass)
{
  MsbMaterialInitInfo initInfo;
  initInfo.renderpass = renderpass;
  initInfo.pInputSets.push_back(globalSet);
  initInfo.pInputSets.push_back(customSet);

  MsbShaderInitInfo shaderInfo;

  // Vertex shader
  // ===============
  uint32_t vertSourceSize = strlen(MATSANDBOX_VERT_DEFAULT_SOURCE);
  char* pVertSource = LapisMemAllocZeroArray(char, vertSourceSize);
  LapisMemCopy((char*)MATSANDBOX_VERT_DEFAULT_SOURCE, pVertSource, vertSourceSize);

  shaderInfo.type = Msb_Shader_Vertex;
  shaderInfo.sourceSize = vertSourceSize;
  shaderInfo.pSource = pVertSource;
  initInfo.pShaderInfos.push_back(shaderInfo);

  // Fragment shader
  // ===============
  uint32_t fragSourceSize = strlen(MATSANDBOX_FRAG_DEFAULT_SOURCE);
  char* pFragSource = LapisMemAllocZeroArray(char, fragSourceSize);
  LapisMemCopy((char*)MATSANDBOX_FRAG_DEFAULT_SOURCE, pFragSource, fragSourceSize);

  shaderInfo.type = Msb_Shader_Fragment;
  shaderInfo.sourceSize = fragSourceSize;
  shaderInfo.pSource = pFragSource;
  initInfo.pShaderInfos.push_back(shaderInfo);

  // Create material
  // ===============
  MSB_ATTEMPT(material->Init(initInfo));

  LapisMemFree(pVertSource);
  LapisMemFree(pFragSource);

  return Msb_Success;
}
