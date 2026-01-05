#![no_std]

#[cfg(test)]
#[macro_use]
extern crate std;

#[macro_use]
extern crate print_rs;

// Meson ビルド時は print_rs と runtime_rs をリンク
#[cfg(not(cargo_build))]
extern crate runtime_rs;

// Meson ビルド時は bindings_net クレートを使用
#[cfg(not(cargo_build))]
extern crate bindings_net;
#[cfg(not(cargo_build))]
use bindings_net::net_if;

extern crate mm;

// Cargo ビルド時は build.rs で生成されたバインディングを使用
#[cfg(cargo_build)]
mod bindings {
    #![allow(non_upper_case_globals)]
    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}
#[cfg(cargo_build)]
use bindings::net_if;

mod ethernet;

mod arp;
use arp::handle_arp_packet;

mod ip;
use ip::handle_ip_packet;

mod icmp;

#[no_mangle]
pub extern "C" fn handle_rx_packet(nif: *mut net_if, data: *const u8, len: u32) -> i32 {
    unsafe {
        if data.is_null() || nif.is_null() {
            return -1;
        }

        let slice = core::slice::from_raw_parts(data, len as usize);
        handle_mac_packet(&mut *nif, slice)
    }
}

fn handle_mac_packet(nif: &mut net_if, packet: &[u8]) -> i32 {
    if packet.len() < 14 {
        //kprintln!("Packet too short: {} bytes", packet.len());
        return -1;
    }

    // Check EtherType directly
    let ethertype_slice = &packet[12..14];

    match ethertype_slice {
        [0x08, 0x06] => handle_arp_packet(nif, &packet[14..]),
        [0x08, 0x00] => handle_ip_packet(nif, &packet[14..]),
        [0x86, 0xdd] => 0,
        _ => 0,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn handle_test() {}

    #[test]
    fn test_kprintln_macro() {
        // モックのkprintln!が動作することを確認
        kprintln!("Testing kprintln! macro");
        kprintln!("Formatted output: {} + {} = {}", 1, 2, 3);
    }
}

/*
 * himawari kernel -> (this stack)
 * test -> (this stack)
 * linux tap -> (this stack)
 * pcap -> (this stack)
 */
