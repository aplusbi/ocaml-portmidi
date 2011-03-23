open Portmidi

let note_on = message 0x90 60 100
let note_off = message 0x80 60 0

let e_on = {message=note_on; timestamp=Int32.zero}
let e_off = {message=note_off; timestamp=Int32.zero}

type mode = Input | Output

let get_device m =
    print_endline "Select device:";
    let print_device i = function
        | Input, {input=true; name=n}
        | Output, {output=true; name=n} ->
                print_int i;
                print_endline (" " ^ n);
        | _ -> ()
    in
    let count = count_devices () in
    for i = 0 to count - 1 do
        let info = get_device_info i in
        print_device i (m, info);
        ()
    done;
    read_int ()

let test_input () =
    let d = get_device Input in
    let st = open_input d 8 in
    let buf = Array.create 1 {message=Int32.zero; timestamp=Int32.zero} in
    let rec loop () =
        let _ = match poll st with
        | true -> ignore (read st buf 0 1);
            let s, d1, d2 = message_contents buf.(0).message in
            Printf.printf "Read: %d %d %d\n" s d1 d2;
            flush stdout
        | false -> ()
        in
        loop ()
    in
    loop ()

let test_output () =
    let d = get_device Output in
    let st = open_output d 8 0 in
    (* Switch to violin *)
    write_short st Int32.zero (message 0xC0 40 0);
    let rec loop msg =
        let m = match msg with
        | (0x90, key) -> write_short st Int32.zero (message 0x80 key 0); (0x80, 0)
        | _ -> let key = Random.int 128 in
            write_short st Int32.zero (message 0x90 key 100);
            (0x90, key)
        in Unix.sleep 1; loop m
    in
    loop (0, 0)

let rec main _ =
    print_endline "Select mode:";
    print_endline "0 Reader";
    print_endline "1 Writer";
    match read_int () with
    | 0 -> test_input ()
    | 1 -> test_output ()
    | _ -> main ()

let _ =
    init ();
    pt_start(1);
    at_exit terminate;
    main ()
