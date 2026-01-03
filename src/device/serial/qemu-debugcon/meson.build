# QEMU debugcon serial output driver (Rust implementation)
qemu_debugcon_src = files('qemu-debugcon.rs')

qemu_debugcon_lib = static_library(
  'qemu_debugcon',
  sources: qemu_debugcon_src,
  rust_abi: 'rust',
  rust_args: ['-C', 'panic=abort'],
)

rlibs += qemu_debugcon_lib

rs_hdr += [['hm/qemu-debugcon.h', qemu_debugcon_src]]
