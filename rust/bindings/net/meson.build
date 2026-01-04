# Import Rust module (stable since Meson 1.0.0)
rust = import('rust')

# Generate bindings using Meson's native rust.bindgen()
# Using wrapper header to include both net_if.h and packet.h
net_bindings = rust.bindgen(
  input: files('bindings_wrapper.h'),
  output: 'bindings.rs',
  include_directories: include_directories('../../../inc'),
  args: [
    '--use-core',
    '--ctypes-prefix', '::core::ffi',
    # net_if.h bindings
    '--allowlist-type', 'net_if',
    '--allowlist-type', 'mac_addr_t',
    '--allowlist-type', 'ipv4_addr_t',
    '--allowlist-function', 'netif_.*',
    # packet.h bindings
    '--allowlist-type', 'packet_t',
    '--allowlist-function', 'packet_buf_alloc',
  ],
)

bindings_net = static_library('bindings_net',
  structured_sources(
    'lib.rs',
    {'.': net_bindings}
  ),
  rust_abi: 'rust',
  rust_args: rust_args,
)

rlibs += bindings_net
