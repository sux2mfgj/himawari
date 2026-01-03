# Import Rust module (stable since Meson 1.0.0)
rust = import('rust')

# Generate bindings using Meson's native rust.bindgen()
mm_bindings = rust.bindgen(
  input: files('../../../inc/hm/mm.h'),
  output: 'bindings.rs',
  include_directories: include_directories('../../../inc'),
  args: [
    '--use-core',
    '--ctypes-prefix', '::core::ffi',
    '--allowlist-function', 'mm_alloc',
    '--allowlist-function', 'mm_early_init',
    '--allowlist-function', 'mm_init',
    '--allowlist-var', 'PAGE_SIZE',
  ],
)

bindings_mm = static_library('bindings_mm',
  structured_sources(
    'lib.rs',
    {'.': mm_bindings}
  ),
  rust_abi: 'rust'
)
