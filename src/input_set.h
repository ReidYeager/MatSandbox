
#ifndef MAT_SANDBOX_ARGUMENTS_H
#define MAT_SANDBOX_ARGUMENTS_H

#include "src/definesNew.h"
#include <string>
#include <vector>

enum MsbInputArgumentType
{
  Msb_Argument_Buffer,
  Msb_Argument_Image,
  Msb_Argument_COUNT
};

enum MsbBufferElementType
{
  Msb_Buffer_Int,
  Msb_Buffer_Int2,
  Msb_Buffer_Int3,
  Msb_Buffer_Int4,

  Msb_Buffer_Uint,
  Msb_Buffer_Uint2,
  Msb_Buffer_Uint3,
  Msb_Buffer_Uint4,

  Msb_Buffer_Float,
  Msb_Buffer_Float2,
  Msb_Buffer_Float3,
  Msb_Buffer_Float4,

  Msb_Buffer_Double,
  Msb_Buffer_Double2,
  Msb_Buffer_Double3,
  Msb_Buffer_Double4,

  Msb_Buffer_Mat4,

  Msb_Buffer_COUNT
};

static const char* MsbBufferElementTypeNames[] =
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
  "Mat4",
  "Invalid"
};

struct MsbBufferElement
{
  std::string name;
  MsbBufferElementType type;
  void* data;
};

struct MsbInputArgumentBuffer
{
  OpalBuffer opal;
  uint32_t size;

  uint32_t elementCount;
  MsbBufferElement* pElements;
};

struct MsbInputArgumentImage
{
  OpalImage opal;
  OpalInputSet set;
  char* sourcePath;
};

union MsbInputArgumentData
{
  MsbInputArgumentBuffer buffer;
  MsbInputArgumentImage image;
};

struct MsbInputArgument
{
  uint32_t id;
  std::string name;

  MsbInputArgumentType type;
  MsbInputArgumentData data;
};

struct MsbInputArgumentInitInfo
{
  MsbInputArgumentType type;

  union
  {
    struct
    {
      uint32_t elementCount;
      MsbBufferElementType* pElementTypes;
    } buffer;
    struct
    {
      char* imagePath;
      void* imageData;
      Vec2U extents;
    } image;
  };
};

class MsbInputSet
{
private:
  OpalInputLayout m_layout;
  OpalInputSet m_set;

  std::string m_name;
  uint32_t m_nextArgumentId = 0;
  std::vector<MsbInputArgument> m_arguments;

public:
  MsbInputSet() { m_layout = OPAL_NULL_HANDLE; m_set = OPAL_NULL_HANDLE; }

  MsbResult Init(std::string name, std::vector<MsbInputArgumentInitInfo>& pInitInfo);
  void Shutdown();

  std::string GetName() { return m_name; }
  OpalInputLayout GetLayout() { return m_layout; }
  OpalInputSet GetSet() { return m_set; };

  uint32_t GetArgumentCount() { return m_arguments.size(); }
  MsbInputArgument* GetArgument(uint32_t index) { return &m_arguments[index]; }

  MsbResult UpdateLayoutAndSet();
  MsbResult PushBuffersData();

  MsbResult AddArgument(MsbInputArgumentInitInfo* initInfo);
  MsbResult InitBufferArgument(MsbInputArgumentInitInfo* initInfo, MsbInputArgument* outArg);
  MsbResult InitImageArgument(MsbInputArgumentInitInfo* initInfo, MsbInputArgument* outArg);

  MsbResult RemoveArgument(uint32_t index);
  MsbResult ShutdownBufferArgument(MsbInputArgument* argument);
  MsbResult ShutdownImageArgument(MsbInputArgument* argument);
};

#endif // !MAT_SANDBOX_ARGUMENTS_H
