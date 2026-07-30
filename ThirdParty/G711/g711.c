#include "g711.h"
#include "g711_ref.h"
#include <assert.h>

size_t g711_alaw_encode(const int16_t *pcm_data, size_t pcm_data_len,
                        uint8_t *encoded_data, size_t encoded_data_len) {

  if (encoded_data_len < pcm_data_len) {
    assert(0 && "encoded_data_len must be >= pcm_data_len");
    return 0;
  }

  for (size_t idx = 0; idx < pcm_data_len; ++idx) {
    encoded_data[idx] = linear_to_alaw(pcm_data[idx]);
  }

  return pcm_data_len;
}

size_t g711_alaw_decode(const uint8_t *encoded_data, size_t encoded_data_len,
                        int16_t *pcm_data, size_t pcm_data_len) {

  if (pcm_data_len < encoded_data_len) {
    assert(0 && "pcm_data_len must be >= encoded_data_len");
    return 0;
  }

  for (size_t idx = 0; idx < encoded_data_len; ++idx) {
    pcm_data[idx] = alaw_to_linear(encoded_data[idx]);
  }

  return encoded_data_len;
}