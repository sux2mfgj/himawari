#![feature(asm)]
#![no_std]
#![allow(dead_code)]
#![feature(lang_items)]

pub mod TaskCall{

#[repr(C)]
    pub enum ServerType {
        Memory,
        Process,
    }

#[repr(C)]
    pub enum MessageType {
        Send,
        Receive,
        Echo,
    }

#[repr(C)]
    pub struct Content {
        pub address: u64,
        pub number: u64,
    }

#[repr(C)]
    pub struct Message {
        pub source: ServerType,
        pub dest:   ServerType,
        pub m_type: MessageType,  
        pub m_content: Content,
    }

    // pub struct TaskCall{
    // }

//     #[no_mangle]
    pub fn taskcall(msg: *const Message) -> i64 {

        let mut result: i64 = 0;
        unsafe {
            asm!(
                "int $$0x81;"
                 :"={rax}"(result)
                 :"{rdi}"(msg)
                 );
//                  :"=a"(result)
        }
        result
    }
    
//     #[no_mangle]
    pub fn test() -> i64 {
        0
    }
//     pub extern fn receive(msg: *const Message) -> i64{
//         syscall(msg)
//     }
}

//#[lang = "eh_personality"] extern fn eh_personality() {}
//#[lang = "panic_fmt"] fn panic_fmt() -> ! {loop{}}
