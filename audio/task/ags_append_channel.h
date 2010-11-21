#ifndef __AGS_APPEND_CHANNEL_H__
#define __AGS_APPEND_CHANNEL_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_task.h>

#include <ags/audio/ags_devout.h>

#define AGS_TYPE_APPEND_CHANNEL                (ags_append_channel_get_type())
#define AGS_APPEND_CHANNEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPEND_CHANNEL, AgsAppendChannel))
#define AGS_APPEND_CHANNEL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_APPEND_CHANNEL, AgsAppendChannelClass))
#define AGS_IS_APPEND_CHANNEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_APPEND_CHANNEL))
#define AGS_IS_APPEND_CHANNEL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_APPEND_CHANNEL))
#define AGS_APPEND_CHANNEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_APPEND_CHANNEL, AgsAppendChannelClass))

typedef struct _AgsAppendChannel AgsAppendChannel;
typedef struct _AgsAppendChannelClass AgsAppendChannelClass;

struct _AgsAppendChannel
{
  AgsTask task;

  AgsDevout *devout;
  AgsDevoutPlay *devout_play;
};

struct _AgsAppendChannelClass
{
  AgsTaskClass task;
};

GType ags_append_channel_get_type();

AgsAppendChannel* ags_append_channel_new(AgsDevout *devout,
					 AgsDevoutPlay *devout_play);

#endif /*__AGS_APPEND_CHANNEL_H__*/
