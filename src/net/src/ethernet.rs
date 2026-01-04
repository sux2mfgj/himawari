use core::convert::TryInto;

pub struct EthernetFrame<'a> {
    data: &'a [u8],
}

#[derive(PartialEq, Debug)]
pub enum EthernetFrameType {
    IPv4,
    Arp,
    Unknown,
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
        let mut start = 12;
        // handle vlan
        if self.data[12..14] == [0x81, 0x00] {
            start += 4;
            unimplemented!("vlan is not supporeted yet");
        }

        match self.data[start..start + 2] {
            [0x08, 0x00] => EthernetFrameType::IPv4,
            [0x08, 0x06] => EthernetFrameType::Arp,
            _ => EthernetFrameType::Unknown,
        }
    }
}

use bindings_net::packet_t;
pub fn fill_ether_header(pkt: &packet_t) {}

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
