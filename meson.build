project('himawari', 'c', 'rust', version: '0.1.0')

srcs = []
incs = []
libs = []
rlibs = []

# `rs_hdr` is used to generate C header files from Rust code.
# Source files add entries to this array in format: [['hm/net.h'], [lib.rs, ...]]
# Header generation is handled by tools/meson.build
rs_hdr = []

# Build Rust dependencies (bindings and runtime libraries)
subdir('rust')

# Process source files (populates rs_hdr array)
subdir('src')

# Generate C headers from Rust using cbindgen
subdir('tools')

# Include header directories
subdir('inc')

configure_file(
  input: 'linker.ld',
  output: 'linker.ld',
  copy: true
)

c_args = ['-nostdlib', '-ffreestanding', '-mno-mmx', '-mno-sse']
link_args = ['-nostdlib', '-Tlinker.ld', '-static', '-Wl,-e,_boot']

# Add build directory to include path for generated headers (e.g., build/hm/net.h)
build_incs = include_directories('.')

libs += static_library('rust', files('bridge.rs'), rust_abi: 'c', 
  rust_args: ['-C', 'panic=abort'],
  link_with: rlibs)

executable('himawari.elf', srcs, include_directories: [incs, build_incs], c_args: c_args,
  link_args: link_args, link_with: libs)
