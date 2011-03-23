type stream

external init : unit -> unit = "caml_pm_initialize"

external terminate : unit -> unit = "caml_pm_terminate"

type device_info = {
    struct_version : int; (**< this internal structure version *) 
    interf : string; (**< underlying MIDI API, e.g. MMSystem or DirectX *)
    name : string;   (**< device name, e.g. USB MidiSport 1x1 *)
    input : bool; (**< true iff input is available *)
    output : bool; (**< true iff output is available *)
    opened : bool (**< used by generic PortMidi code to do error checking on arguments *)
}

external count_devices : unit -> int = "caml_pm_count_devices"

external get_default_input_device_id : unit -> int = "caml_pm_get_default_input_device_id"

external get_default_output_device_id : unit -> int = "caml_pm_get_default_output_device_id"

external get_device_info : int -> device_info = "caml_pm_get_device_info"

external open_input : int -> int -> ('a -> int) option -> 'a option -> stream = "caml_pm_open_input"

let open_input ?(time_proc=None) ?(time_info=None) device_id buffer_size =
    open_input device_id buffer_size time_proc time_info

external open_output : int -> int -> ('a -> int) option -> 'a option -> int -> stream = "caml_pm_open_output"

let open_output ?(time_proc=None) ?(time_info=None) device_id buffer_size latency =
    open_output device_id buffer_size time_proc time_info latency

external abort : stream -> unit = "caml_pm_abort"

external close : stream -> unit = "caml_pm_close"

type event = {
    message : Int32.t;
    timestamp : Int32.t 
}

let message status data1 data2 =
    let s = Int32.of_int (status land 0xFF) in
    let d1 = Int32.of_int ((data1 lsl 8) land 0xFF00) in
    let d2 = Int32.logand (Int32.of_int (data2 lsl 16)) (Int32.of_int 0xFF0000) in
    Int32.logor s (Int32.logor d1 d2)

let message_status msg =
    Int32.to_int (Int32.logand msg (Int32.of_int 0xFF))

let message_data1 msg =
    Int32.to_int (Int32.logand (Int32.shift_right msg 8) (Int32.of_int 0xFF))

let message_data2 msg =
    Int32.to_int (Int32.logand (Int32.shift_right msg 16) (Int32.of_int 0xFF))

let message_contents msg =
    (message_status msg), (message_data1 msg), (message_data2 msg)

external poll : stream -> bool = "caml_pm_poll"

external read : stream -> event array -> int -> int -> int = "caml_pm_read"

external write : stream -> event array -> int -> int -> unit = "caml_pm_write"

external write_short : stream -> Int32.t -> Int32.t -> unit = "caml_pm_write_short"

external pt_start : int -> unit = "caml_pt_start"

external pt_stop : unit -> unit = "caml_pt_stop"

external pt_time : unit -> Int32.t = "caml_pt_time"
