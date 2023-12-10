
#include "src/common.h"

MatSandboxState state;
uint32_t attemptDepth;

int main(void)
{
  MSB_ATTEMPT(MsInit());
  MSB_ATTEMPT(MsUpdate());
  MsShutdown();

  return 0;
}
