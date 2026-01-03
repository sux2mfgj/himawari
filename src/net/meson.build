rssrc = []
subdir('src')

libs += static_library(
  'net',
  sources: rssrc,
  rust_abi: 'c',
  rust_args: ['-C', 'panic=abort'],
  link_with: [bindings_net, print_rs, runtime_rs, bindings_mm],
)

rs_hdr += [['hm/net.h', rssrc]]

srcs += files('net_if.c')
