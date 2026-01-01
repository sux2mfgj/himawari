// ref: https://datatracker.ietf.org/doc/html/rfc826

use core;
use core::convert::TryInto;

// Meson ビルド時
#[cfg(not(cargo_build))]
use bindings_net::net_if;

// Cargo ビルド時
#[cfg(cargo_build)]
use crate::bindings::net_if;

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
        match self.data[0..2] {
            [0x00, 0x01] => HardwareType::Ethernet,
            _ => HardwareType::Unknown,
        }
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

fn generate_arp_response<'a>(_nif: &net_if, _req: &Arp<'a>) -> Arp<'a> {
    // TODO: mm_alloc の実装後に実装

    unimplemented!("generate_arp_response not yet implemented - waiting for mm_alloc");
}

fn handle_arp_request(nif: &net_if, arp: &Arp) -> i32 {
    let target_ip = arp.target_ip();

    if nif.ipv4_addr != target_ip {
        // ignore the request non addressed to me.
        return 0;
    }

    let _resp_arp = generate_arp_response(nif, arp);
    //nif.ops.send_packet()

    unimplemented!("handle_arp_request not yet implemented");
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
    #[should_panic] // unimplemented! を呼ぶので
    fn handle_arp() {
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

        // テスト用のダミー net_if
        let nif = net_if {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
            ipv4_addr: 0x0a000202, // 10.0.2.2
            mac_addr: [0x52, 0x54, 0x00, 0x12, 0x34, 0x56],
        };

        handle_arp_packet(&nif, &packet);
    }
}
