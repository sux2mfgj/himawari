// ref: https://datatracker.ietf.org/doc/html/rfc791

use bindings_net::{ipv4_addr_t, net_if, packet_t};

use arp;
use ethernet::tx_eth_packet;
use icmp::handle_icmp_packet;

use crate::ethernet::EthernetFrameType;

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

pub fn tx_ip_packet(
    nif: &mut net_if,
    prot: Protocol,
    dst_ip: ipv4_addr_t,
    pkt: &mut packet_t,
) -> Result<(), &'static str> {
    let Some(dst_mac) = arp::resolve_mac(nif, dst_ip) else {
        return Err("failed to resolve mac");
    };

    let mut buf: &mut [u8] = unsafe { core::slice::from_raw_parts_mut(pkt.buf, pkt.buf_size) };

    let net_offset = pkt.net_offset as usize;

    let trans_offset = pkt.transport_offset as usize;
    let payload_len = buf[trans_offset..].len();

    let mut net = &mut buf[net_offset..];

    IpBuilder::new(net)
        .protocol(prot)
        .source_addr(nif.ipv4_addr)
        .dest_addr(dst_ip)
        .ttl(64)
        .build(payload_len)
        .expect("Failed to build IP header");

    tx_eth_packet(nif, dst_mac, EthernetFrameType::IPv4, pkt)
}

pub fn fill_headers(
    nif: &mut net_if,
    pkt: &mut packet_t,
    src_ip: ipv4_addr_t,
    dst_ip: ipv4_addr_t,
    prot: Protocol,
    payload_size: usize,
) -> Option<usize> {
    let Some(_dest_mac) = arp::resolve_mac(nif, dst_ip) else {
        return None;
    };

    let _src_mac = nif.mac_addr;

    // Convert raw pointer to mutable slice
    let buf_slice = unsafe { core::slice::from_raw_parts_mut(pkt.buf, pkt.buf_size) };

    let header_len = IpBuilder::new(buf_slice)
        .protocol(prot)
        .source_addr(src_ip)
        .dest_addr(dst_ip)
        .ttl(64)
        .build(payload_size)
        .expect("Failed to build IP header");

    Some(header_len)
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

impl Protocol {
    pub fn to_u8(&self) -> u8 {
        match self {
            Protocol::ICMP => 1,
            Protocol::Unknown(n) => *n,
        }
    }
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
        (self.data[0] & 0xf) * 4
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

/// IPv4 ヘッダチェックサムを計算 (RFC 791)
///
/// # Arguments
/// * `header` - IPv4 ヘッダ（20バイト、チェックサムフィールドは0であること）
///
/// # Returns
/// 計算されたチェックサム値
fn calculate_checksum(header: &[u8]) -> u16 {
    let mut sum: u32 = 0;

    // ヘッダを16ビット単位で合計
    for i in (0..20).step_by(2) {
        let word = ((header[i] as u32) << 8) | (header[i + 1] as u32);
        sum += word;
    }

    // キャリーを加算
    while (sum >> 16) != 0 {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // 1の補数を取る
    !sum as u16
}

/// IPv4 パケットビルダー
///
/// 外部バッファを受け取り、IPv4 ヘッダを構築します。
/// ビルダーパターンを使用してフィールドを設定し、build() でヘッダを完成させます。
///
/// # Example
/// ```
/// let mut buffer = [0u8; 1500];
/// let header_len = IpBuilder::new(&mut buffer)
///     .protocol(Protocol::ICMP)
///     .source_addr(0x0a000202)
///     .dest_addr(0x0a000201)
///     .ttl(64)
///     .build(payload.len())
///     .expect("Failed to build IP header");
/// ```
pub struct IpBuilder<'a> {
    buffer: &'a mut [u8],
}

impl<'a> IpBuilder<'a> {
    /// 新しい IpBuilder を作成し、デフォルト値でヘッダを初期化
    ///
    /// # Arguments
    /// * `buffer` - IPv4 ヘッダを書き込むバッファ（最小20バイト必要）
    ///
    /// # Panics
    /// バッファサイズが20バイト未満の場合
    pub fn new(buffer: &'a mut [u8]) -> Self {
        if buffer.len() < 20 {
            panic!("Buffer too small for IP header (need at least 20 bytes)");
        }

        // バッファをゼロクリア
        for i in 0..20 {
            buffer[i] = 0;
        }

        // デフォルト値を設定
        buffer[0] = 0x45; // Version=4, IHL=5 (20 bytes)
        buffer[1] = 0; // Type of Service
                       // [2-3] Total Length - build() で設定
                       // [4-7] Identification, Flags, Fragment Offset - デフォルト0
        buffer[8] = 64; // TTL = 64
                        // [9] Protocol - 後で設定
                        // [10-11] Checksum - build() で計算
                        // [12-19] Source/Dest IP - 後で設定

        Self { buffer }
    }

    /// プロトコルを設定
    pub fn protocol(self, proto: Protocol) -> Self {
        self.buffer[9] = proto.to_u8();
        self
    }

    /// 送信元 IP アドレスを設定
    pub fn source_addr(self, ip: u32) -> Self {
        self.buffer[12..16].copy_from_slice(&ip.to_be_bytes());
        self
    }

    /// 宛先 IP アドレスを設定
    pub fn dest_addr(self, ip: u32) -> Self {
        self.buffer[16..20].copy_from_slice(&ip.to_be_bytes());
        self
    }

    /// TTL (Time To Live) を設定
    pub fn ttl(self, ttl: u8) -> Self {
        self.buffer[8] = ttl;
        self
    }

    /// Identification フィールドを設定
    pub fn id(self, id: u16) -> Self {
        self.buffer[4..6].copy_from_slice(&id.to_be_bytes());
        self
    }

    /// Don't Fragment フラグを設定
    pub fn dont_fragment(self, enable: bool) -> Self {
        if enable {
            self.buffer[6] |= 0x40; // Set DF bit
        } else {
            self.buffer[6] &= !0x40; // Clear DF bit
        }
        self
    }

    /// Type of Service (TOS) フィールドを設定
    pub fn tos(self, tos: u8) -> Self {
        self.buffer[1] = tos;
        self
    }

    /// ヘッダを完成させる（Total Length とチェックサムを計算）
    ///
    /// # Arguments
    /// * `payload_len` - ペイロードのサイズ（バイト）
    ///
    /// # Returns
    /// * `Ok(usize)` - ヘッダサイズ（常に20）
    /// * `Err(())` - Total Length が 65535 を超える場合
    pub fn build(self, payload_len: usize) -> Result<usize, ()> {
        let total_len = 20 + payload_len;
        if total_len > 0xFFFF {
            return Err(());
        }

        // Total Length を設定
        self.buffer[2..4].copy_from_slice(&(total_len as u16).to_be_bytes());

        // Checksum をクリア
        self.buffer[10..12].copy_from_slice(&[0, 0]);

        // Checksum を計算
        let checksum = calculate_checksum(&self.buffer[..20]);
        self.buffer[10..12].copy_from_slice(&checksum.to_be_bytes());

        Ok(20)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_protocol_to_u8() {
        assert_eq!(Protocol::ICMP.to_u8(), 1);
        assert_eq!(Protocol::Unknown(6).to_u8(), 6); // TCP
        assert_eq!(Protocol::Unknown(17).to_u8(), 17); // UDP
    }

    #[test]
    fn test_checksum_calculation() {
        // 既知のIPv4ヘッダでチェックサムをテスト
        // 例: 10.0.2.15 -> 10.0.2.2, ICMP, TTL=64, ID=0
        let mut header = [
            0x45, 0x00, 0x00, 0x54, // Ver, IHL, TOS, Total Len=84
            0x00, 0x00, 0x40, 0x00, // ID=0, Flags=DF, Frag Offset=0
            0x40, 0x01, 0x00, 0x00, // TTL=64, Proto=ICMP, Checksum=0
            0x0a, 0x00, 0x02, 0x0f, // Source: 10.0.2.15
            0x0a, 0x00, 0x02, 0x02, // Dest: 10.0.2.2
        ];

        let checksum = calculate_checksum(&header);
        // Wireshark などで計算した既知の値と比較
        // この例では手動計算
        assert_ne!(checksum, 0); // チェックサムは0でないはず

        // チェックサムを設定して検証
        header[10..12].copy_from_slice(&checksum.to_be_bytes());

        // 正しいヘッダのチェックサム合計は 0xFFFF になるはず
        let mut sum: u32 = 0;
        for i in (0..20).step_by(2) {
            let word = ((header[i] as u32) << 8) | (header[i + 1] as u32);
            sum += word;
        }
        while (sum >> 16) != 0 {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        assert_eq!(sum as u16, 0xFFFF);
    }

    #[test]
    fn test_ip_builder_default() {
        let mut buffer = [0u8; 20];
        let _ = IpBuilder::new(&mut buffer);

        // デフォルト値を確認
        assert_eq!(buffer[0], 0x45); // Version=4, IHL=5
        assert_eq!(buffer[1], 0); // TOS=0
        assert_eq!(buffer[8], 64); // TTL=64
    }

    #[test]
    fn test_ip_builder_basic() {
        let mut buffer = [0u8; 100];
        let payload_len = 8; // ICMP Echo request の最小サイズ

        let result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::ICMP)
            .source_addr(0x0a000202) // 10.0.2.2
            .dest_addr(0x0a000201) // 10.0.2.1
            .ttl(64)
            .id(0x1234)
            .build(payload_len);

        assert!(result.is_ok());
        assert_eq!(result.unwrap(), 20);

        // ヘッダの内容を確認
        assert_eq!(buffer[0], 0x45); // Version=4, IHL=5
        assert_eq!(buffer[9], 1); // Protocol=ICMP

        // Total Length を確認
        let total_len = ((buffer[2] as u16) << 8) | (buffer[3] as u16);
        assert_eq!(total_len, 28); // 20 + 8

        // IP アドレスを確認
        assert_eq!(&buffer[12..16], &[0x0a, 0x00, 0x02, 0x02]);
        assert_eq!(&buffer[16..20], &[0x0a, 0x00, 0x02, 0x01]);

        // ID を確認
        assert_eq!(&buffer[4..6], &[0x12, 0x34]);

        // TTL を確認
        assert_eq!(buffer[8], 64);
    }

    #[test]
    fn test_ip_builder_with_parser() {
        let mut buffer = [0u8; 100];
        let payload_len = 16;

        let _result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::ICMP)
            .source_addr(0xc0a80001) // 192.168.0.1
            .dest_addr(0xc0a800fe) // 192.168.0.254
            .ttl(128)
            .id(0xabcd)
            .dont_fragment(true)
            .build(payload_len)
            .unwrap();

        // ビルダーで作成したヘッダをパーサーで読む
        let ip = IP::new(&buffer);

        assert_eq!(ip.version(), Version::IPv4);
        assert_eq!(ip.header_length(), 5);
        assert_eq!(ip.total_length(), 36); // 20 + 16
        assert_eq!(ip.id(), 0xabcd);
        assert_eq!(ip.time_to_live(), 128);
        assert_eq!(ip.protocol().to_u8(), 1); // ICMP
        assert_eq!(ip.source_addr(), 0xc0a80001);
        assert_eq!(ip.dest_addr(), 0xc0a800fe);

        // Don't Fragment フラグを確認
        assert_eq!(buffer[6] & 0x40, 0x40);
    }

    #[test]
    fn test_ip_builder_checksum_verification() {
        let mut buffer = [0u8; 100];

        let _result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::ICMP)
            .source_addr(0x0a000202)
            .dest_addr(0x0a000201)
            .ttl(64)
            .build(0)
            .unwrap();

        // チェックサムを検証（ヘッダ全体の合計が 0xFFFF になるはず）
        let mut sum: u32 = 0;
        for i in (0..20).step_by(2) {
            let word = ((buffer[i] as u32) << 8) | (buffer[i + 1] as u32);
            sum += word;
        }
        while (sum >> 16) != 0 {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        assert_eq!(sum as u16, 0xFFFF);
    }

    #[test]
    fn test_ip_builder_all_options() {
        let mut buffer = [0u8; 100];

        let result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::Unknown(17)) // UDP
            .source_addr(0x08080808) // 8.8.8.8
            .dest_addr(0x08080404) // 8.8.4.4
            .ttl(255)
            .id(0xffff)
            .dont_fragment(true)
            .tos(0x10) // Minimize delay
            .build(100);

        assert!(result.is_ok());

        assert_eq!(buffer[1], 0x10); // TOS
        assert_eq!(buffer[9], 17); // Protocol=UDP
        assert_eq!(buffer[8], 255); // TTL
        assert_eq!(&buffer[4..6], &[0xff, 0xff]); // ID
    }

    #[test]
    #[should_panic(expected = "Buffer too small")]
    fn test_ip_builder_small_buffer() {
        let mut buffer = [0u8; 10]; // 20バイト未満
        let _ = IpBuilder::new(&mut buffer);
    }

    #[test]
    fn test_ip_builder_max_length() {
        let mut buffer = [0u8; 100];
        let max_payload = 0xFFFF - 20; // 最大ペイロードサイズ

        let result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::ICMP)
            .source_addr(0x0a000202)
            .dest_addr(0x0a000201)
            .build(max_payload);

        assert!(result.is_ok());

        // Total Length を確認
        let total_len = ((buffer[2] as u16) << 8) | (buffer[3] as u16);
        assert_eq!(total_len, 0xFFFF);
    }

    #[test]
    fn test_ip_builder_overflow_length() {
        let mut buffer = [0u8; 100];
        let too_large = 0xFFFF; // ヘッダ + ペイロード > 0xFFFF

        let result = IpBuilder::new(&mut buffer)
            .protocol(Protocol::ICMP)
            .source_addr(0x0a000202)
            .dest_addr(0x0a000201)
            .build(too_large);

        assert!(result.is_err());
    }
}
