// ref: https://datatracker.ietf.org/doc/html/rfc791

#[cfg(not(cargo_build))]
use bindings_net::net_if;

pub fn handle_ip_packet(nif: &net_if, packet: &[u8]) -> i32 {
    -1
}

struct IP<'a> {
    data: &'a [u8],
}

enum Version {
    IPv4,
    Unknown,
}

enum Protocol {
    ICMP = 1,
    Unknown,
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
        match self.data[0] & 0xf {
            4 => Version::IPv4,
            _ => Version::Unknown,
        }
    }

    pub fn header_length(&self) -> u8 {
        (self.data[0] >> 4) & 0xf
    }

    //pub fn type_of_service(&self) -> ServiceType { self.data[1] }

    pub fn total_length(&self) -> u16 {
        (self.data[3] as u16) << 8 | self.data[4] as u16
    }

    pub fn id(&self) -> u16 {
        (self.data[5] as u16) << 8 | self.data[6] as u16
    }

    //pub fn flags(&self) -> {
    //self.data[7] >> 5
    //}

    pub fn fragment_offset(&self) -> u16 {
        ((self.data[7] as u16) & 0x1f) << 8 | self.data[8] as u16
    }

    pub fn time_to_live(&self) -> u8 {
        self.data[9]
    }

    pub fn protocol(&self) -> Protocol {
        match self.data[10] {
            1 => Protocol::ICMP,
            _ => Protocol::Unknown,
        }
    }

    pub fn checksum(&self) -> u16 {
        (self.data[11] as u16) << 8 | self.data[12] as u16
    }

    pub fn source_addr(&self) -> u32 {
        read_be32(&self.data[13..16])
    }

    pub fn dest_addr(&self) -> u32 {
        read_be32(&self.data[17..20])
    }
}
