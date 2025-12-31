rssrc = []
subdir('src')

libs += static_library(
  'net',
  sources: rssrc,
  rust_abi: 'c',
  rust_args: ['-C', 'panic=abort'],
  link_with: [print_rs, runtime_rs],
)

rs_hdr += [['hm/net.h', rssrc]]
