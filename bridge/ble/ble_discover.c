#include <bridge/ble/ble_discover.h>
#include <regex.h>
#include <gattlib/include/gattlib.h>


void ble_discovered_device(
  void *adapter, const char* addr, const char* name, void *user_data) {
  // 通过 user_data, 传入一个数据结构, 然后返回结果
	int ret;
	int addr_size;
	char *up_addr;
	int name_size;

	if (name) {
		// IGM303e31a5c
    // regex IGM_regex("^IGM.*?",  std::regex_constants::icase);
		
		// regex IGM_regex("IGM303e31a5c",  std::regex_constants::icase);
    // if (regex_match(name, IGM_regex))
    // {
		// 	up_addr = strdup(addr);
		// 	addr_size = strlen(up_addr);
		// 	name_size = strlen(name);
    //   // printf("result Discovered %s - '%s '\n", addr, name);
    // }
    // else
    // {
    //   return;
    // }
    regex_t regex;
    int reti;
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return
    }
    reti = regexec(&regex, name, 0, NULL, 0);
    if (!reti) {
      puts("Match");
    }
    else
    {
      return;
    }
	} 
  else 
  {
		// printf("Discovered %s, don't have name, return\n", addr);
		return;
  }

	sp_near_list->push_front(igm_lock_t(addr_size, up_addr, name_size, name));
	return;
}
