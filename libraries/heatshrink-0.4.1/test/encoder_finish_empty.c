#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "heatshrink_encoder.h"

int main(void) {
    heatshrink_encoder *encoder = heatshrink_encoder_alloc(8, 4);
    assert(encoder != NULL);

    uint8_t output[8];
    size_t output_size = sizeof(output);
    assert(heatshrink_encoder_finish(encoder) == HSER_FINISH_MORE);
    assert(heatshrink_encoder_poll(encoder, output, sizeof(output),
        &output_size) == HSER_POLL_EMPTY);
    assert(output_size == 0);
    assert(heatshrink_encoder_finish(encoder) == HSER_FINISH_DONE);

    heatshrink_encoder_free(encoder);
    return 0;
}
