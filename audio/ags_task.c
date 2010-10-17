#include "ags_task.h"

#include "../object/ags_connectable.h"

void ags_task_class_init(AgsTaskClass *task);
void ags_task_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_task_init(AgsTask *task);
void ags_task_finalize(GObject *gobject);
void ags_task_connect(AgsConnectable *connectable);
void ags_task_disconnect(AgsConnectable *connectable);

void ags_task_real_launch(AgsTask *task);
void ags_task_real_passed(AgsTask *task);

enum{
  LAUNCH,
  PASSED,
  LAST_SIGNAL,
};

static gpointer ags_task_parent_class = NULL;
static guint task_signals[LAST_SIGNAL];

GType
ags_task_get_type()
{
  static GType ags_type_task = 0;

  if(!ags_type_task){
    static const GTypeInfo ags_task_info = {
      sizeof (AgsTaskClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_task_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsTask),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_task_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_task_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_task = g_type_register_static(G_TYPE_OBJECT,
					   "AgsTask\0",
					   &ags_task_info,
					   0);

    g_type_add_interface_static(ags_type_task,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_task);
}

void
ags_task_class_init(AgsTaskClass *task)
{
  GObjectClass *gobject;

  ags_task_parent_class = g_type_class_peek_parent(task);

  gobject = (GObjectClass *) task;

  gobject->finalize = ags_task_finalize;

  task->launch = NULL;
  task->passed = NULL;

  task_signals[LAUNCH] =
    g_signal_new("launch\0",
		 G_TYPE_FROM_CLASS (task),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsTaskClass, launch),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  task_signals[PASSED] =
    g_signal_new("passed\0",
		 G_TYPE_FROM_CLASS (task),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsTaskClass, passed),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

void
ags_task_connectable_interface_init(AgsConnectableInterface *connectable)
{
}

void
ags_task_init(AgsTask *task)
{
  task->name = NULL;

  task->start = 0;
}

void
ags_task_finalize(GObject *gobject)
{
  /* empty */
}

void
ags_task_launch(AgsTask *task)
{
  g_return_if_fail(AGS_IS_TASK(task));

  g_object_ref(G_OBJECT(task));
  g_signal_emit(G_OBJECT(task),
		task_signals[LAUNCH], 0);
  g_object_unref(G_OBJECT(task));
}

void
ags_task_passed(AgsTask *task)
{
  g_return_if_fail(AGS_IS_TASK(task));

  g_object_ref(G_OBJECT(task));
  g_signal_emit(G_OBJECT(task),
		task_signals[PASSED], 0);
  g_object_unref(G_OBJECT(task));
}

AgsTask*
ags_task_new()
{
  AgsTask *task;

  task = (AgsTask *) g_object_new(AGS_TYPE_TASK, NULL);

  return(task);
}
