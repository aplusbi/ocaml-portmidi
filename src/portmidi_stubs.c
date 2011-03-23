#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>

#include <portmidi.h>
#include <porttime.h>

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
    int ret;
    ret = Pm_Initialize();
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_terminate(value unit)
{
    CAMLparam0();
    int ret;
    ret = Pm_Terminate();
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_count_devices(value unit)
{
    CAMLparam0();
    int ret;
    ret = Pm_CountDevices();
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
    CAMLlocal1(ret);

    const PmDeviceInfo *info = Pm_GetDeviceInfo(Int_val(device_id));
    ret = caml_alloc_tuple(6);
    Field(ret, 0) = Val_int(info->structVersion);
    Field(ret, 1) = caml_copy_string(info->interf);
    Field(ret, 2) = caml_copy_string(info->name);
    Field(ret, 3) = Val_bool(info->input);
    Field(ret, 4) = Val_bool(info->output);
    Field(ret, 5) = Val_bool(info->opened);
    CAMLreturn(ret);
}

CAMLprim value caml_pm_open_input(value device_id, value buffer_size, value time_proc, value time_info)
{
    CAMLparam4(device_id, buffer_size, time_proc, time_info);
    CAMLlocal1(ans);
    stream_t *st;
    int ret;

    st = malloc(sizeof(stream_t));
    ret = Pm_OpenInput(&st->stream, Int_val(device_id), NULL, Int_val(buffer_size), NULL, NULL);
    ans = caml_alloc_custom(&stream_ops, sizeof(stream_t*), 1, 0);
    Stream_t_val(ans) = st;

    CAMLreturn(ans);
}

CAMLprim value caml_pm_open_output(value device_id, value buffer_size, value time_proc, value time_info, value latency)
{
    CAMLparam5(device_id, buffer_size, time_proc, time_info, latency);
    CAMLlocal1(ans);
    stream_t *st;
    int ret;

    st = malloc(sizeof(stream_t));
    ret = Pm_OpenOutput(&st->stream, Int_val(device_id), NULL, Int_val(buffer_size), NULL, NULL, Int_val(latency));
    ans = caml_alloc_custom(&stream_ops, sizeof(stream_t*), 1, 0);
    Stream_t_val(ans) = st;

    CAMLreturn(ans);
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

CAMLprim value caml_pm_poll(value st)
{
    CAMLparam1(st);
    int ret;
    ret = Pm_Poll(Stream_val(st));
    CAMLreturn(Val_bool(ret));
}

CAMLprim value caml_pm_read(value st, value buffer, value _ofs, value _len)
{
    CAMLparam4(st, buffer, _ofs, _len);
    CAMLlocal1(field);
    PmEvent *buf;
    int len, ofs, i, ret;

    len = Int_val(_len);
    ofs = Int_val(_ofs);
    buf = malloc(sizeof(PmEvent) * len);
    ret = Pm_Read(Stream_val(st), buf, len);
    for(i = 0; i < len; ++i)
    {
        field = caml_alloc_tuple(2);
        Field(field, 0) = caml_copy_int32(buf[i].message);
        Field(field, 1) = caml_copy_int32(buf[i].timestamp);
        Field(buffer, ofs + i) = field;
    }

    CAMLreturn(Val_int(ret));
}

CAMLprim value caml_pm_write(value st, value buffer, value _ofs, value _len)
{
    CAMLparam4(st, buffer, _ofs, _len);
    CAMLlocal1(field);
    PmEvent *buf;
    int len, ofs, i, ret;

    len = Int_val(_len);
    ofs = Int_val(_ofs);
    buf = malloc(sizeof(PmEvent) * len);
    for(i = 0; i < len; ++i)
    {
        field = Field(buffer, ofs + i);
        buf[i].message = Int32_val(Field(field, 0));
        buf[i].timestamp = Int32_val(Field(field, 1));
    }
    ret = Pm_Write(Stream_val(st), buf, len);

    CAMLreturn(Val_unit);
}

/* PortTime functions */

CAMLprim value caml_pt_start(value resolution)
{
    CAMLparam1(resolution);
    Pt_Start(Int_val(resolution), NULL, NULL);
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pt_stop(value unit)
{
    CAMLparam0();
    Pt_Stop();
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pt_time(value unit)
{
    CAMLparam0();
    PtTimestamp ret;
    ret = Pt_Time();
    CAMLreturn(caml_copy_int32(ret));
}

