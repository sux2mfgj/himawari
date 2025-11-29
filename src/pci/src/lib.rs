#[macro_use]
extern crate print_rs;

#[no_mangle]
pub extern "C" fn pci_register_ecam(
    _base_addr: *const core::ffi::c_void,
    _start_bus: u8,
    _end_bus: u8,
) -> i32 {
    kprintf!("PCI ECAM registered\n");
    // TODO: Implement PCI bus scanning
    0
}

#[cfg(test)]
mod tests {
    use super::*;

    //#[test]
    //fn it_works() {}
}
