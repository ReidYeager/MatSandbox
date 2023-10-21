
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "src/window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"

#include <lapis.h>
#include <opal.h>
#include <peridot.h>

#include <string>
#include <memory>

FILE* newShaderHlslSource;
OpalShader shaders[2] = { 0 };
OpalMaterial material;

void CompileShader(char* buffer, bool isFragment)
{
  printf("Code text is:\n\"%s\"\n", buffer);
  uint32_t size = strlen(buffer);

  if (isFragment)
    newShaderHlslSource = fopen("./NewShaderSource.frag", "w");
  else
    newShaderHlslSource = fopen("./NewShaderSource.vert", "w");
  fwrite(buffer, sizeof(char), size, newShaderHlslSource);
  fclose(newShaderHlslSource);

  //printf(">>> Command\n");

  FILE* fp;
  if (isFragment)
    fp = _popen(VULKAN_COMPILER " NewShaderSource.frag -o NewShaderCompiled.spv", "r");
  else
    fp = _popen(VULKAN_COMPILER " NewShaderSource.vert -o NewShaderCompiled.spv", "r");

  printf(">>> Output\n");
  char shellBuffer[1024];
  while (fgets(shellBuffer, 1024, fp) != NULL)
  {
    printf("} \n");
    printf("\"%s\"", shellBuffer);
  }
  printf(">>> End\n");
  _pclose(fp);

  uint32_t shaderIndex = isFragment ? 1 : 0;

  OpalShaderShutdown(&shaders[shaderIndex]);
  OpalShaderInitInfo initInfo = {};
  initInfo.type = isFragment ? Opal_Shader_Fragment : Opal_Shader_Vertex;
  initInfo.size = LapisFileRead("NewShaderCompiled.spv", &initInfo.pSource);
  OpalShaderInit(&shaders[shaderIndex],  initInfo);

  LapisMemFree(initInfo.pSource);

  OpalMaterialReinit(material);
}

void ImguiVkResultCheck(VkResult error)
{
  return;
}

int main(void)
{
  MsbWindow msWindow;
  MsbWindowInitInfo windowInitInfo = { 0 };
  windowInitInfo.extents = { 1280, 720 };
  windowInitInfo.position = { 100, 100 };
  windowInitInfo.title = "Retrieving code from textbox";
  WindowInit(&msWindow, windowInitInfo);

  OpalInitInfo opalInfo = { 0 };
  opalInfo.windowPlatformInfo.hinstance = msWindow.hinstance;
  opalInfo.windowPlatformInfo.hwnd = msWindow.hwnd;
  opalInfo.debug = false;
  OpalFormat vertexInputFormats[3] = { Opal_Format_RGB32, Opal_Format_RGB32, Opal_Format_RG32 };
  opalInfo.vertexStruct.count = 3;
  opalInfo.vertexStruct.pFormats = vertexInputFormats;

  if (OpalInit(opalInfo) != Opal_Success)
  {
    WindowShutdown(&msWindow);
    return -1;
  }

  OpalWindowInitInfo owInfo = { 0 };
  owInfo.platformInfo.hinstance = msWindow.hinstance;
  owInfo.platformInfo.hwnd = msWindow.hwnd;
  owInfo.extents = windowInitInfo.extents;
  OpalWindow oWindow;
  OpalWindowInit(&oWindow, owInfo);

  OpalImage windowRenderImage;
  OpalWindowGetBufferImage(oWindow, &windowRenderImage);

  OpalImageInitInfo dimInfo = { 0 };
  dimInfo.extent = { 1280, 720, 1 };
  dimInfo.format = Opal_Format_Depth;
  dimInfo.sampleType = Opal_Sample_Bilinear;
  dimInfo.usage = Opal_Image_Usage_Depth;
  OpalImage depthImage;
  OpalImageInit(&depthImage, dimInfo);

  OpalAttachmentInfo attachments[2] = {0};
  attachments[0].clearValue.depthStencil = (OpalDepthStencilValue){ 1, 0 };
  attachments[0].format = Opal_Format_Depth;
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
  OpalRenderpass rp;
  OpalRenderpassInit(&rp, rpInfo);

  OpalImage framebufferImages[2] = { depthImage, windowRenderImage };
  OpalFramebufferInitInfo fbInfo = { 0 };
  fbInfo.imageCount = 2;
  fbInfo.pImages = framebufferImages;
  fbInfo.renderpass = rp;
  OpalFramebuffer fb;
  OpalFramebufferInit(&fb, fbInfo);

  OpalShaderInitInfo shaderInfos[2] = { 0 };
  shaderInfos[0].type = Opal_Shader_Vertex;
  shaderInfos[0].size = LapisFileRead("D:/Dev/MatSandbox/res/shaders/compiled/nothing.vert.spv", &shaderInfos[0].pSource);
  shaderInfos[1].type = Opal_Shader_Fragment;
  shaderInfos[1].size = LapisFileRead("D:/Dev/MatSandbox/res/shaders/compiled/nothing.frag.spv", &shaderInfos[1].pSource);

  OpalShaderInit(&shaders[0], shaderInfos[0]);
  OpalShaderInit(&shaders[1], shaderInfos[1]);

  LapisMemFree(shaderInfos[0].pSource);
  LapisMemFree(shaderInfos[1].pSource);

  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.shaderCount = 2;
  matInfo.pShaders = shaders;
  matInfo.inputLayoutCount = 0;
  matInfo.pInputLayouts = NULL;
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = rp;
  matInfo.subpassIndex = 0;
  OpalMaterialInit(&material, matInfo);

  // IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(msWindow.hwnd);

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
  imguiVulkanInfo.ImageCount = oWindow->imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  ImGui_ImplVulkan_Init(&imguiVulkanInfo, rp->vk.renderpass);

  VkCommandBuffer cmd;
  OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd);
  ImGui_ImplVulkan_CreateFontsTexture(cmd);
  OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd);
  ImGui_ImplVulkan_DestroyFontUploadObjects();

  // Rendering

  struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
  };

  Vertex verts[3] = {
    { { 0.0f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { {-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
  };
  uint32_t indices[3] = {
    0, 2, 1
  };

  OpalMeshInitInfo meshInfo = { 0 };
  meshInfo.vertexCount = 3;
  meshInfo.pVertices = verts;
  meshInfo.indexCount = 3;
  meshInfo.pIndices = indices;
  OpalMesh mesh;
  OpalMeshInit(&mesh, meshInfo);

  char vertCodeBuffer[2048] =
    "#version 410\n"
    "\n"
    "layout (location = 0) in vec3 inPosition;\n"
    "layout (location = 1) in vec3 inNormal;\n"
    "layout (location = 2) in vec2 inUv;\n"
    "\n"
    "void main()\n"
    "{\n"
    "  gl_Position = vec4(inPosition, 1.0);\n"
    "}\n";

  char fragCodeBuffer[2048] =
    "#version 410\n"
    "\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    outColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

  while (msWindow.isOpen)
  {
    WindowUpdate(&msWindow);

    OpalRenderBegin(oWindow);
    OpalRenderBeginRenderpass(rp, fb);
    OpalRenderBindMaterial(material);
    OpalRenderMesh(mesh);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("This is a test thing");
    ImGui::Text("Vertex shader");
    if (ImGui::Button("Compile Vert"))
    {
      CompileShader(vertCodeBuffer, false);
    }
    ImGui::InputTextMultiline("##vertSource", vertCodeBuffer, 2048, { -FLT_MIN, ImGui::GetTextLineHeight() * 20 }, ImGuiInputTextFlags_AllowTabInput);

    ImGui::Text("Fragment shader");
    if (ImGui::Button("Compile Frag"))
    {
      CompileShader(fragCodeBuffer, true);
    }
    ImGui::InputTextMultiline("##fragSource", fragCodeBuffer, 2048, {-FLT_MIN, ImGui::GetTextLineHeight() * 20}, ImGuiInputTextFlags_AllowTabInput);

    ImGui::End();
    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

    OpalRenderEndRenderpass(rp);
    OpalRenderEnd();
  }

  OpalImageShutdown(&depthImage);
  OpalShaderShutdown(&shaders[0]);
  OpalShaderShutdown(&shaders[1]);
  OpalMaterialShutdown(&material);
  OpalFramebufferShutdown(&fb);
  OpalRenderpassShutdown(&rp);

  OpalShutdown();
  WindowShutdown(&msWindow);

  return 0;
}
