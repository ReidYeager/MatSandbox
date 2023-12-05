
#ifndef MAT_SANDBOX_APPLICATION_H
#define MAT_SANDBOX_APPLICATION_H

#include "src/definesNew.h"
#include "src/input_set.h"
#include "src/material.h"
#include "src/camera.h"
#include "src/ui.h"

extern void LapisInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct MsbApplication
{
private:
  MsbWindow m_window;

  MsbSceneRenderResources m_sceneRenderResources;
  MsbUiRenderResources m_uiRenderResources;

  MsbInputSet m_globalSet;
  MsbInputArgumentBuffer* m_globalBuffer;
  MsbInputSet m_customSet;
  MsbMaterial m_customMaterial;

  std::vector<OpalMesh> m_meshes;
  MsbCamera m_camera;

  MsbUi m_ui;

  float m_deltaTime;

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

  void UpdateTime();
  MsbResult ResizeUiRenderResources();

  MsbResult HandleHidInput();

  // Shutdown
  // ===============
  void Shutdown();
};

#endif // !MAT_SANDBOX_APPLICATION_H
