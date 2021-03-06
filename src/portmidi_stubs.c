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


static int pm_errors[] = {
    pmHostError,
    pmInvalidDeviceId,
    pmInsufficientMemory,
    pmBufferTooSmall,
    pmBufferOverflow,
    pmBadPtr,
    pmBadData,
    pmInternalError,
    pmBufferMaxSize,
};

static int pm_err(int ret)
{
  int i;
  if (ret >= 0)
    return ret;

  for(i=0; i<sizeof(pm_errors); ++i)
  {
      if(ret == pm_errors[i])
          break;
  }

  caml_raise_with_arg(*caml_named_value("portmidi_exn_error"), Val_int(i));
}

CAMLprim value caml_pm_get_error_text(value error)
{
    CAMLparam1(error);
    CAMLreturn(caml_copy_string(Pm_GetErrorText(pm_errors[Int_val(error)])));
}

CAMLprim value caml_pm_initialize(value unit)
{
    int ret;
    ret = Pm_Initialize();
    pm_err(ret);
    return Val_unit;
}

CAMLprim value caml_pm_terminate(value unit)
{
    int ret;
    ret = Pm_Terminate();
    pm_err(ret);
    return Val_unit;
}

CAMLprim value caml_pm_count_devices(value unit)
{
    int ret;
    ret = Pm_CountDevices();
    pm_err(ret);
    return Val_int(ret);
}

CAMLprim value caml_pm_get_default_input_device_id(value unit)
{
    int ret;
    ret = Pm_GetDefaultInputDeviceID();
    pm_err(ret);
    return Val_int(ret);
}

CAMLprim value caml_pm_get_default_output_device_id(value unit)
{
    int ret;
    ret = Pm_GetDefaultOutputDeviceID();
    pm_err(ret);
    return Val_int(ret);
}

CAMLprim value caml_pm_get_device_info(value device_id)
{
    CAMLparam1(device_id);
    CAMLlocal1(ret);

    const PmDeviceInfo *info = Pm_GetDeviceInfo(Int_val(device_id));
    if(info == NULL)
    {
        /* raise with PmUnkown */
        caml_raise_with_arg(*caml_named_value("portmidi_exn_error"), Val_int(sizeof(pm_errors)));
    }
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
    if(ret < 0)
    {
        free(st);
        pm_err(ret);
    }
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
    if(ret < 0)
    {
        free(st);
        pm_err(ret);
    }
    ans = caml_alloc_custom(&stream_ops, sizeof(stream_t*), 1, 0);
    Stream_t_val(ans) = st;

    CAMLreturn(ans);
}

CAMLprim value caml_pm_abort(value st)
{
    CAMLparam1(st);
    int ret = Pm_Abort(Stream_val(st));
    pm_err(ret);
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_close(value st)
{
    CAMLparam1(st);
    int ret = Pm_Close(Stream_val(st));
    pm_err(ret);
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_poll(value st)
{
    CAMLparam1(st);
    int ret;
    ret = Pm_Poll(Stream_val(st));
    pm_err(ret);
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
    if(ret < 0)
    {
        free(buf);
        pm_err(ret);
    }
    for(i = 0; i < len; ++i)
    {
        field = caml_alloc_tuple(2);
        Field(field, 0) = caml_copy_int32(buf[i].message);
        Field(field, 1) = caml_copy_int32(buf[i].timestamp);
        Field(buffer, ofs + i) = field;
    }
    free(buf);

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
    free(buf);
    pm_err(ret);

    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_write_short(value st, value when, value msg)
{
    CAMLparam3(st, when, msg);
    int ret;
    ret = Pm_WriteShort(Stream_val(st), Int32_val(when), Int32_val(msg));
    pm_err(ret);
    CAMLreturn(Val_unit);
}

CAMLprim value caml_pm_write_sysex(value st, value when, value msg)
{
    CAMLparam3(st, when, msg);
    int ret;
    ret = Pm_WriteSysEx(Stream_val(st), Int32_val(when), (unsigned char*)String_val(msg));
    pm_err(ret);
    CAMLreturn(Val_unit);
}

/* PortTime functions */

static int pt_errors[] = {
    ptNoError,
    ptHostError,
    ptAlreadyStarted,
    ptAlreadyStopped,
    ptInsufficientMemory
};

static int pt_err(int ret)
{
  int i;
  if (ret >= 0)
    return ret;

  for(i=0; i<sizeof(pt_errors); ++i)
  {
      if(ret == pt_errors[i])
          break;
  }

  caml_raise_with_arg(*caml_named_value("porttime_exn_error"), Val_int(i));
}

CAMLprim value caml_pt_start(value resolution)
{
    int ret = Pt_Start(Int_val(resolution), NULL, NULL);
    pt_err(ret);
    return Val_unit;
}

CAMLprim value caml_pt_stop(value unit)
{
    int ret = Pt_Stop();
    pt_err(ret);
    return Val_unit;
}

CAMLprim value caml_pt_time(value unit)
{
    PtTimestamp ret;
    ret = Pt_Time();
    return caml_copy_int32(ret);
}

CAMLprim value caml_pt_sleep(value time)
{
    Pt_Sleep(Val_int(time));
    return Val_unit;
}

