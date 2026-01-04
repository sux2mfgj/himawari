# Import Rust module (stable since Meson 1.0.0)
rust = import('rust')

# Generate bindings using Meson's native rust.bindgen()
print_bindings = rust.bindgen(
  input: files('../../../inc/hm/puts.h'),
  output: 'bindings.rs',
  include_directories: include_directories('../../../inc'),
  args: ['--use-core', '--ctypes-prefix', '::core::ffi'],
)

print_rs = static_library('print_rs',
  structured_sources(
    'lib.rs',
    {'.': print_bindings}
  ),
  rust_abi: 'rust',
  rust_args: rust_args,
)

rlibs += print_rs
