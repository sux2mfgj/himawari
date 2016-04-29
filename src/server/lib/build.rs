extern crate cheddar;

fn main() {
    cheddar::Cheddar::new().expect("could not read maniefst")
        .run_build("../include/task_call.h");
}
