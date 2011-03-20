open Portmidi

let note_on = Int32.of_int ((0x90 lsl 16) lor (100 lsl 8) lor 100)
let note_off = Int32.of_int ((0x80 lsl 16) lor (100 lsl 8) lor 100)

let e_on = {message=note_on; timestamp=Int32.zero}
let e_off = {message=note_off; timestamp=Int32.zero}

let _ =
    init ();
    let count = count_devices () in
    for i = 0 to count - 1 do
        let info = get_device_info i in
        print_string (info.name);
        if info.input then print_string " input";
        if info.output then print_string " output";
        print_newline ();
        ()
    done;
    let st = open_output 0 8 0 in
    write st [|e_on|] 0 1;
    ignore (read_line ());
    write st [|e_off|] 0 1;
    terminate ()
