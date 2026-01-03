pub trait L2Packet {
    fn fill_buffer(buf: &mut [u8]) -> i32;
}
