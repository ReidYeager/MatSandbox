
#ifndef MATSANDBOX_DEFINES_H
#define MATSANDBOX_DEFINES_H

#include <peridot.h>
#include <opal.h>
#include <lapis.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define MSB_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif

enum MsResult
{
  Ms_Success,
  Ms_Fail,
  Ms_Fail_Invalid_Field,
  MS_Fail_External
};

#define MS_ATTEMPT(fn, ...) \
{                           \
  MsResult result = (fn);   \
  if (result != Ms_Success) \
  {                         \
    return Ms_Fail;         \
  }                         \
}

#define MS_ATTEMPT_OPAL(fn, ...) \
{                                \
  OpalResult result = (fn);      \
  if (result != Opal_Success)    \
  {                              \
    {                            \
      __VA_ARGS__;               \
    }                            \
    return Ms_Fail;              \
  }                              \
}

typedef struct MsbWindow
{
  LapisWindow lapis;
  OpalWindow opal;
} MsbWindow;

struct MsVertex {
  Vec3 position;
  Vec3 normal;
  Vec2 uv;

  bool operator==(const MsVertex& other) const
  {
    return Vec3Compare(position, other.position) && Vec3Compare(normal, other.normal) && Vec2Compare(uv, other.uv);
  }
};

struct MsMeshInfo {
  std::vector<MsVertex> verts;
  std::vector<uint32_t> indices;
};

#define MATSANDBOX_VERT_DEFAULT_SOURCE                          \
  "#version 460\n"                                              \
  "\n"                                                          \
  "layout(set = 0, binding = 0) uniform GlobalUniformStructd\n" \
  "{\n"                                                         \
  "  mat4 cameraView;\n"                                        \
  "  mat4 cameraProjection;\n"                                  \
  "  mat4 viewProj;\n"                                          \
  "  vec3 cameraForward;\n"                                     \
  "} global;\n"                                                 \
  "\n"                                                          \
  "layout(location = 0) in vec3 inPosition;\n"                  \
  "layout(location = 1) in vec3 inNormal;\n"                    \
  "layout(location = 2) in vec2 inUv;\n"                        \
  "\n"                                                          \
  "layout(location = 0) out vec3 outPos;\n"                     \
  "\n"                                                          \
  "void main()\n"                                               \
  "{\n"                                                         \
  "  vec4 p = global.viewProj * vec4(inPosition, 1.0);\n"       \
  "  outPos = inNormal;\n"                                      \
  "  gl_Position = p;\n"                                        \
  "}\n"


#define MATSANDBOX_FRAG_DEFAULT_SOURCE                                    \
  "#version 460\n"                                                        \
  "\n"                                                                    \
  "layout(set = 0, binding = 0) uniform GlobalUniformStructd\n"           \
  "{\n"                                                                   \
  "  mat4 cameraView;\n"                                                  \
  "  mat4 cameraProjection;\n"                                            \
  "  mat4 viewProj;\n"                                                    \
  "  vec3 cameraForward;\n"                                               \
  "} global;\n"                                                           \
  "\n"                                                                    \
  "layout(location = 0) in vec3 inPos;\n"                                 \
  "layout(location = 0) out vec4 outColor;\n"                             \
  "\n"                                                                    \
  "void main()\n"                                                         \
  "{\n"                                                                   \
  "  float d = dot(normalize(global.cameraForward), normalize(inPos));\n" \
  "  d = pow(d * -0.5 + 0.5, 1);\n"                                       \
  "  outColor = vec4(d);\n"                                               \
  "}\n"

struct MatSandboxState
{
  MsbWindow window;

  OpalImage renderBufferImage;
  OpalImage depthImage;

  OpalRenderpass renderpass;
  OpalFramebuffer framebuffer;

  uint32_t meshIndex;
  OpalMesh meshes[4];

  uint32_t shaderCount;
  OpalShader* pShaders;
  OpalMaterial material;

  OpalInputSet globalInputSet;
  OpalBuffer globalInputBuffer;

  struct
  {
    Mat4 camView = mat4Identity;
    Mat4 camProj = mat4Identity;
    Mat4 camViewProj = mat4Identity;
    Vec3 camForward = { 0.0f, 0.0f, -1.0f, };
  } globalInputValues;

  struct
  {
    Vec3 focusPosition = { 0.0f, 0.0f, 0.0f };
    Transform transform = transformIdentity;
  } camera;
};
extern MatSandboxState state;

#endif // !MATSANDBOX_DEFINES_H
