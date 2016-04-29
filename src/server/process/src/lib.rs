#![no_std]
//#![no_main]
#![feature(lang_items)]
//#[no_link]

extern crate server;

#[no_mangle]
pub extern fn main() {

    loop{}
}

#[lang = "eh_personality"] extern fn eh_personality() {}
#[lang = "panic_fmt"] fn panic_fmt() -> ! {loop{}}
