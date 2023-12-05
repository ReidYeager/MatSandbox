
#ifndef MAT_SANDBOX_MATERIAL_H
#define MAT_SANDBOX_MATERIAL_H

#include "src/definesNew.h"
#include "src/input_set.h"
#include "src/shader.h"

#include <vector>

struct MsbMaterialInitInfo
{
  std::vector<MsbShaderInitInfo> pShaderInfos;
  std::vector<MsbInputSet*> pInputSets;

  OpalRenderpass renderpass;
  uint32_t subpassIndex = 0;
};

class MsbMaterial
{
private:
  OpalMaterial m_opal;

  OpalRenderpass m_renderpass;
  uint32_t m_subpassIndex;

  std::vector<MsbShader> m_shaders;
  std::vector<MsbInputSet*> m_inputSets;

public:
  MsbResult Init(MsbMaterialInitInfo initInfo);
  MsbResult Use();
  void Shutdown();

  MsbResult AddArgument(MsbInputArgumentInitInfo* initInfo);
  MsbResult RemoveArgument(uint32_t index);
};

#endif // !MAT_SANDBOX_MATERIAL_H
