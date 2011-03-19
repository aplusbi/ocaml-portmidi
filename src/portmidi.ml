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

external open_input : int -> int -> ('a -> 'b) option -> 'a option -> stream = "caml_pm_open_input"

let open_input ?(time_proc=None) ?(time_info=None) device_id buffer_size =
    open_input device_id buffer_size time_proc time_info

external open_output : int -> int -> ('a -> 'b) option -> 'a option -> int -> stream = "caml_pm_open_output"

let open_output ?(time_proc=None) ?(time_info=None) device_id buffer_size latency =
    open_output device_id buffer_size time_proc time_info latency

external abort : stream -> unit = "caml_pm_abort"

external close : stream -> unit = "caml_pm_close"

type event = {
    message : Int32.t;
    timestamp : Int32.t 
}

external read : stream -> event array -> int -> int = "caml_pm_read"

external write : stream -> event array -> int -> int = "caml_pm_read"

