
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
  OpalInputLayout layout;
  OpalInputSet set;

  uint32_t nextArgumentId = 0;
  std::vector<MsbInputArgument> pArguments;

public:
  MsbInputSet() { layout = OPAL_NULL_HANDLE; set = OPAL_NULL_HANDLE; }

  MsbResult Init(std::vector<MsbInputArgumentInitInfo>& pInitInfo);
  void Shutdown();

  OpalInputLayout GetLayout() { return layout; };
  OpalInputSet GetSet() { return set; };

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
