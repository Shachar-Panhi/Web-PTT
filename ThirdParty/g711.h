#ifndef G711_G711_H
#define G711_G711_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Usage:
 * - Provide non-NULL input/output buffers with valid element counts.
 * - Returns 0 if any buffer is empty or sizes are invalid; in debug builds
 *   this condition also triggers an assertion.
 * - On success, returns the number of elements encoded into encoded_data.
 */
size_t g711_alaw_encode(const int16_t *pcm_data, size_t pcm_data_len,
                        uint8_t *encoded_data, size_t encoded_data_len);

/* Usage:
 * - Provide non-NULL input/output buffers with valid element counts.
 * - Returns 0 if any buffer is empty or sizes are invalid; in debug builds
 *   this condition also triggers an assertion.
 * - On success, returns the number of elements decoded into pcm_data.
 */
size_t g711_alaw_decode(const uint8_t *encoded_data, size_t encoded_data_len,
                        int16_t *pcm_data, size_t pcm_data_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* G711_G711_H */
