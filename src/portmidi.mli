(**
* Bindings for the portmidi library.
*
* @author Niki Yoshiuchi
*)

(** {2 Exceptions} *)
(** Types of errors *)
type error =
    PmHostError
    | PmInvalidDeviceId
    | PmInsufficientMemory
    | PmBufferTooSmall
    | PmBufferOverflow
    | PmBadPtr
    | PmBadData
    | PmInternalError
    | PmBufferMaxSize
    | PmUnkown

(** An error occured.  Use [string_of_error] to get a description of the
  * error. *)
exception Error of error

(** Get a description of an error. *)
external string_of_error : error -> string = "caml_pm_get_error_text"

(** {2 General} *)
(** Initialize the portmidi library.  Call this before opening any streams *)
external init : unit -> unit = "caml_pm_initialize" "noalloc"

(** Close the library *)
external terminate : unit -> unit = "caml_pm_terminate" "noalloc"

(** {2 Devices} *)

(** Device information type *)
type device_info = {
  struct_version : int;
  interf : string;
  name : string;
  input : bool;
  output : bool;
  opened : bool;
}

(** [count_devices ()] returns the number of MIDI devices available *)
external count_devices : unit -> int = "caml_pm_count_devices" "noalloc"

(** [get_default_input_device_id ()] returns the default input MIDI device *)
external get_default_input_device_id : unit -> int
  = "caml_pm_get_default_input_device_id" "noalloc"
  
(** [get_default_output_device_id ()] returns the default output MIDI device *)
external get_default_output_device_id : unit -> int
  = "caml_pm_get_default_output_device_id" "noalloc"

(** [get_device_info id] returns a [device_info] record for device [id] *)
external get_device_info : int -> device_info = "caml_pm_get_device_info"

(** {2 Streams} *)

(** Stream type *)
type stream

(** [open_input ?time_proc:(Some time_fun) ?time_info:(Some time_arg) device_id buffer_size]
 * opens a MIDI stream ready for input on device [device_id] with a buffer of
 * [buffer_size].  [time_fun] is a function that returns a time stamp in
 * milliseconds, [time_args] is the argument passed to [time_fun].  If they are
 * set to [None] then the default time function is used. *)
val open_input :
  ?time_proc:('a -> int) option ->
  ?time_info:'a option -> int -> int -> stream

(** [open_output ?time_proc:(Some time_fun) ?time_info:(Some time_arg) device_id * buffer_size latency]
 * opens a MIDI stream ready for ouput on device [device_id] with a buffer of
 * [buffer_size].  [latency] is the delay in milliseconds applied to the time
 * stamp. *)
val open_output :
  ?time_proc:('a -> int) option ->
  ?time_info:'a option -> int -> int -> int -> stream

(** Abort a stream *)
external abort : stream -> unit = "caml_pm_abort"

(** Close a stream *)
external close : stream -> unit = "caml_pm_close"

(** {2 Events} *)

(** Event type *)
type event = { message : Int32.t; timestamp : Int32.t; }

(** [message status data1 data2] creates a message *)
val message : int -> int -> int -> int32

(** [message_status msg] returns the status of message [msg] *)
val message_status : int32 -> int

(** [message_data1 msg] returns the first data byte of message [msg] *)
val message_data1 : int32 -> int

(** [message_data2 msg] returns the second data byte of message [msg] *)
val message_data2 : int32 -> int

(** [message_contents msg] returns the status and data bytes of message [msg] *)
val message_contents : int32 -> int * int * int

(** [poll st] returns true of the input stream [st] has any events *)
external poll : stream -> bool = "caml_pm_poll"

(** [read_stream st events ofs len] reads up to [len] events from stream [st]
 * and stores them in [events] starting at [ofs].  The return value is the
 * actual number of events read. *)
external read_stream : stream -> event array -> int -> int -> int = "caml_pm_read"

(** [write_stream st events ofs len] writes [len] events to the stream [st] from
 * [events] starting at [ofs]. *)
external write_stream : stream -> event array -> int -> int -> unit
  = "caml_pm_write"

(** [write_short st when msg] writes the event formed by message [msg] and
* timestamp [when] to stream [st] *)
external write_short : stream -> Int32.t -> Int32.t -> unit
  = "caml_pm_write_short"

(** [write_short st when msg] writes the SysEx formed by message [msg] and
* timestamp [when] to stream [st] *)
external write_sysex : stream -> Int32.t -> string -> unit
  = "caml_pm_write_sysex"

module Time :
  sig
    (** {2 Exceptions} *)
    (** Types of errors *)
    type error =
        PtHostError  (** a system-specific error occurred *)
        | PtAlreadyStarted (** cannot start timer because it is already started *)
        | PtAlreadyStopped (** cannot stop timer because it is already stopped *)
        | PtInsufficientMemory (** memory could not be allocated *)
        | PtUnkown (** Unkown error *)

    (** An error occured *)
    exception Error of error

    (** {2 General} *)
    (** [start res] starts the timer with  resolution [res]
     * in milliseconds. *)
    external start : int -> unit = "caml_pt_start" "noalloc"

    (** Stop the timer *)
    external stop : unit -> unit = "caml_pt_stop" "noalloc"
    
    (** Get the time in milliseconds *)
    external time : unit -> Int32.t = "caml_pt_time" "noalloc"

    (** [sleep time] sleeps for [time] milliseconds *)
    external sleep : int -> unit = "caml_pt_sleep" "noalloc"
  end
