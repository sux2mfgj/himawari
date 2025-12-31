#![no_std]

#[cfg(test)]
#[macro_use]
extern crate std;

#[macro_use]
extern crate print_rs;
extern crate runtime_rs;

mod ethernet;
use ethernet::{EthernetFrame, EthernetFrameType};

mod arp;
use arp::Arp;

#[no_mangle]
pub extern "C" fn handle_rx_packet(data: *const u8, len: u32) -> i32 {
    unsafe {
        if data.is_null() {
            return -1;
        }

        let slice = core::slice::from_raw_parts(data, len as usize);
        handle_mac_packet(slice)
    }
}

fn handle_mac_packet(packet: &[u8]) -> i32 {
    if packet.len() < 14 {
        kprintln!("Packet too short: {} bytes", packet.len());
        return -1;
    }

    // Check EtherType directly
    let ethertype_slice = &packet[12..14];

    match ethertype_slice {
        [0x08, 0x06] => {
            kprintln!("[Rust] Received ARP packet");
            let _arp = Arp::new(packet);
            0
        }
        [0x08, 0x00] => {
            kprintln!("[Rust] Received IPv4 packet");
            unimplemented!("IPv4 packet handling not yet implemented");
        }
        [0x86, 0xdd] => {
            kprintln!("[Rust] Received IPv6 packet (ignored)");
            0
        }
        _ => {
            kprintln!("[Rust] Unknown EtherType: {:02x}{:02x}",
                     ethertype_slice[0], ethertype_slice[1]);
            0
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn handle_test() {}
}

/*
 * himawari kernel -> (this stack)
 * test -> (this stack)
 * linux tap -> (this stack)
 * pcap -> (this stack)
 */
