#!/bin/sh
# Helper script to generate C headers from Rust using cbindgen
# Usage: generate_header.sh <output_dummy> <real_output_path> <cbindgen_config> <input_crate>

OUTPUT_DUMMY="$1"
OUTPUT_REAL="$2"
CONFIG="$3"
INPUT="$4"

# Create output directory for the real file
mkdir -p "$(dirname "$OUTPUT_REAL")"

# Run cbindgen to the real location
if [ -n "$CONFIG" ] && [ -f "$CONFIG" ]; then
    cbindgen --lang c --config "$CONFIG" --output "$OUTPUT_REAL" "$INPUT"
else
    cbindgen --lang c --output "$OUTPUT_REAL" "$INPUT"
fi

# Create dummy output file so Meson can track it
touch "$OUTPUT_DUMMY"
