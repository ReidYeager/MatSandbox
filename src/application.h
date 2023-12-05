
#ifndef MAT_SANDBOX_APPLICATION_H
#define MAT_SANDBOX_APPLICATION_H

#include "src/definesNew.h"
#include "src/input_set.h"
#include "src/material.h"
#include "src/camera.h"
#include "src/ui.h"

struct MsbApplication
{
private:
  MsbWindow window;

  MsbSceneRenderResources sceneRenderResources;
  MsbUiRenderResources uiRenderResources;

  MsbInputSet globalSet;
  MsbInputArgumentBuffer* globalBuffer;
  MsbInputSet customSet;
  MsbMaterial customMaterial;

  std::vector<OpalMesh> pMeshes;
  MsbCamera camera;

  MsbUi ui;

public:
  MsbResult Run();

private:
  // Init
  // ===============
  MsbResult Init();

  MsbResult InitWindow();
  MsbResult InitGraphics();
  MsbResult InitSceneRenderResources();
  MsbResult InitUiRenderResources();
  MsbResult InitCustomMaterial();
  MsbResult InitUi();

  MsbResult LoadMesh(char* path);

  // Update
  // ===============
  MsbResult Update();

  MsbResult Resize(Vec2U newExtents);
  MsbResult RenderScene();
  MsbResult RenderUi();
  MsbResult HandleHidInput();

  // Shutdown
  // ===============
  void Shutdown();
};

#endif // !MAT_SANDBOX_APPLICATION_H
