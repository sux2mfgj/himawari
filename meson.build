project('himawari', 'c', version: '0.1.0')

srcs = []
incs = []

subdir('src')
subdir('inc')

configure_file(
  input: 'linker.ld',
  output: 'linker.ld',
  copy: true
)

c_args = ['-nostdlib', '-ffreestanding']
link_args = ['-nostdlib', '-Tlinker.ld', '-static', '-Wl,-e,_boot']

executable('himawari.elf', srcs, include_directories: incs, c_args: c_args, link_args: link_args)
