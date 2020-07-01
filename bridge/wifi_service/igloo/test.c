#include "igloo.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int ret = 0;
  uint8_t *data_out = NULL; 
  uint32_t data_out_len = 100;
  uint32_t bytes_written = 0;
  size_t step1_len = 0;
  uint8_t *step1_payload_bytes = NULL;
  
  step1_payload_bytes = malloc(step1_len);
  memset(step1_payload_bytes, 0, step1_len);
  data_out = malloc(data_out_len);
  ret = ig_pairing_step2(step1_payload_bytes, step1_len, data_out, data_out_len, &bytes_written);
}