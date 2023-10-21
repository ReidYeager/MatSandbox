
#ifndef MATSANDBOX_DEFINES_H
#define MATSANDBOX_DEFINES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum MsResult
{
  Ms_Success,
  Ms_Fail,
  Ms_Fail_Invalid_Field,
  MS_Fail_External
};

#define MS_ATTEMPT(fn)      \
{                           \
  MsResult result = (fn);   \
  if (result != Ms_Success) \
  {                         \
    return Ms_Fail;         \
  }                         \
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define MSB_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif


#endif // !MATSANDBOX_DEFINES_H
