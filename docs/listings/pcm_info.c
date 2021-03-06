#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

AgsApplicationContext *application_context;

GObject *soundcard;

GList *list;

guint channels_min, channels_max;
guint rate_min, rate_max;
guint buffer_size_min, buffer_size_max;

GError *error;

application_context = ags_application_context_get_instance();

list = ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context));

if(list != NULL){
  soundcard = G_OBJECT(list->data);

  error = NULL;
  ags_soundcard_pcm_info(AGS_SOUNDCARD(soundcard),
                         &channels_min, &channels_max,
                         &rate_min, &rate_max,
                         &buffer_size_min, &buffer_size_max,
                         &error);
  if(error != NULL){
    g_warning("%s\0", error->msg);
  }
}
