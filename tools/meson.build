# Generate C header files from Rust code using cbindgen

fs = import('fs')
cbindgen_prog = find_program('cbindgen', required: true, native: true)
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

  # Create output directory if needed
  if out_dir != ''
    run_command('mkdir', '-p', meson.project_source_root() / 'inc' / out_dir, check: false)
  endif

  # cbindgen only needs the crate root (lib.rs), which is the first file
  # Check if cbindgen.toml exists in the same directory as lib.rs
  lib_rs_path = rs_src[0]
  lib_rs_dir = fs.parent(lib_rs_path)
  cbindgen_config = lib_rs_dir / 'cbindgen.toml'

  # Build cbindgen command
  cbindgen_cmd = [
    cbindgen_prog,
    '--lang', 'c',
    '--output', meson.project_source_root() / 'inc' / out_path,
  ]

  # Add config if it exists
  if fs.exists(cbindgen_config)
    cbindgen_cmd += ['--config', cbindgen_config]
  endif

  cbindgen_cmd += ['@INPUT@']

  gen_header = custom_target(
    out_file.underscorify(),
    output: out_file,
    input: rs_src[0],  # Only pass lib.rs to cbindgen
    command: cbindgen_cmd,
    build_by_default: true,
  )
  generated_headers += gen_header
endforeach
