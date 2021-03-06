
#include	<glib-object.h>


#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_schar (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#define g_marshal_value_peek_variant(v)  g_value_get_variant (v)
#else /* !G_ENABLE_DEBUG */
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          Do not access GValues directly in your code. Instead, use the
 *          g_value_get_*() functions
 */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_variant(v)  (v)->data[0].v_pointer
#endif /* !G_ENABLE_DEBUG */


/* VOID:BOOLEAN,POINTER (ags_marshallers.list:1) */
void
g_cclosure_user_marshal_VOID__BOOLEAN_POINTER (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__BOOLEAN_POINTER) (gpointer     data1,
                                                      gboolean     arg_1,
                                                      gpointer     arg_2,
                                                      gpointer     data2);
  GMarshalFunc_VOID__BOOLEAN_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__BOOLEAN_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_boolean (param_values + 1),
            g_marshal_value_peek_pointer (param_values + 2),
            data2);
}

/* VOID:INT,UINT,POINTER (ags_marshallers.list:2) */
void
g_cclosure_user_marshal_VOID__INT_UINT_POINTER (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__INT_UINT_POINTER) (gpointer     data1,
                                                       gint         arg_1,
                                                       guint        arg_2,
                                                       gpointer     arg_3,
                                                       gpointer     data2);
  GMarshalFunc_VOID__INT_UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__INT_UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_int (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_pointer (param_values + 3),
            data2);
}

/* VOID:UINT,BOOLEAN (ags_marshallers.list:3) */
void
g_cclosure_user_marshal_VOID__UINT_BOOLEAN (GClosure     *closure,
                                            GValue       *return_value G_GNUC_UNUSED,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint G_GNUC_UNUSED,
                                            gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_BOOLEAN) (gpointer     data1,
                                                   guint        arg_1,
                                                   gboolean     arg_2,
                                                   gpointer     data2);
  GMarshalFunc_VOID__UINT_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_boolean (param_values + 2),
            data2);
}

/* VOID:UINT,UINT (ags_marshallers.list:4) */
void
g_cclosure_user_marshal_VOID__UINT_UINT (GClosure     *closure,
                                         GValue       *return_value G_GNUC_UNUSED,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint G_GNUC_UNUSED,
                                         gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT) (gpointer     data1,
                                                guint        arg_1,
                                                guint        arg_2,
                                                gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/* VOID:UINT,UINT,BOOLEAN (ags_marshallers.list:5) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_BOOLEAN (GClosure     *closure,
                                                 GValue       *return_value G_GNUC_UNUSED,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint G_GNUC_UNUSED,
                                                 gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_BOOLEAN) (gpointer     data1,
                                                        guint        arg_1,
                                                        guint        arg_2,
                                                        gboolean     arg_3,
                                                        gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_boolean (param_values + 3),
            data2);
}

/* VOID:UINT,UINT,UINT (ags_marshallers.list:6) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_UINT (GClosure     *closure,
                                              GValue       *return_value G_GNUC_UNUSED,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint G_GNUC_UNUSED,
                                              gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_UINT) (gpointer     data1,
                                                     guint        arg_1,
                                                     guint        arg_2,
                                                     guint        arg_3,
                                                     gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:UINT,UINT,UINT,UINT (ags_marshallers.list:7) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT (GClosure     *closure,
                                                   GValue       *return_value G_GNUC_UNUSED,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint G_GNUC_UNUSED,
                                                   gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_UINT_UINT) (gpointer     data1,
                                                          guint        arg_1,
                                                          guint        arg_2,
                                                          guint        arg_3,
                                                          guint        arg_4,
                                                          gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 5);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            g_marshal_value_peek_uint (param_values + 4),
            data2);
}

/* VOID:UINT,UINT,UINT,UINT,UINT (ags_marshallers.list:8) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT_UINT (GClosure     *closure,
                                                        GValue       *return_value G_GNUC_UNUSED,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                                        gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT) (gpointer     data1,
                                                               guint        arg_1,
                                                               guint        arg_2,
                                                               guint        arg_3,
                                                               guint        arg_4,
                                                               guint        arg_5,
                                                               gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 6);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            g_marshal_value_peek_uint (param_values + 4),
            g_marshal_value_peek_uint (param_values + 5),
            data2);
}

/* VOID:UINT,UINT,UINT,UINT,UINT,UINT (ags_marshallers.list:9) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT_UINT_UINT (GClosure     *closure,
                                                             GValue       *return_value G_GNUC_UNUSED,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                                             gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT) (gpointer     data1,
                                                                    guint        arg_1,
                                                                    guint        arg_2,
                                                                    guint        arg_3,
                                                                    guint        arg_4,
                                                                    guint        arg_5,
                                                                    guint        arg_6,
                                                                    gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 7);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            g_marshal_value_peek_uint (param_values + 4),
            g_marshal_value_peek_uint (param_values + 5),
            g_marshal_value_peek_uint (param_values + 6),
            data2);
}

/* VOID:UINT,UINT,UINT,UINT,UINT,UINT,UINT (ags_marshallers.list:10) */
void
g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT_UINT_UINT_UINT (GClosure     *closure,
                                                                  GValue       *return_value G_GNUC_UNUSED,
                                                                  guint         n_param_values,
                                                                  const GValue *param_values,
                                                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                                                  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT_UINT) (gpointer     data1,
                                                                         guint        arg_1,
                                                                         guint        arg_2,
                                                                         guint        arg_3,
                                                                         guint        arg_4,
                                                                         guint        arg_5,
                                                                         guint        arg_6,
                                                                         guint        arg_7,
                                                                         gpointer     data2);
  GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 8);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT_UINT_UINT_UINT_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            g_marshal_value_peek_uint (param_values + 4),
            g_marshal_value_peek_uint (param_values + 5),
            g_marshal_value_peek_uint (param_values + 6),
            g_marshal_value_peek_uint (param_values + 7),
            data2);
}

/* VOID:UINT,DOUBLE,UINT (ags_marshallers.list:11) */
void
g_cclosure_user_marshal_VOID__UINT_DOUBLE_UINT (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_DOUBLE_UINT) (gpointer     data1,
                                                       guint        arg_1,
                                                       gdouble      arg_2,
                                                       guint        arg_3,
                                                       gpointer     data2);
  GMarshalFunc_VOID__UINT_DOUBLE_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_DOUBLE_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_double (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:UINT,INT (ags_marshallers.list:12) */
void
g_cclosure_user_marshal_VOID__UINT_INT (GClosure     *closure,
                                        GValue       *return_value G_GNUC_UNUSED,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                        gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_INT) (gpointer     data1,
                                               guint        arg_1,
                                               gint         arg_2,
                                               gpointer     data2);
  GMarshalFunc_VOID__UINT_INT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_INT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_int (param_values + 2),
            data2);
}

/* VOID:UINT,DOUBLE (ags_marshallers.list:13) */
void
g_cclosure_user_marshal_VOID__UINT_DOUBLE (GClosure     *closure,
                                           GValue       *return_value G_GNUC_UNUSED,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint G_GNUC_UNUSED,
                                           gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_DOUBLE) (gpointer     data1,
                                                  guint        arg_1,
                                                  gdouble      arg_2,
                                                  gpointer     data2);
  GMarshalFunc_VOID__UINT_DOUBLE callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_DOUBLE) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_uint (param_values + 1),
            g_marshal_value_peek_double (param_values + 2),
            data2);
}

/* VOID:ULONG,UINT (ags_marshallers.list:14) */
void
g_cclosure_user_marshal_VOID__ULONG_UINT (GClosure     *closure,
                                          GValue       *return_value G_GNUC_UNUSED,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint G_GNUC_UNUSED,
                                          gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__ULONG_UINT) (gpointer     data1,
                                                 gulong       arg_1,
                                                 guint        arg_2,
                                                 gpointer     data2);
  GMarshalFunc_VOID__ULONG_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__ULONG_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_ulong (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/* VOID:ULONG,UINT,BOOLEAN (ags_marshallers.list:15) */
void
g_cclosure_user_marshal_VOID__ULONG_UINT_BOOLEAN (GClosure     *closure,
                                                  GValue       *return_value G_GNUC_UNUSED,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                                  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__ULONG_UINT_BOOLEAN) (gpointer     data1,
                                                         gulong       arg_1,
                                                         guint        arg_2,
                                                         gboolean     arg_3,
                                                         gpointer     data2);
  GMarshalFunc_VOID__ULONG_UINT_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__ULONG_UINT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_ulong (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_boolean (param_values + 3),
            data2);
}

/* VOID:DOUBLE,DOUBLE (ags_marshallers.list:16) */
void
g_cclosure_user_marshal_VOID__DOUBLE_DOUBLE (GClosure     *closure,
                                             GValue       *return_value G_GNUC_UNUSED,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                             gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__DOUBLE_DOUBLE) (gpointer     data1,
                                                    gdouble      arg_1,
                                                    gdouble      arg_2,
                                                    gpointer     data2);
  GMarshalFunc_VOID__DOUBLE_DOUBLE callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__DOUBLE_DOUBLE) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_double (param_values + 1),
            g_marshal_value_peek_double (param_values + 2),
            data2);
}

/* VOID:ULONG,UINT,UINT (ags_marshallers.list:17) */
void
g_cclosure_user_marshal_VOID__ULONG_UINT_UINT (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__ULONG_UINT_UINT) (gpointer     data1,
                                                      gulong       arg_1,
                                                      guint        arg_2,
                                                      guint        arg_3,
                                                      gpointer     data2);
  GMarshalFunc_VOID__ULONG_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__ULONG_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_ulong (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:STRING,UINT (ags_marshallers.list:18) */
void
g_cclosure_user_marshal_VOID__STRING_UINT (GClosure     *closure,
                                           GValue       *return_value G_GNUC_UNUSED,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint G_GNUC_UNUSED,
                                           gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT) (gpointer     data1,
                                                  gpointer     arg_1,
                                                  guint        arg_2,
                                                  gpointer     data2);
  GMarshalFunc_VOID__STRING_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/* VOID:STRING,UINT,DOUBLE (ags_marshallers.list:19) */
void
g_cclosure_user_marshal_VOID__STRING_UINT_DOUBLE (GClosure     *closure,
                                                  GValue       *return_value G_GNUC_UNUSED,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint G_GNUC_UNUSED,
                                                  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT_DOUBLE) (gpointer     data1,
                                                         gpointer     arg_1,
                                                         guint        arg_2,
                                                         gdouble      arg_3,
                                                         gpointer     data2);
  GMarshalFunc_VOID__STRING_UINT_DOUBLE callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_UINT_DOUBLE) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_double (param_values + 3),
            data2);
}

/* VOID:STRING,UINT,POINTER (ags_marshallers.list:20) */
void
g_cclosure_user_marshal_VOID__STRING_UINT_POINTER (GClosure     *closure,
                                                   GValue       *return_value G_GNUC_UNUSED,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint G_GNUC_UNUSED,
                                                   gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT_POINTER) (gpointer     data1,
                                                          gpointer     arg_1,
                                                          guint        arg_2,
                                                          gpointer     arg_3,
                                                          gpointer     data2);
  GMarshalFunc_VOID__STRING_UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_pointer (param_values + 3),
            data2);
}

/* VOID:STRING,UINT,STRING,DOUBLE (ags_marshallers.list:21) */
void
g_cclosure_user_marshal_VOID__STRING_UINT_STRING_DOUBLE (GClosure     *closure,
                                                         GValue       *return_value G_GNUC_UNUSED,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint G_GNUC_UNUSED,
                                                         gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT_STRING_DOUBLE) (gpointer     data1,
                                                                gpointer     arg_1,
                                                                guint        arg_2,
                                                                gpointer     arg_3,
                                                                gdouble      arg_4,
                                                                gpointer     data2);
  GMarshalFunc_VOID__STRING_UINT_STRING_DOUBLE callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 5);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_UINT_STRING_DOUBLE) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_string (param_values + 3),
            g_marshal_value_peek_double (param_values + 4),
            data2);
}

/* VOID:STRING,POINTER (ags_marshallers.list:22) */
void
g_cclosure_user_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                              GValue       *return_value G_GNUC_UNUSED,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint G_GNUC_UNUSED,
                                              gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
  GMarshalFunc_VOID__STRING_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_pointer (param_values + 2),
            data2);
}

/* VOID:STRING,STRING,STRING (ags_marshallers.list:23) */
void
g_cclosure_user_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
                                                    GValue       *return_value G_GNUC_UNUSED,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                                    gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_STRING_STRING) (gpointer     data1,
                                                           gpointer     arg_1,
                                                           gpointer     arg_2,
                                                           gpointer     arg_3,
                                                           gpointer     data2);
  GMarshalFunc_VOID__STRING_STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_string (param_values + 2),
            g_marshal_value_peek_string (param_values + 3),
            data2);
}

/* VOID:POINTER,UINT (ags_marshallers.list:24) */
void
g_cclosure_user_marshal_VOID__POINTER_UINT (GClosure     *closure,
                                            GValue       *return_value G_GNUC_UNUSED,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint G_GNUC_UNUSED,
                                            gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_UINT) (gpointer     data1,
                                                   gpointer     arg_1,
                                                   guint        arg_2,
                                                   gpointer     data2);
  GMarshalFunc_VOID__POINTER_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_pointer (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/* VOID:POINTER,UINT,UINT (ags_marshallers.list:25) */
void
g_cclosure_user_marshal_VOID__POINTER_UINT_UINT (GClosure     *closure,
                                                 GValue       *return_value G_GNUC_UNUSED,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint G_GNUC_UNUSED,
                                                 gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_UINT_UINT) (gpointer     data1,
                                                        gpointer     arg_1,
                                                        guint        arg_2,
                                                        guint        arg_3,
                                                        gpointer     data2);
  GMarshalFunc_VOID__POINTER_UINT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_UINT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_pointer (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:POINTER,UINT,POINTER (ags_marshallers.list:26) */
void
g_cclosure_user_marshal_VOID__POINTER_UINT_POINTER (GClosure     *closure,
                                                    GValue       *return_value G_GNUC_UNUSED,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                                    gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_UINT_POINTER) (gpointer     data1,
                                                           gpointer     arg_1,
                                                           guint        arg_2,
                                                           gpointer     arg_3,
                                                           gpointer     data2);
  GMarshalFunc_VOID__POINTER_UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_pointer (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            g_marshal_value_peek_pointer (param_values + 3),
            data2);
}

/* VOID:POINTER,POINTER,UINT (ags_marshallers.list:27) */
void
g_cclosure_user_marshal_VOID__POINTER_POINTER_UINT (GClosure     *closure,
                                                    GValue       *return_value G_GNUC_UNUSED,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                                    gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_POINTER_UINT) (gpointer     data1,
                                                           gpointer     arg_1,
                                                           gpointer     arg_2,
                                                           guint        arg_3,
                                                           gpointer     data2);
  GMarshalFunc_VOID__POINTER_POINTER_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_POINTER_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_pointer (param_values + 1),
            g_marshal_value_peek_pointer (param_values + 2),
            g_marshal_value_peek_uint (param_values + 3),
            data2);
}

/* VOID:POINTER,POINTER,POINTER (ags_marshallers.list:28) */
void
g_cclosure_user_marshal_VOID__POINTER_POINTER_POINTER (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_POINTER_POINTER) (gpointer     data1,
                                                              gpointer     arg_1,
                                                              gpointer     arg_2,
                                                              gpointer     arg_3,
                                                              gpointer     data2);
  GMarshalFunc_VOID__POINTER_POINTER_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_POINTER_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_pointer (param_values + 1),
            g_marshal_value_peek_pointer (param_values + 2),
            g_marshal_value_peek_pointer (param_values + 3),
            data2);
}

/* VOID:OBJECT,UINT (ags_marshallers.list:29) */
void
g_cclosure_user_marshal_VOID__OBJECT_UINT (GClosure     *closure,
                                           GValue       *return_value G_GNUC_UNUSED,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint G_GNUC_UNUSED,
                                           gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_UINT) (gpointer     data1,
                                                  gpointer     arg_1,
                                                  guint        arg_2,
                                                  gpointer     data2);
  GMarshalFunc_VOID__OBJECT_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_uint (param_values + 2),
            data2);
}

/* VOID:OBJECT,OBJECT (ags_marshallers.list:30) */
void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                             GValue       *return_value G_GNUC_UNUSED,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                             gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
  GMarshalFunc_VOID__OBJECT_OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            data2);
}

/* VOID:OBJECT,OBJECT,OBJECT,INT,BOOLEAN (ags_marshallers.list:31) */
void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_INT_BOOLEAN (GClosure     *closure,
                                                                GValue       *return_value G_GNUC_UNUSED,
                                                                guint         n_param_values,
                                                                const GValue *param_values,
                                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                                gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_INT_BOOLEAN) (gpointer     data1,
                                                                       gpointer     arg_1,
                                                                       gpointer     arg_2,
                                                                       gpointer     arg_3,
                                                                       gint         arg_4,
                                                                       gboolean     arg_5,
                                                                       gpointer     data2);
  GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_INT_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 6);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_INT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            g_marshal_value_peek_object (param_values + 3),
            g_marshal_value_peek_int (param_values + 4),
            g_marshal_value_peek_boolean (param_values + 5),
            data2);
}

/* VOID:OBJECT,OBJECT,OBJECT,OBJECT,OBJECT,OBJECT,UINT,INT,BOOLEAN (ags_marshallers.list:32) */
void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_UINT_INT_BOOLEAN (GClosure     *closure,
                                                                                          GValue       *return_value G_GNUC_UNUSED,
                                                                                          guint         n_param_values,
                                                                                          const GValue *param_values,
                                                                                          gpointer      invocation_hint G_GNUC_UNUSED,
                                                                                          gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_UINT_INT_BOOLEAN) (gpointer     data1,
                                                                                                 gpointer     arg_1,
                                                                                                 gpointer     arg_2,
                                                                                                 gpointer     arg_3,
                                                                                                 gpointer     arg_4,
                                                                                                 gpointer     arg_5,
                                                                                                 gpointer     arg_6,
                                                                                                 guint        arg_7,
                                                                                                 gint         arg_8,
                                                                                                 gboolean     arg_9,
                                                                                                 gpointer     data2);
  GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_UINT_INT_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 10);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_UINT_INT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            g_marshal_value_peek_object (param_values + 3),
            g_marshal_value_peek_object (param_values + 4),
            g_marshal_value_peek_object (param_values + 5),
            g_marshal_value_peek_object (param_values + 6),
            g_marshal_value_peek_uint (param_values + 7),
            g_marshal_value_peek_int (param_values + 8),
            g_marshal_value_peek_boolean (param_values + 9),
            data2);
}

/* VOID:OBJECT,OBJECT,OBJECT,OBJECT,OBJECT,OBJECT,OBJECT,OBJECT (ags_marshallers.list:33) */
void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT (GClosure     *closure,
                                                                                       GValue       *return_value G_GNUC_UNUSED,
                                                                                       guint         n_param_values,
                                                                                       const GValue *param_values,
                                                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                                                       gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT) (gpointer     data1,
                                                                                              gpointer     arg_1,
                                                                                              gpointer     arg_2,
                                                                                              gpointer     arg_3,
                                                                                              gpointer     arg_4,
                                                                                              gpointer     arg_5,
                                                                                              gpointer     arg_6,
                                                                                              gpointer     arg_7,
                                                                                              gpointer     arg_8,
                                                                                              gpointer     data2);
  GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 9);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            g_marshal_value_peek_object (param_values + 3),
            g_marshal_value_peek_object (param_values + 4),
            g_marshal_value_peek_object (param_values + 5),
            g_marshal_value_peek_object (param_values + 6),
            g_marshal_value_peek_object (param_values + 7),
            g_marshal_value_peek_object (param_values + 8),
            data2);
}

/* ULONG:VOID (ags_marshallers.list:34) */
void
g_cclosure_user_marshal_ULONG__VOID (GClosure     *closure,
                                     GValue       *return_value G_GNUC_UNUSED,
                                     guint         n_param_values,
                                     const GValue *param_values,
                                     gpointer      invocation_hint G_GNUC_UNUSED,
                                     gpointer      marshal_data)
{
  typedef gulong (*GMarshalFunc_ULONG__VOID) (gpointer     data1,
                                              gpointer     data2);
  GMarshalFunc_ULONG__VOID callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gulong v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_ULONG__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_set_ulong (return_value, v_return);
}

/* INT:VOID (ags_marshallers.list:35) */
void
g_cclosure_user_marshal_INT__VOID (GClosure     *closure,
                                   GValue       *return_value G_GNUC_UNUSED,
                                   guint         n_param_values,
                                   const GValue *param_values,
                                   gpointer      invocation_hint G_GNUC_UNUSED,
                                   gpointer      marshal_data)
{
  typedef gint (*GMarshalFunc_INT__VOID) (gpointer     data1,
                                          gpointer     data2);
  GMarshalFunc_INT__VOID callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gint v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_INT__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_set_int (return_value, v_return);
}

/* UINT:VOID (ags_marshallers.list:36) */
void
g_cclosure_user_marshal_UINT__VOID (GClosure     *closure,
                                    GValue       *return_value G_GNUC_UNUSED,
                                    guint         n_param_values,
                                    const GValue *param_values,
                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                    gpointer      marshal_data)
{
  typedef guint (*GMarshalFunc_UINT__VOID) (gpointer     data1,
                                            gpointer     data2);
  GMarshalFunc_UINT__VOID callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  guint v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_UINT__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_set_uint (return_value, v_return);
}

/* BOOLEAN:UINT,POINTER (ags_marshallers.list:37) */
void
g_cclosure_user_marshal_BOOLEAN__UINT_POINTER (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__UINT_POINTER) (gpointer     data1,
                                                          guint        arg_1,
                                                          gpointer     arg_2,
                                                          gpointer     data2);
  GMarshalFunc_BOOLEAN__UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_uint (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* BOOLEAN:OBJECT,OBJECT (ags_marshallers.list:38) */
void
g_cclosure_user_marshal_BOOLEAN__OBJECT_OBJECT (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__OBJECT_OBJECT) (gpointer     data1,
                                                           gpointer     arg_1,
                                                           gpointer     arg_2,
                                                           gpointer     data2);
  GMarshalFunc_BOOLEAN__OBJECT_OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_object (param_values + 1),
                       g_marshal_value_peek_object (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

/* UINT:INT,UINT,POINTER (ags_marshallers.list:39) */
void
g_cclosure_user_marshal_UINT__INT_UINT_POINTER (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef guint (*GMarshalFunc_UINT__INT_UINT_POINTER) (gpointer     data1,
                                                        gint         arg_1,
                                                        guint        arg_2,
                                                        gpointer     arg_3,
                                                        gpointer     data2);
  GMarshalFunc_UINT__INT_UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  guint v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_UINT__INT_UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_int (param_values + 1),
                       g_marshal_value_peek_uint (param_values + 2),
                       g_marshal_value_peek_pointer (param_values + 3),
                       data2);

  g_value_set_uint (return_value, v_return);
}

/* DOUBLE:DOUBLE,BOOLEAN (ags_marshallers.list:40) */
void
g_cclosure_user_marshal_DOUBLE__DOUBLE_BOOLEAN (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef gdouble (*GMarshalFunc_DOUBLE__DOUBLE_BOOLEAN) (gpointer     data1,
                                                          gdouble      arg_1,
                                                          gboolean     arg_2,
                                                          gpointer     data2);
  GMarshalFunc_DOUBLE__DOUBLE_BOOLEAN callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gdouble v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_DOUBLE__DOUBLE_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_double (param_values + 1),
                       g_marshal_value_peek_boolean (param_values + 2),
                       data2);

  g_value_set_double (return_value, v_return);
}

/* STRING:STRING,STRING (ags_marshallers.list:41) */
void
g_cclosure_user_marshal_STRING__STRING_STRING (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef gchar* (*GMarshalFunc_STRING__STRING_STRING) (gpointer     data1,
                                                        gpointer     arg_1,
                                                        gpointer     arg_2,
                                                        gpointer     data2);
  GMarshalFunc_STRING__STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gchar* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_STRING__STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_string (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       data2);

  g_value_take_string (return_value, v_return);
}

/* STRING:ULONG,STRING,STRING,STRING,STRING,UINT,POINTER (ags_marshallers.list:42) */
void
g_cclosure_user_marshal_STRING__ULONG_STRING_STRING_STRING_STRING_UINT_POINTER (GClosure     *closure,
                                                                                GValue       *return_value G_GNUC_UNUSED,
                                                                                guint         n_param_values,
                                                                                const GValue *param_values,
                                                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                                                gpointer      marshal_data)
{
  typedef gchar* (*GMarshalFunc_STRING__ULONG_STRING_STRING_STRING_STRING_UINT_POINTER) (gpointer     data1,
                                                                                         gulong       arg_1,
                                                                                         gpointer     arg_2,
                                                                                         gpointer     arg_3,
                                                                                         gpointer     arg_4,
                                                                                         gpointer     arg_5,
                                                                                         guint        arg_6,
                                                                                         gpointer     arg_7,
                                                                                         gpointer     data2);
  GMarshalFunc_STRING__ULONG_STRING_STRING_STRING_STRING_UINT_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gchar* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 8);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_STRING__ULONG_STRING_STRING_STRING_STRING_UINT_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_ulong (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       g_marshal_value_peek_string (param_values + 3),
                       g_marshal_value_peek_string (param_values + 4),
                       g_marshal_value_peek_string (param_values + 5),
                       g_marshal_value_peek_uint (param_values + 6),
                       g_marshal_value_peek_pointer (param_values + 7),
                       data2);

  g_value_take_string (return_value, v_return);
}

/* POINTER:VOID (ags_marshallers.list:43) */
void
g_cclosure_user_marshal_POINTER__VOID (GClosure     *closure,
                                       GValue       *return_value G_GNUC_UNUSED,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                       gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__VOID) (gpointer     data1,
                                                  gpointer     data2);
  GMarshalFunc_POINTER__VOID callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:UINT (ags_marshallers.list:44) */
void
g_cclosure_user_marshal_POINTER__UINT (GClosure     *closure,
                                       GValue       *return_value G_GNUC_UNUSED,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                       gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__UINT) (gpointer     data1,
                                                  guint        arg_1,
                                                  gpointer     data2);
  GMarshalFunc_POINTER__UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__UINT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_uint (param_values + 1),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:POINTER,UINT (ags_marshallers.list:45) */
void
g_cclosure_user_marshal_POINTER__POINTER_UINT (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__POINTER_UINT) (gpointer     data1,
                                                          gpointer     arg_1,
                                                          guint        arg_2,
                                                          gpointer     data2);
  GMarshalFunc_POINTER__POINTER_UINT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__POINTER_UINT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       g_marshal_value_peek_uint (param_values + 2),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:STRING,STRING (ags_marshallers.list:46) */
void
g_cclosure_user_marshal_POINTER__STRING_STRING (GClosure     *closure,
                                                GValue       *return_value G_GNUC_UNUSED,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint G_GNUC_UNUSED,
                                                gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__STRING_STRING) (gpointer     data1,
                                                           gpointer     arg_1,
                                                           gpointer     arg_2,
                                                           gpointer     data2);
  GMarshalFunc_POINTER__STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_string (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:STRING,STRING,STRING (ags_marshallers.list:47) */
void
g_cclosure_user_marshal_POINTER__STRING_STRING_STRING (GClosure     *closure,
                                                       GValue       *return_value G_GNUC_UNUSED,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint G_GNUC_UNUSED,
                                                       gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__STRING_STRING_STRING) (gpointer     data1,
                                                                  gpointer     arg_1,
                                                                  gpointer     arg_2,
                                                                  gpointer     arg_3,
                                                                  gpointer     data2);
  GMarshalFunc_POINTER__STRING_STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_string (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       g_marshal_value_peek_string (param_values + 3),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:OBJECT,STRING,STRING,STRING,STRING,POINTER (ags_marshallers.list:48) */
void
g_cclosure_user_marshal_POINTER__OBJECT_STRING_STRING_STRING_STRING_POINTER (GClosure     *closure,
                                                                             GValue       *return_value G_GNUC_UNUSED,
                                                                             guint         n_param_values,
                                                                             const GValue *param_values,
                                                                             gpointer      invocation_hint G_GNUC_UNUSED,
                                                                             gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__OBJECT_STRING_STRING_STRING_STRING_POINTER) (gpointer     data1,
                                                                                        gpointer     arg_1,
                                                                                        gpointer     arg_2,
                                                                                        gpointer     arg_3,
                                                                                        gpointer     arg_4,
                                                                                        gpointer     arg_5,
                                                                                        gpointer     arg_6,
                                                                                        gpointer     data2);
  GMarshalFunc_POINTER__OBJECT_STRING_STRING_STRING_STRING_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 7);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__OBJECT_STRING_STRING_STRING_STRING_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_object (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       g_marshal_value_peek_string (param_values + 3),
                       g_marshal_value_peek_string (param_values + 4),
                       g_marshal_value_peek_string (param_values + 5),
                       g_marshal_value_peek_pointer (param_values + 6),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* POINTER:POINTER,STRING,STRING (ags_marshallers.list:49) */
void
g_cclosure_user_marshal_POINTER__POINTER_STRING_STRING (GClosure     *closure,
                                                        GValue       *return_value G_GNUC_UNUSED,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                                        gpointer      marshal_data)
{
  typedef gpointer (*GMarshalFunc_POINTER__POINTER_STRING_STRING) (gpointer     data1,
                                                                   gpointer     arg_1,
                                                                   gpointer     arg_2,
                                                                   gpointer     arg_3,
                                                                   gpointer     data2);
  GMarshalFunc_POINTER__POINTER_STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  gpointer v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_POINTER__POINTER_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_pointer (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       g_marshal_value_peek_string (param_values + 3),
                       data2);

  g_value_set_pointer (return_value, v_return);
}

/* OBJECT:VOID (ags_marshallers.list:50) */
void
g_cclosure_user_marshal_OBJECT__VOID (GClosure     *closure,
                                      GValue       *return_value G_GNUC_UNUSED,
                                      guint         n_param_values,
                                      const GValue *param_values,
                                      gpointer      invocation_hint G_GNUC_UNUSED,
                                      gpointer      marshal_data)
{
  typedef GObject* (*GMarshalFunc_OBJECT__VOID) (gpointer     data1,
                                                 gpointer     data2);
  GMarshalFunc_OBJECT__VOID callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  GObject* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_OBJECT__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_take_object (return_value, v_return);
}

/* OBJECT:OBJECT (ags_marshallers.list:51) */
void
g_cclosure_user_marshal_OBJECT__OBJECT (GClosure     *closure,
                                        GValue       *return_value G_GNUC_UNUSED,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                        gpointer      marshal_data)
{
  typedef GObject* (*GMarshalFunc_OBJECT__OBJECT) (gpointer     data1,
                                                   gpointer     arg_1,
                                                   gpointer     data2);
  GMarshalFunc_OBJECT__OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  GObject* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_OBJECT__OBJECT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_object (param_values + 1),
                       data2);

  g_value_take_object (return_value, v_return);
}

/* OBJECT:STRING,STRING,STRING (ags_marshallers.list:52) */
void
g_cclosure_user_marshal_OBJECT__STRING_STRING_STRING (GClosure     *closure,
                                                      GValue       *return_value G_GNUC_UNUSED,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint G_GNUC_UNUSED,
                                                      gpointer      marshal_data)
{
  typedef GObject* (*GMarshalFunc_OBJECT__STRING_STRING_STRING) (gpointer     data1,
                                                                 gpointer     arg_1,
                                                                 gpointer     arg_2,
                                                                 gpointer     arg_3,
                                                                 gpointer     data2);
  GMarshalFunc_OBJECT__STRING_STRING_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  GObject* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_OBJECT__STRING_STRING_STRING) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_string (param_values + 1),
                       g_marshal_value_peek_string (param_values + 2),
                       g_marshal_value_peek_string (param_values + 3),
                       data2);

  g_value_take_object (return_value, v_return);
}

/* OBJECT:OBJECT,POINTER,POINTER (ags_marshallers.list:53) */
void
g_cclosure_user_marshal_OBJECT__OBJECT_POINTER_POINTER (GClosure     *closure,
                                                        GValue       *return_value G_GNUC_UNUSED,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                                        gpointer      marshal_data)
{
  typedef GObject* (*GMarshalFunc_OBJECT__OBJECT_POINTER_POINTER) (gpointer     data1,
                                                                   gpointer     arg_1,
                                                                   gpointer     arg_2,
                                                                   gpointer     arg_3,
                                                                   gpointer     data2);
  GMarshalFunc_OBJECT__OBJECT_POINTER_POINTER callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  GObject* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_OBJECT__OBJECT_POINTER_POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_object (param_values + 1),
                       g_marshal_value_peek_pointer (param_values + 2),
                       g_marshal_value_peek_pointer (param_values + 3),
                       data2);

  g_value_take_object (return_value, v_return);
}

/* OBJECT:OBJECT,OBJECT (ags_marshallers.list:54) */
void
g_cclosure_user_marshal_OBJECT__OBJECT_OBJECT (GClosure     *closure,
                                               GValue       *return_value G_GNUC_UNUSED,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint G_GNUC_UNUSED,
                                               gpointer      marshal_data)
{
  typedef GObject* (*GMarshalFunc_OBJECT__OBJECT_OBJECT) (gpointer     data1,
                                                          gpointer     arg_1,
                                                          gpointer     arg_2,
                                                          gpointer     data2);
  GMarshalFunc_OBJECT__OBJECT_OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;
  GObject* v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_OBJECT__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_marshal_value_peek_object (param_values + 1),
                       g_marshal_value_peek_object (param_values + 2),
                       data2);

  g_value_take_object (return_value, v_return);
}

