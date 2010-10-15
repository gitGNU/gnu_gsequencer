#include "ags_runnable.h"

#include <stdio.h>

void ags_runnable_base_init(AgsRunnableInterface *interface);

GType
ags_runnable_get_type()
{
  static GType ags_type_runnable = 0;

  if(!ags_type_runnable){
    static const GTypeInfo ags_runnable_info = {
      sizeof(AgsRunnableInterface),
      (GBaseInitFunc) ags_runnable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_runnable = g_type_register_static(G_TYPE_INTERFACE,
					       "AgsRunnable\0", &ags_runnable_info,
					       0);
  }

  return(ags_type_runnable);
}

void
ags_runnable_base_init(AgsRunnableInterface *interface)
{
  /* empty */
}

void
ags_runnable_run(AgsRunnable *runnable)
{
  AgsRunnableInterface *runnable_interface;

  g_return_if_fail(AGS_IS_RUNNABLE(runnable));
  runnable_interface = AGS_RUNNABLE_GET_INTERFACE(runnable);
  g_return_if_fail(runnable_interface->run);
  runnable_interface->run(runnable);
}

void
ags_runnable_stop(AgsRunnable *runnable)
{
  AgsRunnableInterface *runnable_interface;

  g_return_if_fail(AGS_IS_RUNNABLE(runnable));
  runnable_interface = AGS_RUNNABLE_GET_INTERFACE(runnable);
  g_return_if_fail(runnable_interface->stop);
  runnable_interface->stop(runnable);
}
