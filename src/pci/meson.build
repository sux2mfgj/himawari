rssrc = []
subdir('src')

#libs += static_library(
#  'pci',
#  sources : rssrc,
#  rust_abi: 'c',
#  rust_args: ['-C', 'panic=abort'],
#  link_with: [print_rs, runtime_rs],
#)

rs_hdr += [['hm/pci.h', rssrc]]

srcs += files('pci.c')
srcs += files('msix.c')
