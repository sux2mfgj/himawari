#![no_std]
//#![no_main]
#![feature(lang_items)]
//#[no_link]

//#[link(name="libserver", kind="staticlib")]
extern crate server;

use server::TaskCall::{taskcall, test};

#[no_mangle]
pub extern fn main() {

    let msg = server::TaskCall::Message{
        source: server::TaskCall::ServerType::Memory,
        dest:   server::TaskCall::ServerType::Process,
        m_type: server::TaskCall::MessageType::Send,
        m_content: server::TaskCall::Content{address: 0x0, number: 0x0}
    };

    let retval = taskcall(&msg);
//    let t = test();
//     let x = server::TaskCall::receive();
    loop{}
}

#[lang = "eh_personality"] extern fn eh_personality() {}
#[lang = "panic_fmt"] fn panic_fmt() -> ! {loop{}}
