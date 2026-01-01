# Generate C header files from Rust code using cbindgen

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
  gen_header = custom_target(
    out_file.underscorify(),
    output: out_file,
    input: rs_src[0],  # Only pass lib.rs to cbindgen
    command: [
      cbindgen_prog,
      '--lang', 'c',
      '--output', meson.project_source_root() / 'inc' / out_path,
      '@INPUT@'
    ],
    build_by_default: true,
  )
  generated_headers += gen_header
endforeach
