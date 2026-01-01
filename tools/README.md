# Build Tools

This directory contains build-time utilities for the Himawari kernel.

## Header Generation (`meson.build`)

Automatically generates C header files from Rust FFI functions using `cbindgen`.

### How it works

1. Source modules populate the `rs_hdr` array in their `meson.build` files:
   ```meson
   rs_hdr += [['hm/net.h', rssrc]]
   ```
   where `rssrc` is an array of Rust source files (e.g., `[lib.rs, ethernet.rs, arp.rs]`)

2. After all source files are processed, `tools/meson.build` runs `cbindgen` on each entry

3. C headers are generated to `inc/` directory (e.g., `inc/hm/net.h`)

### Adding a new header

To generate a C header for a new Rust module:

1. In your module's `meson.build`, add Rust sources to `rssrc`:
   ```meson
   rssrc = []
   subdir('src')  # This should populate rssrc with lib.rs, etc.
   ```

2. Register the header path:
   ```meson
   rs_hdr += [['hm/your_module.h', rssrc]]
   ```

3. Build the kernel - the header will be auto-generated at `inc/hm/your_module.h`

### Notes

- `cbindgen` only processes the crate root (`lib.rs`), which should be the first file in `rssrc`
- Entries with empty `rssrc` arrays are automatically skipped
- Headers are regenerated whenever the Rust source changes
- Use `#[no_mangle]` and `pub extern "C"` in Rust for FFI functions
