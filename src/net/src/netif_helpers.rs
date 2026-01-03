// net_if のRustヘルパー関数
// C FFI の生ポインタをRustの参照で扱えるようにするラッパー

// Meson ビルド時
#[cfg(not(cargo_build))]
use bindings_net::{net_if, netif_tx_packet};

// Cargo ビルド時
#[cfg(cargo_build)]
use crate::bindings::net_if;

/// netif_tx_packet の安全なラッパー
///
/// # Arguments
/// * `nif` - ネットワークインターフェース（参照）
/// * `dst` - 宛先MACアドレス
/// * `mac_type` - EtherType（例: 0x0806 = ARP）
/// * `payload` - ペイロードデータ
///
/// # Returns
/// 成功時は 0、失敗時は負の値
pub fn tx_packet(nif: &net_if, dst: &[u8; 6], mac_type: u16, payload: &[u8]) -> i32 {
    #[cfg(not(cargo_build))]
    unsafe {
        netif_tx_packet(
            nif as *const net_if as *mut net_if, // &net_if → *mut net_if
            dst.as_ptr() as *mut u8,             // &[u8; 6] → *mut u8
            mac_type,
            payload.as_ptr() as *mut u8, // &[u8] → *mut u8
            payload.len(),
        )
    }

    #[cfg(cargo_build)]
    {
        // Cargo テスト時はダミー実装
        let _ = (nif, dst, mac_type, payload);
        // テスト時のデバッグ出力（stdが使える場合）
        #[cfg(test)]
        eprintln!(
            "tx_packet: dst={:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}, type=0x{:04x}, len={}",
            dst[0],
            dst[1],
            dst[2],
            dst[3],
            dst[4],
            dst[5],
            mac_type,
            payload.len()
        );
        0 // 成功を返す
    }
}

/// ARPパケット送信用のヘルパー
pub fn tx_arp_packet(nif: &net_if, dst: &[u8; 6], payload: &[u8]) -> i32 {
    tx_packet(nif, dst, 0x0608, payload) // 0x0806 = ARP
}

/// IPv4パケット送信用のヘルパー
pub fn tx_ipv4_packet(nif: &net_if, dst: &[u8; 6], payload: &[u8]) -> i32 {
    tx_packet(nif, dst, 0x0008, payload) // 0x0800 = IPv4
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tx_packet() {
        let nif = net_if {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
            ops: core::ptr::null_mut(),
            ipv4_addr: 0x0a000202,
            mac_addr: [0x52, 0x54, 0x00, 0x12, 0x34, 0x56],
        };

        let dst_mac = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff];
        let payload = [0x00, 0x01, 0x02, 0x03];

        let result = tx_packet(&nif, &dst_mac, 0x0806, &payload);
        assert_eq!(result, 0);
    }

    #[test]
    fn test_tx_arp_packet() {
        let nif = net_if {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
            ops: core::ptr::null_mut(),
            ipv4_addr: 0x0a000202,
            mac_addr: [0x52, 0x54, 0x00, 0x12, 0x34, 0x56],
        };

        let dst_mac = [0xa6, 0xe4, 0x15, 0x50, 0x2b, 0x36];
        let arp_payload = [0u8; 28];

        let result = tx_arp_packet(&nif, &dst_mac, &arp_payload);
        assert_eq!(result, 0);
    }
}
