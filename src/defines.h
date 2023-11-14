
#ifndef MATSANDBOX_DEFINES_H
#define MATSANDBOX_DEFINES_H

#define PERIDOT_VULKAN
#include <peridot.h>
#include <opal.h>
#include <lapis.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define MSB_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif

#define SHADER_NAME_MAX_LENGTH 64

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

#define MS_ATTEMPT_LAPIS(fn, ...) \
{                                 \
  LapisResult result = (fn);      \
  if (result != Lapis_Success)    \
  {                               \
    {                             \
      __VA_ARGS__;                \
    }                             \
    return Ms_Fail;               \
  }                               \
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

struct MsMeshInfo
{
  std::vector<MsVertex> verts;
  std::vector<uint32_t> indices;
};

#define MATSANDBOX_VERT_DEFAULT_SOURCE                                    \
  "#version 460\n"                                                        \
  "\n"                                                                    \
  "layout(set = 0, binding = 0) uniform GlobalUniformStructd\n"           \
  "{\n"                                                                   \
  "  mat4 camView;\n"                                                     \
  "  mat4 camProj;\n"                                                     \
  "  vec3 cameraForward;\n"                                               \
  "} global;\n"                                                           \
  "\n"                                                                    \
  "layout(location = 0) in vec3 inPosition;\n"                            \
  "layout(location = 1) in vec3 inNormal;\n"                              \
  "layout(location = 2) in vec2 inUv;\n"                                  \
  "\n"                                                                    \
  "layout(location = 0) out vec3 outPos;\n"                               \
  "\n"                                                                    \
  "void main()\n"                                                         \
  "{\n"                                                                   \
  "  vec4 p = global.camProj * global.camView * vec4(inPosition, 1.0);\n" \
  "  outPos = inNormal;\n"                                                \
  "  gl_Position = p;\n"                                                  \
  "}\n"


#define MATSANDBOX_FRAG_DEFAULT_SOURCE                                    \
  "#version 460\n"                                                        \
  "\n"                                                                    \
  "layout(set = 0, binding = 0) uniform GlobalUniformStructd\n"           \
  "{\n"                                                                   \
  "  mat4 camView;\n"                                                     \
  "  mat4 camProj;\n"                                                     \
  "  vec3 cameraForward;\n"                                               \
  "} global;\n"                                                           \
  "\n"                                                                    \
  "layout(set = 1, binding = 0) uniform MaterialUniform\n"                \
  "{\n"                                                                   \
  "  float mat;\n"                                                        \
  "  vec3 color;\n"                                                       \
  "} material;\n"                                                         \
  "\n"                                                                    \
  "layout(location = 0) in vec3 inPos;\n"                                 \
  "layout(location = 0) out vec4 outColor;\n"                             \
  "\n"                                                                    \
  "void main()\n"                                                         \
  "{\n"                                                                   \
  "  float d = dot(normalize(global.cameraForward), normalize(inPos));\n" \
  "  d = pow(d * 0.5 + 0.5, 1);\n"                                        \
  "  outColor = vec4(material.color * material.mat, 1.0);\n"              \
  "}\n"

enum MsInputType
{
  Ms_Input_Buffer,
  Ms_Input_Image,
};

enum MsBufferElementType
{
  Ms_Buffer_Float,
  Ms_Buffer_Float2,
  Ms_Buffer_Float3,
  Ms_Buffer_Float4,

  Ms_Buffer_Int,
  Ms_Buffer_Int2,
  Ms_Buffer_Int3,
  Ms_Buffer_Int4,

  Ms_Buffer_Uint,
  Ms_Buffer_Uint2,
  Ms_Buffer_Uint3,
  Ms_Buffer_Uint4,

  Ms_Buffer_double,
  Ms_Buffer_double2,
  Ms_Buffer_double3,
  Ms_Buffer_double4,

  Ms_Buffer_Mat4,
};

struct MsInputArgumentImage
{
  OpalImage image;
};

struct MsBufferElement
{
  std::string name;
  MsBufferElementType type;
  void* data;
};

struct MsInputArgumentBuffer
{
  OpalBuffer buffer;
  uint32_t size;

  uint32_t elementCount;
  MsBufferElement* pElements;
};

struct MsInputArgument
{
  std::string name;
  MsInputType type;
  union
  {
    MsInputArgumentImage image;
    MsInputArgumentBuffer buffer;
  } data;
};

struct MsInputArgumentInitInfo
{
  MsInputType type;
  union
  {
    struct
    {
      uint32_t elementCount;
      MsBufferElementType* pElementTypes;
    } bufferInfo;
    struct
    {
      int x;
    } imageInfo;
  };
};

struct MsMaterialInfo
{
  uint32_t inputArgumentCount;
  MsInputArgument* pInputArguements;

  OpalInputLayout inputLayout;
  OpalInputSet inputSet;
};

struct MatSandboxState
{
  MsbWindow window;

  Vec2I sceneGuiExtentsPrevFrame;
  OpalImage sceneImage;
  OpalImage depthImage;
  OpalImage renderBufferImage;


  OpalRenderpass sceneRenderpass;
  OpalFramebuffer sceneFramebuffer;

  OpalInputLayout uiSceneImageInputLayout;
  OpalInputSet uiSceneImageInputSet;
  OpalRenderpass uiRenderpass;
  OpalFramebuffer uiFramebuffer;

  uint32_t meshIndex;
  OpalMesh meshes[4];

  uint32_t shaderCount;
  OpalShader* pShaders;
  OpalMaterial material;
  MsMaterialInfo materialInfo;

  OpalInputLayout globalInputLayout;
  OpalInputSet globalInputSet;
  OpalBuffer globalInputBuffer;

  struct
  {
    Mat4 camView = mat4Identity;
    Mat4 camProj = mat4Identity;
    Vec3 camForward = { 0.0f, 0.0f, -1.0f, };
  } globalInputValues;

  struct
  {
    Vec3 focusPosition = { 0.0f, 0.0f, 0.0f };
    float armLength = 2.0f;
    Transform transform = transformIdentity;
    Quaternion rotationQuat;
  } camera;
};
extern MatSandboxState state;

#endif // !MATSANDBOX_DEFINES_H
