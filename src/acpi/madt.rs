#![no_std]

#[macro_use]
extern crate print_rs;
extern crate runtime_rs;

#[repr(C, packed)]
struct MadtHeader {
    signature: [u8; 4],
    length: u32,
    revision: u8,
    checksum: u8,
    oemid: [u8; 6],
    oemtableid: u64,
    oemrev: u32,
    creator_id: u32,
    creator_rev: u32,
    local_apic_address: u32,
    flags: u32,
}

#[repr(C, packed)]
struct MadtInterruptDevice {
    entry_type: u8,
    record_length: u8,
}

use core::iter::Iterator;

struct MadtIterator {}

impl Iterator for MadtIterator {}

impl MadtHeader {
    unsafe fn from_ptr<'a>(ptr: *const core::ffi::c_void) -> Option<&'a Self> {
        if ptr.is_null() {
            return None;
        }

        (ptr as *const Self).as_ref()
    }

    fn length(&self) -> u32 {
        unsafe { core::ptr::addr_of!(self.length).read_unaligned() }
    }

    fn has_8259_pic(&self) -> bool {
        unsafe { core::ptr::addr_of!(self.flags).read_unaligned() == 1 }
    }

    fn local_apic_address(&self) -> u32 {
        unsafe { core::ptr::addr_of!(self.local_apic_address).read_unaligned() }
    }
}

#[no_mangle]
pub extern "C" fn acpi_table_parse_madt(hdr: *const core::ffi::c_void) -> i32 {
    kprintln!("MADT detected");
    unsafe {
        if let Some(madt_hdr) = MadtHeader::from_ptr(hdr) {
            kprintln!("madt: length {}", madt_hdr.length());
            kprintln!("Local APIC address: 0x{:x}", madt_hdr.local_apic_address());

            if madt_hdr.has_8259_pic() {
                kprintln!("Legacy PIC exists");
            } else {
                kprintln!("No legacy PIC")
            }
        }
    }

    0
}
