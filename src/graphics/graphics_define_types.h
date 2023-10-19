
#ifndef MATSANDBOX_GRAPHICS_DEFINE_TYPES_H
#define MATSANDBOX_GRAPHICS_DEFINE_TYPES_H

#include "src/defines.h"

#include <vulkan/vulkan.h>

enum MsgFormat
{
  Msg_Format_Undefined,

  Msg_Format_R8,
  Msg_Format_RG8,
  Msg_Format_RGB8,
  Msg_Format_RGBA8,
  Msg_Format_R32,
  Msg_Format_RG32,
  Msg_Format_RGB32,
  Msg_Format_RGBA32,

  Msg_Format_R8_Int,
  Msg_Format_RG8_Int,
  Msg_Format_RGB8_Int,
  Msg_Format_RGBA8_Int,
  Msg_Format_R32_Int,
  Msg_Format_RG32_Int,
  Msg_Format_RGB32_Int,
  Msg_Format_RGBA32_Int,

  Msg_Format_R8_Uint,
  Msg_Format_RG8_Uint,
  Msg_Format_RGB8_Uint,
  Msg_Format_RGBA8_Uint,
  Msg_Format_R32_Uint,
  Msg_Format_RG32_Uint,
  Msg_Format_RGB32_Uint,
  Msg_Format_RGBA32_Uint,

  Msg_Format_R8_Nonlinear,
  Msg_Format_RG8_Nonlinear,
  Msg_Format_RGB8_Nonlinear,
  Msg_Format_RGBA8_Nonlinear,
  Msg_Format_R32_Nonlinear,
  Msg_Format_RG32_Nonlinear,
  Msg_Format_RGB32_Nonlinear,
  Msg_Format_RGBA32_Nonlinear,

  Msg_Format_D16_S8_Uint,
  Msg_Format_D24_S8_Uint,
  Msg_Format_D32_S8_Uint,
  Msg_Format_D32,

  Msg_Format_Mat4x4,

  Msg_Format_Count
};

#endif // !MATSANDBOX_GRAPHICS_DEFINE_TYPES_H
