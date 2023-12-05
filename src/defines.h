
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

#define MS_SHADER_NAME_MAX_LENGTH 64
#define MS_CODE_MAX_LENGTH 2048

#define MSB_LOG(msg, ...) LapisConsolePrintMessage(Lapis_Console_Info, "MSB :: " msg, __VA_ARGS__);
#define MSB_ERR(msg, ...) LapisConsolePrintMessage(Lapis_Console_Error, "MSB :: " msg, __VA_ARGS__);

enum MsbResult
{
  Msb_Success,
  Msb_Fail,
  Msb_Fail_Invalid_Field,
  Msb_Fail_External
};

extern uint32_t attemptDepth;

#define MSB_ATTEMPT(fn, ...)                                                                             \
{                                                                                                        \
  attemptDepth++;                                                                                        \
  MsbResult result = (fn);                                                                               \
  attemptDepth--;                                                                                        \
  if (result != Msb_Success)                                                                             \
  {                                                                                                      \
    MSB_ERR("ERR-%02u :: Function failure :: '%s'\n\t%s : %d\n", attemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                    \
      __VA_ARGS__;                                                                                       \
    }                                                                                                    \
    return Msb_Fail;                                                                                     \
  }                                                                                                      \
}

#define MSB_ATTEMPT_OPAL(fn, ...)                                                                             \
{                                                                                                             \
  attemptDepth++;                                                                                             \
  OpalResult result = (fn);                                                                                   \
  attemptDepth--;                                                                                             \
  if (result != Opal_Success)                                                                                 \
  {                                                                                                           \
    MSB_ERR("ERR-%02u :: Opal function failure :: '%s'\n\t%s : %d\n", attemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                         \
      __VA_ARGS__;                                                                                            \
    }                                                                                                         \
    return Msb_Fail;                                                                                          \
  }                                                                                                           \
}

#define MSB_ATTEMPT_LAPIS(fn, ...)                                                                             \
{                                                                                                              \
  attemptDepth++;                                                                                              \
  LapisResult result = (fn);                                                                                   \
  attemptDepth--;                                                                                              \
  if (result != Lapis_Success)                                                                                 \
  {                                                                                                            \
    MSB_ERR("ERR-%02u :: Lapis function failure :: '%s'\n\t%s : %d\n", attemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                          \
      __VA_ARGS__;                                                                                             \
    }                                                                                                          \
    return Msb_Fail;                                                                                           \
  }                                                                                                            \
}

typedef struct MsWindow
{
  LapisWindow lapis;
  OpalWindow opal;
} MsWindow;

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

#define MATSANDBOX_VERT_DEFAULT_SOURCE                                         \
  "#version 460\n"                                                             \
  "\n"                                                                         \
  "layout(set = 0, binding = 0) uniform GlobalUniformStruct\n"                 \
  "{\n"                                                                        \
  "\tfloat time;\n"                                                            \
  "\tuvec2 viewportExtents;\n"                                                 \
  "\tmat4 camView;\n"                                                          \
  "\tmat4 camProj;\n"                                                          \
  "\tvec3 cameraForward;\n"                                                    \
  "} global;\n"                                                                \
  "\n"                                                                         \
  "layout(location = 0) in vec3 inPosition;\n"                                 \
  "layout(location = 1) in vec3 inNormal;\n"                                   \
  "layout(location = 2) in vec2 inUv;\n"                                       \
  "\n"                                                                         \
  "layout(location = 0) out vec3 outPosition;\n"                               \
  "layout(location = 1) out vec3 outNormal;\n"                                 \
  "layout(location = 2) out vec2 outUv;\n"                                     \
  "\n"                                                                         \
  "void main()\n"                                                              \
  "{\n"                                                                        \
  "\toutPosition = inPosition;\n"                                              \
  "\toutNormal = inNormal;\n"                                                  \
  "\toutUv = inUv;\n"                                                          \
  "\tgl_Position = global.camProj * global.camView * vec4(inPosition, 1.0);\n" \
  "}\n"


#define MATSANDBOX_FRAG_DEFAULT_SOURCE                                       \
  "#version 460\n"                                                           \
  "\n"                                                                       \
  "layout(set = 0, binding = 0) uniform GlobalUniformStruct\n"               \
  "{\n"                                                                      \
  "\tfloat time;\n"                                                          \
  "\tuvec2 viewportExtents;\n"                                               \
  "\tmat4 camView;\n"                                                        \
  "\tmat4 camProj;\n"                                                        \
  "\tvec3 cameraForward;\n"                                                  \
  "} global;\n"                                                              \
  "\n"                                                                       \
  "layout(location = 0) in vec3 inPosition;\n"                               \
  "layout(location = 1) in vec3 inNormal;\n"                                 \
  "layout(location = 2) in vec2 inUv;\n"                                     \
  "\n"                                                                       \
  "layout(location = 0) out vec4 outColor;\n"                                \
  "\n"                                                                       \
  "void main()\n"                                                            \
  "{\n"                                                                      \
  "\toutColor = vec4(1.0);\n"                                                \
  "}\n"

enum MsInputType
{
  Ms_Input_Buffer,
  Ms_Input_Image,
};

enum MsBufferElementType
{
  Ms_Buffer_Int,
  Ms_Buffer_Int2,
  Ms_Buffer_Int3,
  Ms_Buffer_Int4,

  Ms_Buffer_Uint,
  Ms_Buffer_Uint2,
  Ms_Buffer_Uint3,
  Ms_Buffer_Uint4,

  Ms_Buffer_Float,
  Ms_Buffer_Float2,
  Ms_Buffer_Float3,
  Ms_Buffer_Float4,

  Ms_Buffer_Double,
  Ms_Buffer_Double2,
  Ms_Buffer_Double3,
  Ms_Buffer_Double4,

  Ms_Buffer_Mat4,
  Ms_Buffer_COUNT,
};

static const char* MsBufferElementTypeNames[Ms_Buffer_COUNT] =
{
  "Int",
  "Int2",
  "Int3",
  "Int4",
  "Uint",
  "Uint2",
  "Uint3",
  "Uint4",
  "Float",
  "Float2",
  "Float3",
  "Float4",
  "Double",
  "Double2",
  "Double3",
  "Double4",
  "Mat4"
};

struct MsInputArgumentImage
{
  OpalImage image;
  OpalInputSet set;
  char* sourcePath;
};

struct MsBufferElement
{
  char* name;
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
  uint32_t id;
  char* name;
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
      char* imagePath;
      void* imageData;
      OpalExtent extents;
    } imageInfo;
  };
};

struct MsInputSet
{
  uint32_t count;
  uint32_t nextId = 0;
  MsInputArgument* pArguments;

  OpalInputLayout layout;
  OpalInputSet set;
};

struct MsMaterialInfo
{
  uint32_t inputArgumentNextId = 0;
  uint32_t inputArgumentCount;
  MsInputArgument* pInputArguements;

  OpalInputLayout singleImageLayout;
  OpalInputSet inputSet;
};

struct ShaderCodeInfo
{
  OpalShaderType type;
  uint32_t size;
  uint32_t capacity;
  char* buffer;
  OpalShader shader;
};

struct ImageReimportInfo
{
  MsInputSet* set;
  uint32_t argumentIndex;
  char pathBuffer[1024];
};

struct MatSandboxState
{
  MsWindow window;
  char serialLoadPath[1024];

  struct
  {
    bool previewFocused;
    bool previewHovered;
  } inputState;

  Vec2I sceneGuiExtentsPrevFrame;
  OpalImage sceneImage;
  OpalImage depthImage;
  OpalImage renderBufferImage;

  OpalRenderpass sceneRenderpass;
  OpalFramebuffer sceneFramebuffer;

  uint32_t uiImageImportFlags;
  OpalInputLayout uiSingleImageInputLayout;
  OpalInputSet uiSceneImageInputSet;
  OpalRenderpass uiRenderpass;
  OpalFramebuffer uiFramebuffer;

  uint32_t meshIndex;
  OpalMesh meshes[5];

  uint32_t shaderCount;
  OpalMaterial material;
  MsInputSet materialInputSet;
  ShaderCodeInfo* pShaderCodeInfos;

  MsInputSet globalInputSet;

  uint32_t shaderCompileQueueLength;
  ShaderCodeInfo* pShaderCompileQueue[Opal_Shader_COUNT];
  uint32_t imageReimportQueueLength;
  ImageReimportInfo pImageReimportQueue[10];

  struct
  {
    float* time;
    Vec2U* viewportExtents;
    Mat4* camView;
    Mat4* camProj;
    Vec3* camForward;
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
