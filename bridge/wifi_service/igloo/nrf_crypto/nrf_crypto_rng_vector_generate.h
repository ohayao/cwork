#ifndef _NRF_CRYPTO_RNG_VECTOR_GENERATE_H_
#define _NRF_CRYPTO_RNG_VECTOR_GENERATE_H_
#include <stdint.h>
#include <stdlib.h>

uint32_t nrf_crypto_rng_vector_generate(uint8_t * p_target, uint32_t length);

#endif