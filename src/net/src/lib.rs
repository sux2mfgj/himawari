#![no_std]

#[cfg(test)]
#[macro_use]
extern crate std;

// テスト時はモックを使用
#[cfg(test)]
#[macro_use]
extern crate print_rs;

#[cfg(not(test))]
#[macro_use]
extern crate print_rs;

// Cargo ビルド時は mm_rs を使用
#[cfg(cargo_build)]
extern crate mm_rs;

// Meson ビルド時は print_rs と runtime_rs をリンク
#[cfg(not(cargo_build))]
extern crate runtime_rs;

// Meson ビルド時は bindings_net クレートを使用
#[cfg(not(cargo_build))]
extern crate bindings_net;
#[cfg(not(cargo_build))]
use bindings_net::net_if;

// Meson ビルド時のみ bindings_mm を使用
#[cfg(not(cargo_build))]
extern crate bindings_mm;

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

mod l2;

mod netif_helpers;

#[no_mangle]
pub extern "C" fn handle_rx_packet(nif: *mut net_if, data: *const u8, len: u32) -> i32 {
    unsafe {
        if data.is_null() || nif.is_null() {
            return -1;
        }

        let slice = core::slice::from_raw_parts(data, len as usize);
        let nif_ref = &*nif;
        handle_mac_packet(nif_ref, slice)
    }
}

fn handle_mac_packet(nif: &net_if, packet: &[u8]) -> i32 {
    if packet.len() < 14 {
        //kprintln!("Packet too short: {} bytes", packet.len());
        return -1;
    }

    // Check EtherType directly
    let ethertype_slice = &packet[12..14];

    match ethertype_slice {
        [0x08, 0x06] => handle_arp_packet(nif, &packet[14..]),
        [0x08, 0x00] => {
            unimplemented!("IPv4 packet handling not yet implemented");
        }
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
