# Vendored heatshrink encoder

This directory contains the encoder and license from the upstream
`v0.4.1` tag (`b9ac05e`) of
<https://github.com/atomicobject/heatshrink>.

Only the encoder is required. Marlin provides the matching decoder and
announces its window and lookahead parameters during Binary File Transfer
capability negotiation.

The encoder includes the finalization fix from upstream commit `3b85e98`
(atomicobject/heatshrink#87), applied separately from the pristine import.
