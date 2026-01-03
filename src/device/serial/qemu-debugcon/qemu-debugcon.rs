#![no_std]

const DEBUGCON_PORT: u16 = 0xe9;

/// Output a single character to QEMU debugcon port (0xe9)
#[no_mangle]
pub extern "C" fn qemu_debugcon_putc(c: u8) {
    unsafe {
        core::arch::asm!(
            "out dx, al",
            in("al") c,
            in("dx") DEBUGCON_PORT,
            options(nomem, nostack, preserves_flags)
        );
    }
}
