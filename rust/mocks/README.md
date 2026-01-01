# Rust Test Mocks

このディレクトリには、Rust の単体テスト用のモッククレートが含まれています。

## 概要

Himawari カーネルのRustコードは `no_std` 環境で動作し、カーネル固有のランタイム（`print_rs`, `runtime_rs`）に依存しています。しかし、単体テスト時には標準ライブラリを使用できるため、カーネル依存を模倣するモックを提供します。

## ディレクトリ構造

```
rust/mocks/
├── README.md              # このファイル
├── print-rs/              # print_rs のモック
│   ├── Cargo.toml
│   └── src/
│       └── lib.rs         # kprint!, kprintln! マクロのモック実装
└── runtime-rs/            # runtime_rs のモック
    ├── Cargo.toml
    └── src/
        └── lib.rs         # bcmp などのランタイム関数のモック実装
```

## モッククレート

### print-rs

カーネルの `kprint!` と `kprintln!` マクロを提供します。

**本番環境:**
- C の `putsn()` FFI 関数を呼び出す
- `no_std` 環境で動作

**テスト環境（このモック）:**
- Rust の標準 `eprint!` / `eprintln!` にリダイレクト
- 標準ライブラリを使用

**提供するマクロ:**
```rust
kprint!("text");              // 改行なし
kprintln!("text");            // 改行あり
kprintln!("x={}", 42);        // フォーマット対応
```

### runtime-rs

カーネルのランタイムサポート関数を提供します。

**本番環境:**
- `panic_handler`: カーネルパニック処理
- `rust_eh_personality`: 例外処理パーソナリティ
- `bcmp()`: バイト比較関数
- `abort()`: 強制終了

**テスト環境（このモック）:**
- `panic_handler`, `rust_eh_personality`, `abort()`: 標準ライブラリが提供するため不要
- `bcmp()`: スライス比較を使ったモック実装を提供

**提供する関数:**
```rust
pub unsafe fn bcmp(s1: *const u8, s2: *const u8, n: usize) -> i32;
```

## 使い方

### 1. Cargo.toml に dev-dependencies を追加

テスト対象のクレートの `Cargo.toml` に以下を追加：

```toml
[dev-dependencies]
print-rs = { path = "../../rust/mocks/print-rs" }
runtime-rs = { path = "../../rust/mocks/runtime-rs" }
```

C FFI バインディング（例: `net_if` 構造体）が必要な場合は、`build.rs` で bindgen を使用してください。

### 2. lib.rs でテスト時のみモックを使用

```rust
#![no_std]

#[cfg(test)]
#[macro_use]
extern crate std;

// テスト時はモックを使用
#[cfg(test)]
#[macro_use]
extern crate print_rs;

// ... コード ...

#[cfg(test)]
mod tests {
    #[test]
    fn test_something() {
        kprintln!("Testing...");
        // テストコード
    }
}
```

### 3. テストを実行

```bash
cargo test
```

出力を確認する場合：
```bash
cargo test -- --nocapture
```

## 実装例: src/net

`src/net/` クレートは以下のようにモックと build.rs を使用しています：

**Cargo.toml:**
```toml
[build-dependencies]
bindgen = "0.70"

[dev-dependencies]
print-rs = { path = "../../rust/mocks/print-rs" }
runtime-rs = { path = "../../rust/mocks/runtime-rs" }
```

**build.rs:**
```rust
use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rerun-if-changed=../../inc/hm/net_if.h");

    let bindings = bindgen::Builder::default()
        .header("../../inc/hm/net_if.h")
        .clang_arg("-I../../inc")
        .use_core()
        .ctypes_prefix("core::ffi")
        .allowlist_type("net_if")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
```

**src/lib.rs:**
```rust
#![no_std]

#[cfg(test)]
#[macro_use]
extern crate std;

#[cfg(test)]
#[macro_use]
extern crate print_rs;

// Meson ビルド時は bindings_net クレートを使用
#[cfg(not(test))]
extern crate bindings_net;
#[cfg(not(test))]
use bindings_net::net_if;

// Cargo テスト時は build.rs で生成されたバインディングを使用
#[cfg(test)]
mod bindings {
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}
#[cfg(test)]
use bindings::net_if;

// テストで kprintln! と net_if を使用可能
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kprintln_macro() {
        kprintln!("Testing kprintln! macro");
        kprintln!("Formatted output: {} + {} = {}", 1, 2, 3);
    }
}
```

## テスト出力例

```bash
$ cargo test test_kprintln_macro -- --nocapture
running 1 test
Testing kprintln! macro
Formatted output: 1 + 2 = 3
test tests::test_kprintln_macro ... ok

test result: ok. 1 passed; 0 failed; 0 ignored; 0 measured; 4 filtered out
```

## モックのテスト

モッククレート自体もテストを含んでいます：

```bash
# print-rs モックのテスト
cd rust/mocks/print-rs
cargo test

# runtime-rs モックのテスト
cd rust/mocks/runtime-rs
cargo test
```

## 注意事項

### 1. dev-dependencies のみで使用

モックは **テスト時のみ** 使用され、本番ビルドには含まれません。`[dev-dependencies]` に記載することで、`cargo build` ではモックが使用されず、`cargo test` でのみ使用されます。

### 2. no_std とのトレードオフ

テスト環境では標準ライブラリが使用できるため：
- ✅ デバッグが容易（`println!` で出力確認）
- ✅ テストが高速（カーネルFFI不要）
- ⚠️ 実際のカーネル環境とは異なる動作の可能性

単体テストで動作を検証し、統合テストで実際のカーネル環境での動作を確認することを推奨します。

### 3. モックの制限

以下の機能はモックで提供されません：
- `panic_handler` （標準ライブラリが提供）
- `rust_eh_personality` （標準ライブラリが提供）
- `abort()` （標準ライブラリが提供）
- カーネル固有のデバイスI/O

これらが必要な場合は、統合テストまたは実機テストで検証してください。

## Meson ビルドとの関係

### 本番ビルド（Meson）

```bash
meson setup build
meson compile -C build
```

Meson ビルド時:
- `rust/bindings/net/` が使用される（Meson の `rust.bindgen()`）
- `link_with: [bindings_net]` で自動リンク
- モックは使用されない

### テストビルド（Cargo）

```bash
cargo test
```

Cargo テスト時:
- C FFI バインディングは `build.rs` で bindgen を使用して生成
- 同じヘッダーファイルから生成されるため、Meson ビルドと互換性が保証される
- モックによりカーネル依存なしでテスト可能

## まとめ

- ✅ `print-rs` モック: `kprint!`, `kprintln!` をテスト環境で使用可能
- ✅ `runtime-rs` モック: `bcmp()` などのランタイム関数を提供
- ✅ Meson ビルドとCargo テストの両方に対応
- ✅ 標準ライブラリを活用した簡易実装
- ✅ C FFI バインディングは各クレートの `build.rs` で生成（bindgen使用）

これにより、カーネルコードをCargo標準のテストフレームワークで容易にテストできます。
