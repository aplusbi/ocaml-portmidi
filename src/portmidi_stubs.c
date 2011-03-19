#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>

#include <portmidi.h>

typedef struct
{
    PortMidiStream *stream;
} stream_t;

#define Stream_t_val(v) (*((stream_t**)Data_custom_val(v)))
#define Stream_val(v) (Stream_t_val(v))->stream

static void finalize_stream(value s)
{
  stream_t *st = Stream_t_val(s);

  if (st->stream)
    Pm_Close(st->stream);
  free(st);
}

static struct custom_operations stream_ops =
{
    "caml_pa_stream",
    finalize_stream,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
};

CAMLprim value caml_pm_initialize(value unit)
{
    CAMLparam0();
    int ret = Pm_Initialize();
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_terminate(value unit)
{
    CAMLparam0();
    int ret = Pm_Terminate();
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_count_devices(value unit)
{
    CAMLparam0();
    int ret = Pm_CountDevices();
    CAMLreturn(Val_int(ret));
}

CAMLprim value caml_pm_get_default_input_device_id(value unit)
{
    CAMLparam0();
    int ret;
    ret = Pm_GetDefaultInputDeviceID();
    CAMLreturn(Val_int(ret));
}

CAMLprim value caml_pm_get_default_output_device_id(value unit)
{
    CAMLparam0();
    int ret;
    ret = Pm_GetDefaultOutputDeviceID();
    CAMLreturn(Val_int(ret));
}

CAMLprim value caml_pm_get_device_info(value device_id)
{
    CAMLparam1(device_id);
    value ret;

    PmDeviceInfo *info = Pm_GetDeviceInfo(Int_val(device_id));
    ret = caml_alloc_tuple(6);
    Field(ret, 0) = Val_int(info->structVersion);
    Field(ret, 1) = caml_copy_string(info->interf);
    Field(ret, 2) = caml_copy_string(info->name);
    Field(ret, 3) = Val_bool(info->input);
    Field(ret, 4) = Val_bool(info->output);
    Field(ret, 5) = Val_bool(info->opened);
    CAMLreturn(ret);
}

CAMLprim value caml_pm_open_input()
{
}

CAMLprim value caml_pm_open_output()
{
}

CAMLprim value caml_pm_abort(value st)
{
    CAMLparam1(st);
    Pm_Abort(Stream_val(st));
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_close(value st)
{
    CAMLparam1(st);
    Pm_Close(Stream_val(st));
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_read()
{
}

CAMLprim value caml_pm_write()
{
}

