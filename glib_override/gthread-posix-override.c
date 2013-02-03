#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <glib.h>
 
//void *(*orig_g_private_set)(GPrivate *key,gpointer value);
//static pthread_key_t *(*g_private_get_impl) (GPrivate *key);
static pthread_key_t *g_private_get_impl (GPrivate *key);
static pthread_key_t *(*g_private_impl_new) (GDestroyNotify);
static void *(*g_private_impl_free) (pthread_key_t *);



void g_private_set(GPrivate *key, gpointer value)
{
	gint status;
	//printf("OVERRIDDEN g_private_set!\n");
//	orig_g_private_set(key,value);
	if G_UNLIKELY ((status = pthread_setspecific (*g_private_get_impl (key), value)) != 0)
		printf ("pthread_setspecific, possible race!?");
}
 

void
_init(void)
{
//	orig_g_private_set = dlsym(RTLD_NEXT, "g_private_set");
	g_private_impl_new = dlsym(RTLD_NEXT, "g_private_impl_new");
	g_private_impl_free = dlsym(RTLD_NEXT, "g_private_impl_free");
}

static pthread_key_t *g_private_get_impl (GPrivate *key)
{
  pthread_key_t *impl = key->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_private_impl_new (key->notify);
      if (!g_atomic_pointer_compare_and_exchange (&key->p, NULL, impl))
        {
          g_private_impl_free (impl);
          impl = key->p;
        }
    }

  return impl;
}

