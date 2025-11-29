#srcs += files('pci.c')

rssrc = []
subdir('src')

libs += static_library(
  'pci',
  sources : rssrc,
  rust_abi: 'c',
  link_with: print_rs,
)

rs_hdr += [['hm/pci.h', rssrc]]
