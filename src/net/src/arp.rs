// ref: https://datatracker.ietf.org/doc/html/rfc826

use core;
use core::convert::TryInto;

#[macro_use]
use crate::print_rs;

// Meson ビルド時
#[cfg(not(cargo_build))]
use bindings_net::net_if;

// Cargo ビルド時
#[cfg(cargo_build)]
use crate::bindings::net_if;

// net_if ヘルパー関数
#[cfg(not(cargo_build))]
use bindings_net::tx_arp_packet;
#[cfg(cargo_build)]
use crate::netif_helpers::tx_arp_packet;

// Meson ビルド時は bindings_mm を使用
#[cfg(not(cargo_build))]
use bindings_mm::mm_alloc;

// Cargo ビルド時（テスト）は mm_rs を使用
#[cfg(cargo_build)]
use mm_rs::mm_alloc;

use crate::l2::L2Packet;

pub struct Arp<'a> {
    data: &'a [u8],
}

#[derive(PartialEq)]
pub enum HardwareType {
    Ethernet,
    Unknown,
}

#[derive(PartialEq)]
pub enum ProtocolType {
    IPv4,
    Unknown,
}

#[derive(PartialEq)]
pub enum Operation {
    Request,
    Reply,
    Unknown,
}

impl<'a> Arp<'a> {
    pub fn new(data: &'a [u8]) -> Self {
        Self { data }
    }

    pub fn hardware_type(&self) -> HardwareType {
        kprintln!("{}: {} {}", line!(), self.data[0], self.data[1]);
        match self.data[0..2] {
            [0x00, 0x01] => HardwareType::Ethernet,
            _ => HardwareType::Unknown,
        }
    }

    pub fn protocol_type_bytes(&self) -> u16 {
        (self.data[2] as u16) << 8 | self.data[3] as u16
    }

    pub fn protocol_type(&self) -> ProtocolType {
        match self.data[2..4] {
            [0x08, 0x00] => ProtocolType::IPv4,
            _ => ProtocolType::Unknown,
        }
    }

    pub fn protocol_len(&self) -> u8 {
        self.data[4]
    }

    pub fn hardware_len(&self) -> u8 {
        self.data[5]
    }

    pub fn operation(&self) -> Operation {
        match self.data[6..8] {
            [0x00, 0x01] => Operation::Request,
            [0x00, 0x02] => Operation::Reply,
            _ => Operation::Unknown,
        }
    }

    pub fn sender_mac(&self) -> [u8; 6] {
        self.data[8..14].try_into().unwrap()
    }

    pub fn sender_ip_bytes(&self) -> [u8; 4] {
        //self.data[14..18]
        unimplemented!();
    }

    pub fn sender_ip(&self) -> u32 {
        u32::from_be_bytes(self.data[14..18].try_into().unwrap())
    }

    pub fn target_mac(&self) -> [u8; 6] {
        self.data[18..24].try_into().unwrap()
    }

    pub fn target_ip_bytes(&self) -> [u8; 4] {
        //self.data[24..28]
        unimplemented!();
    }

    pub fn target_ip(&self) -> u32 {
        u32::from_be_bytes(self.data[24..28].try_into().unwrap())
    }
}

impl core::fmt::Display for Arp<'_> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        let op = match self.operation() {
            Operation::Request => "Request",
            Operation::Reply => "Reply",
            Operation::Unknown => "Unknown",
        };

        let sender_mac = self.sender_mac();
        let target_mac = self.target_mac();
        let sender_ip = self.sender_ip();
        let target_ip = self.target_ip();

        write!(
            f,
            "ARP {}: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x} ({}.{}.{}.{}) -> {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x} ({}.{}.{}.{})",
            op,
            sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3], sender_mac[4], sender_mac[5],
            (sender_ip >> 24) & 0xff, (sender_ip >> 16) & 0xff, (sender_ip >> 8) & 0xff, sender_ip & 0xff,
            target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5],
            (target_ip >> 24) & 0xff, (target_ip >> 16) & 0xff, (target_ip >> 8) & 0xff, target_ip & 0xff
        )
    }
}

impl L2Packet for Arp<'_> {
    fn fill_buffer(_buf: &mut [u8]) -> i32 {
        // TODO: implement ARP packet serialization
        unimplemented!("fill_buffer for ARP not yet implemented");
    }
}

// ARP パケット生成用のビルダー
pub struct ArpBuilder {
    buffer: [u8; 28],
}

impl ArpBuilder {
    /// 新しい ArpBuilder を作成（デフォルト値を設定）
    pub fn new() -> Self {
        let mut buffer = [0u8; 28];
        // Hardware type: Ethernet
        buffer[0..2].copy_from_slice(&[0x00, 0x01]);
        // Protocol type: IPv4
        buffer[2..4].copy_from_slice(&[0x08, 0x00]);
        // Hardware address length: 6 (MAC)
        buffer[4] = 6;
        // Protocol address length: 4 (IPv4)
        buffer[5] = 4;
        Self { buffer }
    }

    /// Operation を設定
    pub fn operation(mut self, op: Operation) -> Self {
        match op {
            Operation::Request => self.buffer[6..8].copy_from_slice(&[0x00, 0x01]),
            Operation::Reply => self.buffer[6..8].copy_from_slice(&[0x00, 0x02]),
            _ => {}
        }
        self
    }

    /// Sender MAC address を設定
    pub fn sender_mac(mut self, mac: [u8; 6]) -> Self {
        self.buffer[8..14].copy_from_slice(&mac);
        self
    }

    /// Sender IP address を設定
    pub fn sender_ip(mut self, ip: u32) -> Self {
        self.buffer[14..18].copy_from_slice(&ip.to_be_bytes());
        self
    }

    /// Target MAC address を設定
    pub fn target_mac(mut self, mac: [u8; 6]) -> Self {
        self.buffer[18..24].copy_from_slice(&mac);
        self
    }

    /// Target IP address を設定
    pub fn target_ip(mut self, ip: u32) -> Self {
        self.buffer[24..28].copy_from_slice(&ip.to_be_bytes());
        self
    }

    /// パケットをバッファにコピーして返す
    pub fn build(self) -> [u8; 28] {
        self.buffer
    }

    /// 外部バッファに書き込む
    pub fn write_to(self, dest: &mut [u8]) -> Result<usize, ()> {
        if dest.len() < 28 {
            return Err(());
        }
        dest[..28].copy_from_slice(&self.buffer);
        Ok(28)
    }
}

fn generate_arp_response(nif: &net_if, req: &Arp) -> *mut u8 {
    // ArpBuilder でレスポンスパケットを構築
    let packet = ArpBuilder::new()
        .operation(Operation::Reply)
        .sender_mac(nif.mac_addr)
        .sender_ip(nif.ipv4_addr)
        .target_mac(req.sender_mac())
        .target_ip(req.sender_ip())
        .build();

    // mm_alloc でメモリを確保（28バイト）
    let ptr = unsafe { mm_alloc(28) as *mut u8 };
    if ptr.is_null() {
        panic!("mm_alloc failed");
    }

    // 確保したメモリにパケットをコピー
    unsafe {
        core::ptr::copy_nonoverlapping(packet.as_ptr(), ptr, 28);
    }

    ptr
}

fn handle_arp_request(nif: &net_if, arp: &Arp) -> i32 {
    let target_ip = arp.target_ip();

    kprintln!("arp request: {:#x}({:#x})", target_ip, nif.ipv4_addr);
    if nif.ipv4_addr != target_ip {
        // ignore the request non addressed to me.
        return 0;
    }

    kprintln!("{}:{}", file!(), line!());
    // ARPレスポンスパケットを生成
    let ptr = generate_arp_response(nif, arp);

    kprintln!("{}:{}", file!(), line!());
    // 生成されたARPパケットを送信
    // &net_if を使える！
    let payload = unsafe { core::slice::from_raw_parts(ptr, 28) };
    let dst_mac = arp.sender_mac();

    kprintln!("{}:{}", file!(), line!());
    tx_arp_packet(nif, &dst_mac, payload)
}

fn handle_arp_reply(_nif: &net_if, _arp: &Arp) -> i32 {
    // TODO: ARP応答を処理してARPテーブルを更新
    unimplemented!();
}

pub fn handle_arp_packet(nif: &net_if, packet: &[u8]) -> i32 {
    let arp = Arp::new(packet);

    if arp.hardware_type() != HardwareType::Ethernet {
        return -1;
    }

    if arp.protocol_type() != ProtocolType::IPv4 {
        return -1;
    }

    match arp.operation() {
        Operation::Request => handle_arp_request(nif, &arp),
        Operation::Reply => handle_arp_reply(nif, &arp),
        Operation::Unknown => {
            return -1;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn arp() {
        let packet = [
            0x00, 0x01, // ar$hrd
            0x08, 0x00, // ar$pro
            0x06, // ar$hln
            0x04, // ar$pln
            0x00, 0x01, // ar$op
            0xa6, 0xe4, 0x15, 0x50, 0x2b, 0x36, // ar$sha
            0x0a, 0x00, 0x02, 0x01, // ar$spa
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ar$tha
            0x0a, 0x00, 0x02, 0x02, // ar$tpa
        ];

        let arp = Arp::new(packet.as_slice());
        println!("{}", arp);
    }

    #[test]
    fn handle_arp_request_test() {
        // ARP Request パケット
        let packet = [
            0x00, 0x01, // ar$hrd (Ethernet)
            0x08, 0x00, // ar$pro (IPv4)
            0x06, // ar$hln (MAC address length)
            0x04, // ar$pln (IP address length)
            0x00, 0x01, // ar$op (Request)
            0xa6, 0xe4, 0x15, 0x50, 0x2b, 0x36, // ar$sha (送信元MAC)
            0x0a, 0x00, 0x02, 0x01, // ar$spa (送信元IP: 10.0.2.1)
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ar$tha (ターゲットMAC)
            0x0a, 0x00, 0x02, 0x02, // ar$tpa (ターゲットIP: 10.0.2.2)
        ];

        // テスト用のダミー net_if
        let nif = net_if {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
            ops: core::ptr::null_mut(),
            ipv4_addr: 0x0a000202, // 10.0.2.2 （パケットのターゲットIPと一致）
            mac_addr: [0x52, 0x54, 0x00, 0x12, 0x34, 0x56],
        };

        // ARPリクエストを処理（レスポンスを送信）
        let result = handle_arp_packet(&nif, &packet);
        assert_eq!(result, 0); // 成功
    }
}
