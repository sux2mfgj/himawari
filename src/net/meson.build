rssrc = []
subdir('src')

rlibs += static_library(
  'net',
  sources: rssrc,
  rust_abi: 'rust',
  rust_args: rust_args,
  link_with: [bindings_net, print_rs, runtime_rs, bindings_mm],
)

rs_hdr += [['hm/net.h', rssrc]]

srcs += files('net_if.c')
srcs += files('packet.c')
