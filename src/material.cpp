
#include "src/definesNew.h"
#include "src/material.h"

MsbResult MsbMaterial::Init(MsbMaterialInitInfo initInfo)
{
  m_renderpass = initInfo.renderpass;
  m_subpassIndex = initInfo.subpassIndex;

  std::vector<OpalShader> pShaders;
  std::vector<OpalInputLayout> pLayouts;

  // Create shaders
  // ===============
  for (uint32_t i = 0; i < initInfo.pShaderInfos.size(); i++)
  {
    MsbShader newShader;
    MSB_ATTEMPT(newShader.Init(initInfo.pShaderInfos[i]));

    this->m_shaders.push_back(newShader);
    pShaders.push_back(newShader.GetOpal());
  }

  m_inputSets = initInfo.pInputSets;
  for (uint32_t i = 0; i < initInfo.pInputSets.size(); i++)
  {
    pLayouts.push_back(initInfo.pInputSets[i]->GetLayout());
  }

  // Create material
  // ===============
  OpalMaterialInitInfo matInfo = { 0 };
  matInfo.inputLayoutCount = pLayouts.size();
  matInfo.pInputLayouts = pLayouts.data();
  matInfo.shaderCount = pShaders.size();
  matInfo.pShaders = pShaders.data();
  matInfo.pushConstantSize = 0;
  matInfo.renderpass = m_renderpass;
  matInfo.subpassIndex = m_subpassIndex;
  MSB_ATTEMPT_OPAL(OpalMaterialInit(&m_opal, matInfo));

  MSB_LOG("Material init success\n");

  return Msb_Success;
}

MsbResult MsbMaterial::Use()
{
  
  return Msb_Success;
}

void MsbMaterial::Shutdown()
{
  
}

// ===============
// Arguments
// ===============

MsbResult MsbMaterial::AddArgument(MsbInputArgumentInitInfo* initInfo)
{
  
  return Msb_Success;
}

MsbResult MsbMaterial::RemoveArgument(uint32_t index)
{
  
  return Msb_Success;
}

