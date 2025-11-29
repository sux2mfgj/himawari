subdir('bindings')

cbindgen_prog = find_program('cbindgen', required: true, native: true)

generated_headers = []

# rs_hdr format is expected like [['aa/test.h'], file(rssrc)]
foreach r : rs_hdr
  #message(r)
  out_path = r.get(0)
  rs_src = r.get(1)

  # パスからディレクトリとファイル名を分離
  path_parts = out_path.split('/')
  if path_parts.length() > 1
    # ディレクトリが含まれる場合
    out_dir = '/'.join(path_parts.get(0, -1))
    out_file = path_parts.get(-1)
  else
    # ファイル名のみの場合
    out_dir = ''
    out_file = out_path
  endif

  # ビルドディレクトリにサブディレクトリを作成
  if out_dir != ''
    run_command('mkdir', '-p', meson.current_build_dir() / out_dir, check: false)
  endif

  gen_header = custom_target(
    out_file.underscorify(),
    output: out_file,
    input: rs_src,
    command: [
      cbindgen_prog,
      '--lang', 'c',
      '--output', meson.build_root() / out_path,
      '@INPUT@'
    ],
	build_by_default: true,
)
  generated_headers += gen_header
endforeach
