#include <bridge/bridge_main/sysinfo.h>
#include <string.h>

void sysinfoInit(sysinfo_t *system_info)
{
  memset(system_info, 0, sizeof(sysinfo_t));
}