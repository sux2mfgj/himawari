// ref: https://datatracker.ietf.org/doc/html/rfc791

use bindings_net::{ipv4_addr_t, net_if, packet_t};

use arp;
use icmp::handle_icmp_packet;

pub fn handle_ip_packet(nif: &mut net_if, packet: &[u8]) -> i32 {
    let ip = IP::new(packet);

    if ip.version() != Version::IPv4 {
        return 0;
    }

    kprintln!(
        "{}:{}: ver {:?} prot {:?}, src {:#x}, dst {:#x}",
        file!(),
        line!(),
        ip.version(),
        ip.protocol(),
        ip.source_addr(),
        ip.dest_addr()
    );

    match ip.protocol() {
        Protocol::ICMP => handle_icmp_packet(nif, &ip),
        Protocol::Unknown(prot) => {
            kprintln!("found unknown ipv4 packet: {}", prot);
            -1
        }
    }
}

pub fn fill_headers(nif: &mut net_if, pkt: &mut packet_t, dst_ip: ipv4_addr_t) -> bool {
    let Some(dest_mac) = arp::resolve_mac(nif, dst_ip) else {
        return false;
    };

    let src_mac = nif.mac_addr;
    // ethernet::fill_header(src_mac, dst_mac, &pkt);

    // src mac <- net_if
    // src ip  <- net_if
    // dst mac <- dest ip <- arp table (or arp request)
    // dst ip :必要
    //
    // ip header

    unimplemented!();

    true
}

#[derive(Debug)]
pub struct IP<'a> {
    data: &'a [u8],
}

#[derive(Debug, PartialEq)]
pub enum Version {
    IPv4,
    Unknown,
}

#[derive(Debug)]
pub enum Protocol {
    ICMP,
    Unknown(u8),
}

//enum ServiceType {}

fn read_be32(data: &[u8]) -> u32 {
    (data[0] as u32) << 24 | (data[1] as u32) << 16 | (data[2] as u32) << 8 | data[3] as u32
}

impl<'a> IP<'a> {
    pub fn new(data: &'a [u8]) -> Self {
        Self { data }
    }

    pub fn version(&self) -> Version {
        match self.data[0] >> 4 {
            4 => Version::IPv4,
            _ => Version::Unknown,
        }
    }

    pub fn header_length(&self) -> u8 {
        (self.data[0] >> 4) & 0xf
    }

    //pub fn type_of_service(&self) -> ServiceType { self.data[1] }

    pub fn total_length(&self) -> u16 {
        (self.data[2] as u16) << 8 | self.data[3] as u16
    }

    pub fn id(&self) -> u16 {
        (self.data[4] as u16) << 8 | self.data[5] as u16
    }

    //pub fn flags(&self) -> {
    //self.data[7] >> 5
    //}

    pub fn fragment_offset(&self) -> u16 {
        ((self.data[6] as u16) & 0x1f) << 8 | self.data[7] as u16
    }

    pub fn time_to_live(&self) -> u8 {
        self.data[8]
    }

    pub fn protocol(&self) -> Protocol {
        match self.data[9] {
            1 => Protocol::ICMP,
            _ => Protocol::Unknown(self.data[9]),
        }
    }

    pub fn checksum(&self) -> u16 {
        (self.data[11] as u16) << 8 | self.data[12] as u16
    }

    pub fn source_addr(&self) -> u32 {
        read_be32(&self.data[12..16])
    }

    pub fn dest_addr(&self) -> u32 {
        read_be32(&self.data[16..20])
    }

    pub fn data(&self) -> Option<&[u8]> {
        if self.data.len() < 20 {
            return None;
        }

        Some(&self.data[20..])
    }
}
