#ifndef __GSTPAKSRC_H__
#define __GSTPAKSRC_H__

#include <sys/types.h>

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

#include "../PakLib/PakInterface.h"

G_BEGIN_DECLS

#define GST_TYPE_PAK_SRC                        \
        (gst_pak_src_get_type())
#define GST_PAK_SRC(obj)                                                \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PAK_SRC,GstPakSrc))
#define GST_PAK_SRC_CLASS(klass)                                        \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PAK_SRC,GstPakSrcClass))
#define GST_IS_PAK_SRC(obj)                                     \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PAK_SRC))
#define GST_IS_PAK_SRC_CLASS(klass)                             \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PAK_SRC))

typedef struct _GstPakSrc GstPakSrc;
typedef struct _GstPakSrcClass GstPakSrcClass;

struct _GstPakSrc {
        GstBaseSrc element;

        PFILE *fp;
        gchar *filename;			/* filename */
        gchar *uri;				/* caching the URI */
        guint64 read_position;		        /* position of fd */
        guint64 file_size;

        gboolean seekable;                      /* whether the file is seekable */
        gboolean is_regular;                    /* whether it's a (symlink to a)
                                                   regular file */
};

struct _GstPakSrcClass {
        GstBaseSrcClass parent_class;
};

GType gst_pak_src_get_type (void);

void gst_pak_src_plugin_register (void);

G_END_DECLS

#endif
