project('himawari', 'c', 'rust', version: '0.1.0')

srcs = []
incs = []
libs = []

# `rs_hdr` is used to generate C header file from Rust code.
# More details, see the rust/meson.build.
rs_hdr = []

subdir('rust')
subdir('src')
subdir('inc')


configure_file(
  input: 'linker.ld',
  output: 'linker.ld',
  copy: true
)

c_args = ['-nostdlib', '-ffreestanding']
link_args = ['-nostdlib', '-Tlinker.ld', '-static', '-Wl,-e,_boot']

executable('himawari.elf', srcs + generated_headers, include_directories: incs, c_args: c_args, link_args: link_args,
  link_with: libs)
