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

mod arp;
mod ethernet;
use ethernet::handle_eth_packet;
mod icmp;
mod ip;

#[no_mangle]
pub extern "C" fn handle_rx_packet(nif: *mut net_if, data: *const u8, len: u32) -> i32 {
    kprintln!("handle_rx_packet called: len={}", len);
    unsafe {
        if data.is_null() || nif.is_null() {
            kprintln!("handle_rx_packet: null pointer");
            return -1;
        }

        let slice = core::slice::from_raw_parts(data, len as usize);
        kprintln!("handle_rx_packet: calling handle_eth_packet");
        let result = handle_eth_packet(&mut *nif, slice);
        kprintln!("handle_rx_packet: result={}", result);
        result
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
