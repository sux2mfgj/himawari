//ref: https://datatracker.ietf.org/doc/html/rfc792

use core::convert::TryInto;

use bindings_net::{alloc_packet_buf, net_if};

use ip;
use ip::IP;

struct ICMP<'a> {
    ip: &'a IP<'a>,
    data: &'a [u8],
}

#[derive(Debug)]
enum Type<'a> {
    DestinationUnreachable,
    TimeExcceded,
    ParameterProblem,
    SourceQuench,
    Redirect,
    EchoRequest(ICMPEcho<'a>),
    EchoReply(ICMPEcho<'a>),
    TimeStamp,
    TimeStampReply,
    InfoRequest,
    InfoReply,
    Unknown(u8),
}

// impl From<u8> for Type {
//     fn from(value: u8) -> Self {
//         match value {
//             0 => Self::EchoReply,
//             3 => Self::DestinationUnreachable,
//             4 => Self::SourceQuench,
//             5 => Self::Redirect,
//             8 => Self::EchoRequest,
//             11 => Self::TimeExcceded,
//             12 => Self::ParameterProblem,
//             13 => Self::TimeStamp,
//             14 => Self::TimeStampReply,
//             15 => Self::InfoRequest,
//             16 => Self::InfoReply,
//             _ => Self::Unknown(value),
//         }
//     }
// }

enum Code {
    NetUnreachable,
    HostUnreachable,
    ProtocolUnreachable,
    PortUnreachable,
    FlagmentationNeeded,
    SourceRouteFailed,
    Unknown(u8),
}

impl From<u8> for Code {
    fn from(value: u8) -> Self {
        match value {
            0 => Self::NetUnreachable,
            1 => Self::HostUnreachable,
            2 => Self::ProtocolUnreachable,
            3 => Self::PortUnreachable,
            4 => Self::FlagmentationNeeded,
            5 => Self::SourceRouteFailed,
            _ => Self::Unknown(value),
        }
    }
}

fn read_be16(data: &[u8; 2]) -> u16 {
    (data[0] as u16) << 8 | (data[1] as u16)
}

fn interpret_icmp<'a>(ip: &'a IP) -> Option<Type<'a>> {
    let Some(data) = ip.data() else {
        return None;
    };

    let icmp_type = data[0];

    let icmp = match icmp_type {
        8 => Type::EchoRequest(ICMPEcho::new(ip, data)),
        0 | 3 | 4 | 5 | 11 | 12 | 13 | 14 | 15 | 16 => {
            unimplemented!("icmp type {} is not supported yet", icmp_type);
        }
        // 8 => Type::EchoReply(ICMPEcho::new(data)),
        _ => Type::Unknown(icmp_type),
    };

    Some(icmp)
}

impl<'a> ICMP<'a> {
    pub fn new(ip: &'a IP<'a>) -> Option<Self> {
        let Some(data) = ip.data() else {
            return None;
        };

        Some(Self { ip, data })
    }

    //
    // pub fn p_type(&self) -> Type {
    //     self.data[0].into()
    // }
    //
    // pub fn code(&self) -> u8 {
    //     self.data[1]
    // }
    //
    // pub fn checksum(&self) -> u16 {
    //     read_be16(&self.data[2..4].try_into().unwrap())
    // }
    //
    // pub fn id(&self) -> u16 {
    //     read_be16(&self.data[4..6].try_into().unwrap())
    // }
}

#[derive(Debug)]
struct ICMPEcho<'a> {
    ip: &'a IP<'a>,
    data: &'a [u8],
}

impl<'a> ICMPEcho<'a> {
    pub fn new(ip: &'a IP, data: &'a [u8]) -> Self {
        Self { ip, data }
    }

    pub fn is_reqeust(&self) -> bool {
        self.data[0] == 8
    }

    pub fn id(&self) -> u16 {
        read_be16(&self.data[4..6].try_into().unwrap())
    }

    pub fn seq(&self) -> u16 {
        read_be16(&self.data[6..8].try_into().unwrap())
    }

    pub fn data(&self) -> &[u8] {
        &self.data[8..]
    }
}

pub fn handle_icmp_packet(nif: &mut net_if, ip: &IP) -> i32 {
    let Some(icmp) = interpret_icmp(ip) else {
        return -1;
    };

    match icmp {
        Type::EchoRequest(req) => {
            kprintln!("icmp echo: req? {}", req.is_reqeust());
            //unimplemented!();

            let Ok(pkt) = alloc_packet_buf(1024) else {
                return -1;
            };

            ip::fill_headers(nif, pkt, ip.source_addr());

            unimplemented!();
        }
        _ => {
            unimplemented!();
        }
        Type::Unknown(t) => {
            kprintln!("unknown icmp type: {}", t);
        }
    }

    0
}
