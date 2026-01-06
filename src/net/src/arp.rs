// ref: https://datatracker.ietf.org/doc/html/rfc826

use core;
use core::convert::TryInto;

use bindings_net::{alloc_packet_buf, ipv4_addr_t, mac_addr_t, net_if, packet_t};

use crate::ethernet::tx_eth_packet;

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

#[derive(PartialEq, Debug)]
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

// ARP パケット生成用のビルダー
pub struct ArpBuilder<'a> {
    buf: &'a mut [u8],
    op_set: bool,
    sender_mac_set: bool,
    sender_ip_set: bool,
    target_mac_set: bool,
    target_ip_set: bool,
}

impl<'a> ArpBuilder<'a> {
    /// 新しい ArpBuilder を作成（デフォルト値を設定）
    pub fn new(buf: &'a mut [u8]) -> Self {
        if buf.len() < 28 {
            panic!("Buffer too small for ARP packet (need at least 28 bytes)");
        }

        // Hardware type: Ethernet
        buf[0..2].copy_from_slice(&[0x00, 0x01]);
        // Protocol type: IPv4
        buf[2..4].copy_from_slice(&[0x08, 0x00]);
        // Hardware address length: 6 (MAC)
        buf[4] = 6;
        // Protocol address length: 4 (IPv4)
        buf[5] = 4;

        Self {
            buf,
            op_set: false,
            sender_mac_set: false,
            sender_ip_set: false,
            target_mac_set: false,
            target_ip_set: false,
        }
    }

    /// Operation を設定
    pub fn operation(mut self, op: Operation) -> Self {
        match op {
            Operation::Request => self.buf[6..8].copy_from_slice(&[0x00, 0x01]),
            Operation::Reply => self.buf[6..8].copy_from_slice(&[0x00, 0x02]),
            _ => {}
        }
        self.op_set = true;
        self
    }

    /// Sender MAC address を設定
    pub fn sender_mac(mut self, mac: &[u8; 6]) -> Self {
        self.buf[8..14].copy_from_slice(mac);
        self.sender_mac_set = true;
        self
    }

    /// Sender IP address を設定
    pub fn sender_ip(mut self, ip: u32) -> Self {
        self.buf[14..18].copy_from_slice(&ip.to_be_bytes());
        self.sender_ip_set = true;
        self
    }

    /// Target MAC address を設定
    pub fn target_mac(mut self, mac: &[u8; 6]) -> Self {
        self.buf[18..24].copy_from_slice(mac);
        self.target_mac_set = true;
        self
    }

    /// Target IP address を設定
    pub fn target_ip(mut self, ip: u32) -> Self {
        self.buf[24..28].copy_from_slice(&ip.to_be_bytes());
        self.target_ip_set = true;
        self
    }

    /// パケット構築を完了し、バリデーションを行う
    pub fn build(self) -> Result<usize, ()> {
        if !self.op_set
            || !self.sender_mac_set
            || !self.sender_ip_set
            || !self.target_mac_set
            || !self.target_ip_set
        {
            return Err(());
        }
        Ok(28)
    }
}

fn setup_arp_resp_packet(nif: &net_if, pkt: &packet_t, arp: &Arp) -> Result<(), ()> {
    let buf: &mut [u8] = unsafe { core::slice::from_raw_parts_mut(pkt.buf, pkt.buf_size) };

    let net_offset = pkt.net_offset as usize;
    kprintln!(
        "pkt len {}, offset mac {} net {} | arp len {}",
        buf.len(),
        pkt.mac_offset,
        pkt.net_offset,
        buf[net_offset..].len(),
    );

    ArpBuilder::new(&mut buf[net_offset..])
        .operation(Operation::Reply)
        .sender_mac(&nif.mac_addr)
        .sender_ip(nif.ipv4_addr)
        .target_mac(&arp.sender_mac())
        .target_ip(arp.sender_ip())
        .build()?;

    Ok(())
}

fn handle_arp_request(nif: &mut net_if, arp: &Arp) -> i32 {
    let target_ip = arp.target_ip();

    kprintln!("{}:{}", file!(), line!());
    if nif.ipv4_addr != target_ip {
        // ignore the request non addressed to me.
        return 0;
    }

    let Ok(pkt_ref) = alloc_packet_buf(nif, 28) else {
        return -1;
    };
    kprintln!("{}:{}", file!(), line!());

    pkt_ref.buf_size = pkt_ref.net_offset as usize + 8 + 6 + 4 + 6 + 4;

    // ARPレスポンスパケットを生成
    if setup_arp_resp_packet(nif, pkt_ref, arp).is_err() {
        return -1;
    }
    kprintln!("{}:{}", file!(), line!());

    let dst_mac = arp.sender_mac();

    kprintln!("{}:{}", file!(), line!());
    let Ok(_len) = tx_eth_packet(
        nif,
        dst_mac,
        crate::ethernet::EthernetFrameType::Arp,
        pkt_ref,
    ) else {
        return -1;
    };
    kprintln!("{}:{}", file!(), line!());

    0
}

fn handle_arp_reply(nif: &mut net_if, arp: &Arp) -> i32 {
    update_arp_table(nif, arp.sender_ip(), arp.sender_mac());
    0
}

pub fn handle_arp_packet(nif: &mut net_if, packet: &[u8]) -> i32 {
    let arp = Arp::new(packet);

    if arp.hardware_type() != HardwareType::Ethernet {
        return -1;
    }

    if arp.protocol_type() != ProtocolType::IPv4 {
        return -1;
    }

    kprintln!("handle arp: {:?}", arp.operation());

    match arp.operation() {
        Operation::Request => handle_arp_request(nif, &arp),
        Operation::Reply => handle_arp_reply(nif, &arp),
        Operation::Unknown => {
            return -1;
        }
    }
}

fn update_arp_table(nif: &mut net_if, ipv4_addr: ipv4_addr_t, mac_addr: mac_addr_t) {
    for entry in &mut nif.arp_table {
        if entry.ipv4_addr != 0 {
            continue;
        }

        entry.ipv4_addr = ipv4_addr;
        entry.mac_addr = mac_addr;

        return;
    }

    unimplemented!();
}

fn tx_arp_request(nif: &mut net_if, ipv4_addr: ipv4_addr_t) -> Result<(), ()> {
    let Ok(pkt_ref) = alloc_packet_buf(nif, 28) else {
        return Err(());
    };

    let buf: &mut [u8] = unsafe { core::slice::from_raw_parts_mut(pkt_ref.buf, pkt_ref.buf_size) };

    let net_offset = pkt_ref.net_offset as usize;

    ArpBuilder::new(&mut buf[net_offset..])
        .operation(Operation::Request)
        .sender_mac(&nif.mac_addr)
        .sender_ip(nif.ipv4_addr)
        .target_mac(&[0xff, 0xff, 0xff, 0xff, 0xff, 0xff])
        .target_ip(ipv4_addr)
        .build()?;

    tx_eth_packet(
        nif,
        [0xff, 0xff, 0xff, 0xff, 0xff, 0xff],
        crate::ethernet::EthernetFrameType::Arp,
        pkt_ref,
    )
    .map_err(|e| kprintln!("{}", e))
}

pub fn resolve_mac(nif: &mut net_if, dst_ip: ipv4_addr_t) -> Option<mac_addr_t> {
    for entry in &nif.arp_table {
        if entry.ipv4_addr == dst_ip {
            return Some(entry.mac_addr);
        }
    }

    if tx_arp_request(nif, dst_ip).is_ok() {
        kprintln!("sent arp request: {:x}", dst_ip);
    } else {
        kprintln!("failed to send arp request: {:x}", dst_ip);
    }

    None
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
