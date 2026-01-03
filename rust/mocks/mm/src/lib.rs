// mm (メモリマネージャ) のテスト用モック
// std::alloc を使用してメモリを確保

use std::alloc::{alloc, Layout};

/// PAGE_SIZE 定数（カーネルと同じ値）
pub const PAGE_SIZE: u32 = 4096;

/// メモリを確保する（モック実装）
///
/// 本物の mm_alloc と同様、確保したメモリは解放されません。
/// テスト環境では std::alloc::alloc を使用します。
#[no_mangle]
pub unsafe fn mm_alloc(size: usize) -> *mut core::ffi::c_void {
    if size == 0 {
        return std::ptr::null_mut();
    }

    // アライメントは 8 バイト（または size に応じて調整）
    let align = if size >= 4096 { 4096 } else { 8 };

    let layout = match Layout::from_size_align(size, align) {
        Ok(layout) => layout,
        Err(_) => return std::ptr::null_mut(),
    };

    let ptr = alloc(layout);

    if ptr.is_null() {
        eprintln!("mm_alloc: failed to allocate {} bytes", size);
        return std::ptr::null_mut();
    }

    // デバッグ出力（テスト時に確認用）
    // eprintln!("mm_alloc: allocated {} bytes at {:?}", size, ptr);

    ptr as *mut core::ffi::c_void
}

/// mm_init のダミー実装（テストでは使用されない）
#[no_mangle]
pub unsafe fn mm_init(_entries: *mut core::ffi::c_void, _nentries: usize) -> i32 {
    0 // 成功を返す
}

/// mm_early_init のダミー実装（テストでは使用されない）
#[no_mangle]
pub unsafe fn mm_early_init() -> i32 {
    0 // 成功を返す
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mm_alloc() {
        unsafe {
            // 小さいサイズ
            let ptr1 = mm_alloc(64);
            assert!(!ptr1.is_null());

            // PAGE_SIZE
            let ptr2 = mm_alloc(PAGE_SIZE as usize);
            assert!(!ptr2.is_null());

            // 大きいサイズ
            let ptr3 = mm_alloc(8192);
            assert!(!ptr3.is_null());

            // ゼロサイズ
            let ptr4 = mm_alloc(0);
            assert!(ptr4.is_null());
        }
    }

    #[test]
    fn test_mm_alloc_write() {
        unsafe {
            let ptr = mm_alloc(28) as *mut u8;
            assert!(!ptr.is_null());

            // 書き込みテスト
            for i in 0..28 {
                *ptr.add(i) = i as u8;
            }

            // 読み取りテスト
            for i in 0..28 {
                assert_eq!(*ptr.add(i), i as u8);
            }
        }
    }
}
