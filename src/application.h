
#ifndef MAT_SANDBOX_APPLICATION_H
#define MAT_SANDBOX_APPLICATION_H

#include "src/definesNew.h"
#include "src/input_set.h"

struct MsbApplication
{
private:
  MsbWindow window;

  MsbSceneRenderResources sceneRenderResources;
  MsbUiRenderResources uiRenderResources;

  MsbInputSet globalSet;
  MsbInputSet customSet;

public:
  MsbResult Run();

private:
  MsbResult Init();
  MsbResult Update();
  void Shutdown();

  MsbResult Resize(Vec2U newExtents);
  MsbResult RenderScene();
  MsbResult RenderUi();
  MsbResult HandleHidInput();
};

#endif // !MAT_SANDBOX_APPLICATION_H
