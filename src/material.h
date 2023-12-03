
#ifndef MAT_SANDBOX_MATERIAL_H
#define MAT_SANDBOX_MATERIAL_H

#include "src/definesNew.h"
#include "src/input_set.h"

#include <vector>

struct MsbMaterialInitInfo
{
  std::vector<MsbInputSet> pInputs;
};

class MsbMaterial
{
private:
  OpalMaterial opal;
  MsbInputSet inputSet;

public:
  MsbResult Init(MsbMaterialInitInfo info);
  MsbResult Use();
  void Shutdown();

  MsbResult AddArgument(MsbInputArgumentInitInfo* info);
  MsbResult RemoveArgument(uint32_t index);
};

#endif // !MAT_SANDBOX_MATERIAL_H
