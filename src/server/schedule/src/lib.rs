#![no_std]
//#![no_main]
#![feature(lang_items)]
//#[no_link]

extern crate server;

#[no_mangle]
pub extern fn main() {

    loop{}
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
    }
}
