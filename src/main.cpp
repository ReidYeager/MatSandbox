
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <lapis.h>
#include <opal.h>
#include <peridot.h>

int main(void)
{
  LapisWindow window;
  LapisWindowInitInfo windowInfo = { 0 };
  windowInfo.extents = { 1280, 720 };
  windowInfo.position = { 100, 100 };
  windowInfo.resizable = true;
  windowInfo.title = "Material sandbox";
  if (LapisWindowInit(windowInfo, &window) != Lapis_Success)
  {
    return -1;
  }

  OpalInitInfo opalInfo = { 0 };
  opalInfo.lapisWindow = window;
  opalInfo.debug = false;
  OpalFormat vertexInputFormats[3] = { Opal_Format_RGB32, Opal_Format_RGB32, Opal_Format_RG32 };
  opalInfo.vertexStruct.count = 3;
  opalInfo.vertexStruct.pFormats = vertexInputFormats;

  if (OpalInit(opalInfo) != Opal_Success)
  {
    LapisWindowShutdown(&window);
    return -1;
  }

  OpalWindowInitInfo owInfo = { 0 };
  owInfo.lapisWindow = window;
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

  OpalShader shaders[2] = { 0 };
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
  OpalMaterial material;
  OpalMaterialInit(&material, matInfo);

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

  while (!LapisWindowGetShouldClose(window))
  {
    LapisWindowProcessOsEvents(window);

    OpalRenderBegin(oWindow);
    OpalRenderBeginRenderpass(rp, fb);
    OpalRenderBindMaterial(material);
    OpalRenderMesh(mesh);
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
  LapisWindowShutdown(&window);

  return 0;
}
