
#include "src/common.h"

MatSandboxState state;

int main(void)
{
  MS_ATTEMPT(MsInit());
  MS_ATTEMPT(MsUpdate());
  MsShutdown();

  return 0;
}
