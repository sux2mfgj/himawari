pub struct Arp<'a> {
    data: &'a [u8],
}

pub enum HardwareType {
    Ethernet = 1,
    Unknown,
}

pub enum ProtocolType {
    IPv4 = 0x0800,
    Unknown,
}

pub enum Operation {
    Request,
    Response,
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
        //self.data[2..4]
        unimplemented!();
    }

    pub fn protocol_len(&self) -> u8 {
        //self.data[4]
        unimplemented!();
    }

    pub fn hardware_len(&self) -> u8 {
        //self.data[5]
        unimplemented!();
    }

    pub fn operation(&self) -> Operation {
        //self.data[6..8]
        unimplemented!();
    }

    pub fn sender_mac(&self) -> [u8; 6] {
        //self.data[8..14]
        unimplemented!();
    }

    pub fn sender_ip_bytes(&self) -> [u8; 4] {
        //self.data[14..18]
        unimplemented!();
    }

    pub fn sender_ip(&self) -> u32 {
        //self.data[14..18]
        unimplemented!();
    }

    pub fn target_mac(&self) -> [u8; 6] {
        //self.data[18..24]
        unimplemented!();
    }

    pub fn target_ip_bytes(&self) -> [u8; 4] {
        //self.data[24..28]
        unimplemented!();
    }

    pub fn target_ip(&self) -> u32 {
        //self.data[24..28]
        unimplemented!();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn arp() {
        //let packet = [];
    }
}
