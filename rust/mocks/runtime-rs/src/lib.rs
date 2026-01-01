//! Mock implementation of runtime-rs for testing
//!
//! This is a test-only mock that provides the same interface as the real runtime_rs,
//! but uses standard library features instead of kernel runtime.
//!
//! Note: panic_handler, abort, and rust_eh_personality are NOT included in this mock,
//! as the standard library already provides these in test environments.

/// Byte comparison function (bcmp)
///
/// Mock implementation that uses standard library comparison
pub unsafe fn bcmp(s1: *const u8, s2: *const u8, n: usize) -> i32 {
    if s1.is_null() || s2.is_null() {
        return 1;
    }

    let slice1 = core::slice::from_raw_parts(s1, n);
    let slice2 = core::slice::from_raw_parts(s2, n);

    if slice1 == slice2 {
        0
    } else {
        1
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bcmp_equal() {
        let a = [1u8, 2, 3, 4];
        let b = [1u8, 2, 3, 4];
        unsafe {
            assert_eq!(bcmp(a.as_ptr(), b.as_ptr(), 4), 0);
        }
    }

    #[test]
    fn test_bcmp_not_equal() {
        let a = [1u8, 2, 3, 4];
        let b = [1u8, 2, 3, 5];
        unsafe {
            assert_eq!(bcmp(a.as_ptr(), b.as_ptr(), 4), 1);
        }
    }

    #[test]
    fn test_bcmp_null() {
        let a = [1u8, 2, 3, 4];
        unsafe {
            assert_eq!(bcmp(core::ptr::null(), a.as_ptr(), 4), 1);
            assert_eq!(bcmp(a.as_ptr(), core::ptr::null(), 4), 1);
        }
    }
}
