# Rust runtime module for kernel
# Provides common panic handler and compiler builtins

runtime_rs = static_library('runtime_rs',
  'src/lib.rs',
  rust_abi: 'rust',
  rust_args: rust_args,
  link_with: print_rs,
)

rlibs += runtime_rs
