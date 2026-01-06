//ref: https://datatracker.ietf.org/doc/html/rfc792

use core::convert::TryInto;

use bindings_net::{alloc_packet_buf, net_if, packet_t};

use ip;
use ip::{tx_ip_packet, IP};

use arp;

use crate::ip::Protocol;

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
        kprintln!(
            "buf {}, total {}, header {}",
            self.data.len(),
            self.ip.total_length(),
            self.ip.header_length()
        );
        &self.data[8..]
    }
}

fn gen_icmp_echo_reply_packet<'a>(pkt: &mut packet_t, req: ICMPEcho<'a>) {
    let buf: &mut [u8] = unsafe { core::slice::from_raw_parts_mut(pkt.buf, pkt.buf_size) };
    let trans_offset = pkt.transport_offset as usize;

    let mut icmp = &mut buf[trans_offset..];

    // type
    icmp[0] = 0;
    // code
    icmp[1] = 0;
    // checksum
    icmp[2];
    icmp[3];
    // identifier
    icmp[4] = (req.id() >> 8) as u8;
    icmp[5] = req.id() as u8;
    // sequence number
    let seq = req.seq() + 1;
    icmp[6] = (seq >> 8) as u8;
    icmp[7] = seq as u8;
    // data
    let data_len = req.data().len();
    icmp[8..(8 + data_len)].copy_from_slice(req.data());
}

pub fn handle_icmp_echo_request<'a>(nif: &mut net_if, req: ICMPEcho<'a>) -> i32 {
    let l3_size = 8 + req.data().len();

    let Ok(pkt_ref) = alloc_packet_buf(nif, l3_size) else {
        return -1;
    };

    let dst_ip = req.ip.source_addr();

    gen_icmp_echo_reply_packet(pkt_ref, req);

    let Ok(()) = tx_ip_packet(nif, Protocol::ICMP, dst_ip, pkt_ref) else {
        return -1;
    };

    0
}

pub fn handle_icmp_packet(nif: &mut net_if, ip: &IP) -> i32 {
    let Some(icmp) = interpret_icmp(ip) else {
        return -1;
    };

    match icmp {
        Type::EchoRequest(req) => {
            kprintln!("icmp echo: req? {}", req.is_reqeust());

            handle_icmp_echo_request(nif, req)
        }
        Type::Unknown(t) => {
            kprintln!("unknown icmp type: {}", t);
            0
        }
        _ => {
            unimplemented!();
        }
    }
}
