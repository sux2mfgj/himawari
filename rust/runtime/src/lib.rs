//! Himawari Kernel Rust Runtime
//!
//! This module provides common runtime support for all Rust code in the kernel,
//! including panic handling and compiler builtins.

#![no_std]

use core::panic::PanicInfo;

#[macro_use]
extern crate print_rs;

/// Panic handler for the kernel
///
/// When a panic occurs, this function prints the panic information
/// and then halts the CPU in an infinite loop.
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    kprintln!("\n!!! KERNEL PANIC !!!");

    if let Some(location) = info.location() {
        kprintln!(
            "panic occurred in file '{}' at line {}",
            location.file(),
            location.line(),
        );
    }

    kprintln!("message: {}", info.message());

    // Halt the CPU forever
    loop {
        unsafe {
            core::arch::asm!("hlt", options(nomem, nostack));
        }
    }
}

/// Exception handling personality function
///
/// Required by the compiler for panic unwinding support.
/// Since we use panic=abort, this is essentially a no-op.
#[no_mangle]
pub extern "C" fn rust_eh_personality() {}

/// Byte comparison function (bcmp)
///
/// This is a compiler builtin that compares two byte sequences.
/// We implement it by calling the C memcmp function.
#[no_mangle]
pub unsafe extern "C" fn bcmp(s1: *const u8, s2: *const u8, n: usize) -> i32 {
    extern "C" {
        fn memcmp(a: *const core::ffi::c_void, b: *const core::ffi::c_void, size: usize) -> bool;
    }

    if memcmp(
        s1 as *const core::ffi::c_void,
        s2 as *const core::ffi::c_void,
        n,
    ) {
        0
    } else {
        1
    }
}

/// Abort function
///
/// Called when an unrecoverable error occurs.
#[no_mangle]
pub extern "C" fn abort() -> ! {
    panic!("abort() called");
}
