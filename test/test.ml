open Portmidi

let note_on = message 0x90 60 100
let note_off = message 0x90 60 0

let e_on = {message=note_on; timestamp=Int32.zero}
let e_off = {message=note_off; timestamp=Int32.zero}

let _ =
    pt_start(1);
    init ();
    let count = count_devices () in
    for i = 0 to count - 1 do
        let info = get_device_info i in
        print_int i;
        print_string (" " ^ info.name);
        if info.input then print_string " input";
        if info.output then print_string " output";
        print_newline ();
        ()
    done;
    let d = read_int () in
    let st = open_output d 8 0 in
    ignore (read_line ());
    write st [|e_on|] 0 1;
    ignore (read_line ());
    write st [|e_off|] 0 1;
    terminate ()
