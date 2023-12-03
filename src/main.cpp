
#include "src/common.h"

#include "src/application.h"

uint32_t attemptDepth = 0;

MatSandboxState state;

int main(void)
{
  //MSB_ATTEMPT(MsInit());
  //MSB_ATTEMPT(MsUpdate());
  //MsShutdown();

  MsbApplication a;
  return (int)a.Run();
}
