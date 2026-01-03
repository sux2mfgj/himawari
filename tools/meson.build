# Generate C header files from Rust code using cbindgen

fs = import('fs')
generate_header_script = files('generate_header.sh')
generated_headers = []

# rs_hdr format is expected like [['hm/net.h'], [file1.rs, file2.rs, ...]]
# Each entry is [output_path, list_of_rust_source_files]
foreach r : rs_hdr
  out_path = r.get(0)
  rs_src = r.get(1)

  # Skip if no Rust source files
  if rs_src.length() == 0
    continue
  endif

  # Parse output path to extract directory and filename
  path_parts = out_path.split('/')
  if path_parts.length() > 1
    out_dir = '/'.join(path_parts.get(0, -1))
    out_file = path_parts.get(-1)
  else
    out_dir = ''
    out_file = out_path
  endif

  # cbindgen only needs the crate root (lib.rs), which is the first file
  # Check if cbindgen.toml exists in the same directory as lib.rs
  lib_rs_path = rs_src[0]
  lib_rs_dir = fs.parent(lib_rs_path)
  cbindgen_config = lib_rs_dir / 'cbindgen.toml'

  # Real output path in build root (e.g., build/hm/net.h)
  # We use meson.project_build_root() but construct a relative path
  build_root = meson.global_build_root()
  real_output_path = build_root / out_path

  # Dummy output for Meson tracking
  dummy_output = out_file

  # Build command using helper script
  # Usage: generate_header.sh <output_dummy> <real_output_path> <cbindgen_config> <input_crate>
  gen_cmd = [
    generate_header_script,
    '@OUTPUT@',
    real_output_path,
    fs.exists(cbindgen_config) ? cbindgen_config : '',
    '@INPUT@'
  ]

  # Generate header in build directory
  # Output must be just the filename for Meson tracking
  gen_header = custom_target(
    out_path.underscorify(),
    output: out_file,
    input: rs_src[0],  # Only pass lib.rs (crate root) to cbindgen
    command: gen_cmd,
    build_by_default: true,
  )
  generated_headers += gen_header
endforeach

# Add generated headers to sources so they're tracked as dependencies
srcs += generated_headers
