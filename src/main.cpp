
#include "src/common.h"

#include "src/application.h"

uint32_t attemptDepth = 0;
OpalInputLayout msbSingleImageLayout;

MatSandboxState state;

int main(void)
{
  //MSB_ATTEMPT(MsInit());
  //MSB_ATTEMPT(MsUpdate());
  //MsShutdown();

  MsbApplication a;
  a.Run();
  return 0;
}
