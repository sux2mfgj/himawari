//! Mock implementation of print_rs for testing
//!
//! This is a test-only mock that provides the same interface as the real print_rs,
//! but uses standard library features instead of kernel FFI.

#![allow(unused_macros)]

/// Mock print macro that outputs to stderr
///
/// In tests, this simply delegates to eprintln!
#[macro_export]
macro_rules! kprint {
    ($($arg:tt)*) => {{
        eprint!($($arg)*);
    }};
}

/// Mock println macro that outputs to stderr with newline
///
/// In tests, this simply delegates to eprintln!
#[macro_export]
macro_rules! kprintln {
    () => { eprintln!() };
    ($($arg:tt)*) => {{
        eprintln!($($arg)*);
    }};
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_kprint() {
        kprint!("test");
    }

    #[test]
    fn test_kprintln() {
        kprintln!("test");
        kprintln!();
    }
}
