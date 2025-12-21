#![no_std]

use core::ptr;

#[macro_use]
extern crate print_rs;
extern crate runtime_rs;

const PCI_DEV_NUM_MAX: usize = 128;
const PCI_DEV_MAX: u8 = 32;

#[derive(Clone, Copy)]
struct PciDevice {
    cfg_space: *const u8,
}

static mut PCI_DEVICES: [Option<PciDevice>; PCI_DEV_NUM_MAX] = [None; PCI_DEV_NUM_MAX];
static mut PCI_DEV_NUM: usize = 0;

/// Register a PCI device
fn pci_register_device(config_space: *const u8) -> Result<(), i32> {
    unsafe {
        if PCI_DEV_NUM >= PCI_DEV_NUM_MAX {
            return Err(-1);
        }

        PCI_DEVICES[PCI_DEV_NUM] = Some(PciDevice {
            cfg_space: config_space,
        });

        PCI_DEV_NUM += 1;
    }

    Ok(())
}

/// Scan a single PCI device at the specified bus and device number
fn pci_scan_device(base: *const u8, bus: u8, dev: u8) -> Result<(), i32> {
    // ECAM address calculation: base | (bus << 20) | (dev << 15)
    let offset = ((bus as usize) << 20) | ((dev as usize) << 15);
    let addr = unsafe { base.add(offset) };

    // Read vendor ID and device ID
    let vendor_id = unsafe { ptr::read_volatile(addr as *const u16) };
    let device_id = unsafe { ptr::read_volatile(addr.add(2) as *const u16) };

    // Vendor ID 0xffff indicates no device present
    if vendor_id == 0xffff {
        return Ok(());
    }

    kprintln!(
        "Found pci device({}:{}) : vendor {:#x}, device {:#x}",
        bus,
        dev,
        vendor_id,
        device_id
    );

    // This is a virtio net device
    if vendor_id == 0x1af4 && device_id == 0x1000 {
        vnet_init(base);
    }

    // TODO: check the device type and etc.

    Ok(())
}

/// Scan all devices on the specified range of buses
fn pci_scan_bus(base: *const u8, start_bus: u8, end_bus: u8) -> Result<(), i32> {
    for bus in start_bus..=end_bus {
        for dev in 0..PCI_DEV_MAX {
            pci_scan_device(base, bus, dev).map_err(|ret| {
                kprintln!("failed to scan device: bus {}, dev {}", bus, dev);
                ret
            })?;
        }
    }

    Ok(())
}

#[no_mangle]
pub extern "C" fn pci_register_ecam(
    base_addr: *const core::ffi::c_void,
    start_bus: u8,
    end_bus: u8,
) -> i32 {
    kprintln!("PCI ECAM registered");

    let base = base_addr as *const u8;

    match pci_scan_bus(base, start_bus, end_bus) {
        Ok(()) => 0,
        Err(ret) => ret,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    //#[test]
    //fn it_works() {}
}
