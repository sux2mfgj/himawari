use bindings_net::{mac_addr_t, net_if, packet_t, tx_packet};
use core::convert::TryInto;

use arp::handle_arp_packet;
//use ip::handle_ip_packet;

pub struct EthernetFrame<'a> {
    data: &'a [u8],
}

#[derive(PartialEq, Debug)]
pub enum EthernetFrameType {
    IPv4,
    Arp,
    Unknown([u8; 2]),
}

impl Into<[u8; 2]> for EthernetFrameType {
    fn into(self) -> [u8; 2] {
        match self {
            Self::IPv4 => [0x08, 0x00],
            Self::Arp => [0x08, 0x06],
            Self::Unknown(frame_type) => frame_type,
        }
    }
}

impl<'a> EthernetFrame<'a> {
    pub fn new(data: &'a [u8]) -> Self {
        Self { data }
    }

    pub fn dst_mac(&self) -> [u8; 6] {
        self.data[0..6].try_into().unwrap()
    }

    pub fn src_mac(&self) -> [u8; 6] {
        self.data[6..12].try_into().unwrap()
    }

    pub fn frame_type(&self) -> EthernetFrameType {
        let start = 12;
        // handle vlan
        if self.data[12..14] == [0x81, 0x00] {
            // VLAN tag present, offset would be +4
            unimplemented!("vlan is not supporeted yet");
        }

        let frame_type: [u8; 2] = self.data[start..start + 2].try_into().unwrap();

        match frame_type {
            [0x08, 0x00] => EthernetFrameType::IPv4,
            [0x08, 0x06] => EthernetFrameType::Arp,
            _ => EthernetFrameType::Unknown(frame_type),
        }
    }
}

pub fn handle_eth_packet(nif: &mut net_if, packet: &[u8]) -> i32 {
    kprintln!("handle_eth_packet: len={}", packet.len());
    if packet.len() < 14 {
        kprintln!("Packet too short: {} bytes", packet.len());
        return -1;
    }

    // Check EtherType directly
    let ethertype_slice = &packet[12..14];
    kprintln!("EtherType: {:02x} {:02x}", ethertype_slice[0], ethertype_slice[1]);

    match ethertype_slice {
        [0x08, 0x06] => {
            kprintln!("Calling handle_arp_packet");
            handle_arp_packet(nif, &packet[14..])
        },
        //[0x08, 0x00] => handle_ip_packet(nif, &packet[14..]),
        _ => {
            kprintln!("Unknown EtherType");
            0
        },
    }
}

struct EthHeaderBuilder<'a> {
    buf: &'a mut [u8],
    dst_set: bool,
    src_set: bool,
    type_set: bool,
}

impl<'a> EthHeaderBuilder<'a> {
    pub fn new(buf: &'a mut [u8]) -> Self {
        if buf.len() < 14 {
            panic!("Buffer too small for Ethernet header (need at least 14 bytes)");
        }
        Self {
            buf,
            dst_set: false,
            src_set: false,
            type_set: false,
        }
    }

    pub fn dst_mac(mut self, mac: &[u8; 6]) -> Self {
        self.buf[0..6].copy_from_slice(mac);
        self.dst_set = true;
        self
    }

    pub fn src_mac(mut self, mac: &[u8; 6]) -> Self {
        self.buf[6..12].copy_from_slice(mac);
        self.src_set = true;
        self
    }

    pub fn frame_type(mut self, frame_type: EthernetFrameType) -> Self {
        let type_ary: [u8; 2] = frame_type.into();
        self.buf[12..14].copy_from_slice(&type_ary);
        self.type_set = true;
        self
    }

    pub fn build(self) -> Result<usize, ()> {
        if !self.dst_set || !self.src_set || !self.type_set {
            return Err(());
        }
        Ok(14)
    }
}

pub fn tx_eth_packet(
    nif: &mut net_if,
    dst_mac: mac_addr_t,
    frame_type: EthernetFrameType,
    packet: &mut packet_t,
) -> Result<(), &'static str> {
    let mut buf: &mut [u8] =
        unsafe { core::slice::from_raw_parts_mut(packet.buf, packet.buf_size) };

    let mac_offset = packet.mac_offset as usize;

    let mut mac = &mut buf[mac_offset..];

    let Ok(eth_hdr_len) = EthHeaderBuilder::new(&mut mac)
        .dst_mac(&dst_mac)
        .src_mac(&nif.mac_addr)
        .frame_type(frame_type)
        .build()
    else {
        return Err("Invalid eth frame");
    };

    // Set data_len: mac_offset + Ethernet header (14) + payload length
    // payload length is (net_offset - mac_offset - 14) + actual data
    // For ARP, net_offset points to ARP data (28 bytes)
    let payload_start = packet.net_offset as usize;
    let payload_len = packet.buf_size - payload_start;  // This includes any padding

    // Calculate actual data length: from start to end of written data
    // For now, we use net_offset + expected payload size
    // TODO: track actual payload length more precisely
    packet.data_len = packet.net_offset as usize + 28;  // ARP is 28 bytes

    let ret = tx_packet(nif, packet);
    if ret != 0 {
        return Err("failed to submit a packet");
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ether_frame() {
        let dst_mac: [u8; 6] = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66];
        let src_mac = [0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff];
        let frame_type = [0x08, 0x06];
        let eth_frame = [&dst_mac[..], &src_mac[..], &frame_type[..]].concat();

        let eth_frame = EthernetFrame::new(&eth_frame);

        assert_eq!(dst_mac, eth_frame.dst_mac());
        assert_eq!(src_mac, eth_frame.src_mac());
        assert_eq!(EthernetFrameType::Arp, eth_frame.frame_type());
    }
}
