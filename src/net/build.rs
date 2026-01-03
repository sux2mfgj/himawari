use std::env;
use std::path::PathBuf;

fn main() {
    // Cargo ビルド時であることを示すフラグを設定
    // Meson ビルド時はこの build.rs は実行されない
    println!("cargo::rustc-check-cfg=cfg(cargo_build)");
    println!("cargo:rustc-cfg=cargo_build");

    // テスト時のみ bindgen を実行
    println!("cargo:rerun-if-changed=../../inc/hm/net_if.h");

    let bindings = bindgen::Builder::default()
        .header("../../inc/hm/net_if.h")
        .clang_arg("-I../../inc")
        .use_core()
        .ctypes_prefix("core::ffi")
        .allowlist_type("net_if")
        .allowlist_type("mac_addr_t")
        .allowlist_type("ipv4_addr_t")
        .allowlist_function("netif_.*")
        .layout_tests(false)
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
