#include "bridge/mqtt/base64.h"
#include "bridge/bridge_main/log.h"

int main(int argc, char *argv[]) {
    // Init(NULL);
    serverLog(LL_NOTICE,"test decode base64 Ready to start.");
    if (argc != 2) {
      serverLog(LL_NOTICE, "%s <base64_code> \n", argv[0]);
      return 1;
    }

    size_t output_len = 0;
    uint8_t *decode_data = NULL;
    decode_data = base64_decode(argv[1], strlen(argv[1]), &output_len);
    for (int j = 0; j <output_len; j++)
    {
      printf("%02x", decode_data[j]);
    }
    printf("\n");
    free(decode_data);
    decode_data = NULL;
    return 0;
}