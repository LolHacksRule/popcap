#include <gst/gst.h>
#include "GstPakSrc.h"


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifndef _
#define _(x) x
#endif

static GstStaticPadTemplate srctemplate =
	GST_STATIC_PAD_TEMPLATE ("src",
				 GST_PAD_SRC,
				 GST_PAD_ALWAYS,
				 GST_STATIC_CAPS_ANY);

#ifndef S_ISREG
#define S_ISREG(mode) ((mode) & _S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode) ((mode) & _S_IFDIR)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(x) (0)
#endif
#ifndef O_BINARY
#define O_BINARY (0)
#endif

GST_DEBUG_CATEGORY_STATIC (gst_pak_src_debug);
#define GST_CAT_DEFAULT gst_pak_src_debug

/* PakSrc signals and args */
enum
{
	LAST_SIGNAL
};

#define DEFAULT_BLOCKSIZE       4 * 1024

enum
{
	ARG_0,
	ARG_LOCATION,
};

static void gst_pak_src_finalize (GObject * object);

static void gst_pak_src_set_property (GObject * object, guint prop_id,
				      const GValue * value, GParamSpec * pspec);
static void gst_pak_src_get_property (GObject * object, guint prop_id,
				      GValue * value, GParamSpec * pspec);

static gboolean gst_pak_src_start (GstBaseSrc * basesrc);
static gboolean gst_pak_src_stop (GstBaseSrc * basesrc);

static gboolean gst_pak_src_is_seekable (GstBaseSrc * src);
static gboolean gst_pak_src_get_size (GstBaseSrc * src, guint64 * size);
static GstFlowReturn gst_pak_src_create (GstBaseSrc * src, guint64 offset,
					 guint length, GstBuffer ** buffer);

static void gst_pak_src_uri_handler_init (gpointer g_iface,
					  gpointer iface_data);

static void
_do_init (GType paksrc_type)
{
	static const GInterfaceInfo urihandler_info = {
		gst_pak_src_uri_handler_init,
		NULL,
		NULL
	};

	g_type_add_interface_static (paksrc_type, GST_TYPE_URI_HANDLER,
				     &urihandler_info);
	GST_DEBUG_CATEGORY_INIT (gst_pak_src_debug, "paksrc", 0, "paksrc element");
}

GST_BOILERPLATE_FULL (GstPakSrc, gst_pak_src, GstBaseSrc, GST_TYPE_BASE_SRC,
		      _do_init);

static void
gst_pak_src_base_init (gpointer g_class)
{
	GstElementClass *gstelement_class = GST_ELEMENT_CLASS (g_class);

	gst_element_class_set_details_simple (gstelement_class,
					      "Pak Source",
					      "Source/Pak",
					      "Read from arbitrary point in a file from pop package file",
					      "Luo Jinghua <sunmoon1997@gmail.com>");
	gst_element_class_add_pad_template (gstelement_class,
					    gst_static_pad_template_get (&srctemplate));
}

static void
gst_pak_src_class_init (GstPakSrcClass * klass)
{
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;
	GstBaseSrcClass *gstbasesrc_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gstelement_class = GST_ELEMENT_CLASS (klass);
	gstbasesrc_class = GST_BASE_SRC_CLASS (klass);

	gobject_class->set_property = gst_pak_src_set_property;
	gobject_class->get_property = gst_pak_src_get_property;

	g_object_class_install_property (gobject_class, ARG_LOCATION,
					 g_param_spec_string ("location", "File Location",
							      "Location of the file to read", NULL,
							      (GParamFlags)(G_PARAM_READWRITE)));

	gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_pak_src_finalize);

	gstbasesrc_class->start = GST_DEBUG_FUNCPTR (gst_pak_src_start);
	gstbasesrc_class->stop = GST_DEBUG_FUNCPTR (gst_pak_src_stop);
	gstbasesrc_class->is_seekable = GST_DEBUG_FUNCPTR (gst_pak_src_is_seekable);
	gstbasesrc_class->get_size = GST_DEBUG_FUNCPTR (gst_pak_src_get_size);
	gstbasesrc_class->create = GST_DEBUG_FUNCPTR (gst_pak_src_create);
}

static void
gst_pak_src_init (GstPakSrc * src, GstPakSrcClass * g_class)
{
	src->fp = NULL;
	src->filename = NULL;
	src->uri = NULL;
	src->is_regular = FALSE;
}

static void
gst_pak_src_finalize (GObject * object)
{
	GstPakSrc *src;

	src = GST_PAK_SRC (object);

	g_free (src->filename);
	g_free (src->uri);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_pak_src_set_location (GstPakSrc * src, const gchar * location)
{
	GstState state;

	/* the element must be stopped in order to do this */
	GST_OBJECT_LOCK (src);
	state = GST_STATE (src);
	if (state != GST_STATE_READY && state != GST_STATE_NULL)
		goto wrong_state;
	GST_OBJECT_UNLOCK (src);

	g_free (src->filename);
	g_free (src->uri);

	/* clear the filename if we get a NULL (is that possible?) */
	if (location == NULL) {
		src->filename = NULL;
		src->uri = NULL;
	} else {
		/* we store the filename as received by the application. On Windoes this
		 * should be UTF8 */
		src->filename = g_strdup (location);
		src->uri = gst_uri_construct ("pak", src->filename);
	}
	g_object_notify (G_OBJECT (src), "location");
	gst_uri_handler_new_uri (GST_URI_HANDLER (src), src->uri);

	return TRUE;

	/* ERROR */
 wrong_state:
	{
		GST_DEBUG_OBJECT (src, "setting location in wrong state");
		GST_OBJECT_UNLOCK (src);
		return FALSE;
	}
}

static void
gst_pak_src_set_property (GObject * object, guint prop_id,
			  const GValue * value, GParamSpec * pspec)
{
	GstPakSrc *src;

	g_return_if_fail (GST_IS_PAK_SRC (object));

	src = GST_PAK_SRC (object);

	switch (prop_id) {
	case ARG_LOCATION:
		gst_pak_src_set_location (src, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gst_pak_src_get_property (GObject * object, guint prop_id, GValue * value,
			  GParamSpec * pspec)
{
	GstPakSrc *src;

	g_return_if_fail (GST_IS_PAK_SRC (object));

	src = GST_PAK_SRC (object);

	switch (prop_id) {
	case ARG_LOCATION:
		g_value_set_string (value, src->filename);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static GstFlowReturn
gst_pak_src_create_read (GstPakSrc * src, guint64 offset, guint length,
			 GstBuffer ** buffer)
{
	int ret;
	GstBuffer *buf;

	if (G_UNLIKELY (src->read_position != offset)) {
		off_t res;

		res = p_fseek (src->fp, offset, SEEK_SET);
		if (G_UNLIKELY (res < 0))
			goto seek_failed;

		src->read_position = offset;
	}

	buf = gst_buffer_new_and_alloc (length);

	GST_LOG_OBJECT (src, "Reading %d bytes", length);
	ret = p_fread (GST_BUFFER_DATA (buf), 1, length, src->fp);
	if (G_UNLIKELY (ret < 0))
		goto could_not_read;

	/* seekable regular files should have given us what we expected */
	if (G_UNLIKELY ((guint) ret < length && src->seekable))
		goto unexpected_eos;

	/* other files should eos if they read 0 and more was requested */
	if (G_UNLIKELY (ret == 0 && length > 0))
		goto eos;

	length = ret;

	GST_BUFFER_SIZE (buf) = length;
	GST_BUFFER_OFFSET (buf) = offset;
	GST_BUFFER_OFFSET_END (buf) = offset + length;

	*buffer = buf;

	src->read_position += length;

	return GST_FLOW_OK;

	/* ERROR */
 seek_failed:
	{
		GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL), GST_ERROR_SYSTEM);
		return GST_FLOW_ERROR;
	}
 could_not_read:
	{
		GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL), GST_ERROR_SYSTEM);
		gst_buffer_unref (buf);
		return GST_FLOW_ERROR;
	}
 unexpected_eos:
	{
		GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL),
				   ("unexpected end of file."));
		gst_buffer_unref (buf);
		return GST_FLOW_ERROR;
	}
 eos:
	{
		GST_DEBUG ("non-regular file hits EOS");
		gst_buffer_unref (buf);
		return GST_FLOW_UNEXPECTED;
	}
}

static GstFlowReturn
gst_pak_src_create (GstBaseSrc * basesrc, guint64 offset, guint length,
		    GstBuffer ** buffer)
{
	GstPakSrc *src;
	GstFlowReturn ret;

	src = GST_PAK_SRC (basesrc);
	ret = gst_pak_src_create_read (src, offset, length, buffer);

	return ret;
}

static gboolean
gst_pak_src_is_seekable (GstBaseSrc * basesrc)
{
	GstPakSrc *src = GST_PAK_SRC (basesrc);

	return src->seekable;
}

static gboolean
gst_pak_src_get_size (GstBaseSrc * basesrc, guint64 * size)
{
	GstPakSrc *src;

	src = GST_PAK_SRC (basesrc);

	if (!src->seekable) {
		/* If it isn't seekable, we won't know the length (but fstat will still
		 * succeed, and wrongly say our length is zero. */
		return FALSE;
	}

	*size = src->file_size;

	return TRUE;
}

/* open the file and mmap it, necessary to go to READY state */
static gboolean
gst_pak_src_start (GstBaseSrc * basesrc)
{
	GstPakSrc *src = GST_PAK_SRC (basesrc);
	int res;

	if (src->filename == NULL || src->filename[0] == '\0')
		goto no_filename;

	GST_INFO_OBJECT (src, "opening file %s", src->filename);

	/* open the file */
	src->fp = p_fopen (src->filename, "rb");
	if (!src->fp)
		goto open_failed;

	src->read_position = 0;
	src->is_regular = TRUE;

	res = p_fseek (src->fp, 0, SEEK_END);
	if (res < 0)
		src->seekable = FALSE;
	else
		src->seekable = TRUE;
	src->file_size = p_ftell (src->fp);
	p_fseek (src->fp, 0, SEEK_SET);

	/* We can only really do seeking on regular files - for other file types, we
	 * don't know their length, so seeking isn't useful/meaningful */
	src->seekable = src->seekable && src->is_regular;

	return TRUE;

	/* ERROR */
 no_filename:
	{
		GST_ELEMENT_ERROR (src, RESOURCE, NOT_FOUND,
				   (_("No file name specified for reading.")), (NULL));
		return FALSE;
	}
 open_failed:
	{
		switch (errno) {
		case ENOENT:
			GST_ELEMENT_ERROR (src, RESOURCE, NOT_FOUND, (NULL),
					   ("No such file \"%s\"", src->filename));
			break;
		default:
			GST_ELEMENT_ERROR (src, RESOURCE, OPEN_READ,
					   (_("Could not open file \"%s\" for reading."), src->filename),
					   GST_ERROR_SYSTEM);
			break;
		}
		return FALSE;
	}
}

/* unmap and close the file */
static gboolean
gst_pak_src_stop (GstBaseSrc * basesrc)
{
	GstPakSrc *src = GST_PAK_SRC (basesrc);

	/* close the file */
	p_fclose (src->fp);

	/* zero out a lot of our state */
	src->fp = 0;
	src->is_regular = FALSE;

	return TRUE;
}

/*** GSTURIHANDLER INTERFACE *************************************************/

static GstURIType
gst_pak_src_uri_get_type (void)
{
	return GST_URI_SRC;
}

static gchar **
gst_pak_src_uri_get_protocols (void)
{
	static gchar *protocols[] = { "pak", NULL };

	return protocols;
}

static const gchar *
gst_pak_src_uri_get_uri (GstURIHandler * handler)
{
	GstPakSrc *src = GST_PAK_SRC (handler);

	return src->uri;
}

static gboolean
gst_pak_src_uri_set_uri (GstURIHandler * handler, const gchar * uri)
{
	gchar *protocol, *location;
	gboolean ret;
	GstPakSrc *src = GST_PAK_SRC (handler);

	protocol = gst_uri_get_protocol (uri);
	if (strcmp (protocol, "pak") != 0) {
		g_free (protocol);
		return FALSE;
	}
	g_free (protocol);

	if (strcmp (uri, "pak://") == 0) {
		/* Special case for "file://" as this is used by some applications
		 *  to test with gst_element_make_from_uri if there's an element
		 *  that supports the URI protocol. */
		gst_pak_src_set_location (src, NULL);
		return TRUE;
	} else {
		location = gst_uri_get_location (uri);
	}

	if (!location)
		return FALSE;

	ret = gst_pak_src_set_location (src, location);
	g_free (location);

	return ret;
}

static void
gst_pak_src_uri_handler_init (gpointer g_iface, gpointer iface_data)
{
	GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

	iface->get_type = gst_pak_src_uri_get_type;
	iface->get_protocols = gst_pak_src_uri_get_protocols;
	iface->get_uri = gst_pak_src_uri_get_uri;
	iface->set_uri = gst_pak_src_uri_set_uri;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

	gst_element_register (plugin, "paksrc", GST_RANK_PRIMARY,
			      gst_pak_src_get_type ());

	return TRUE;
}

void gst_pak_src_plugin_register (void)
{
	static bool registered = false;

	if (!registered)
	{
		gboolean ret = gst_plugin_register_static (GST_VERSION_MAJOR,
							   GST_VERSION_MINOR,
							   "paksrc",
							   "pak source",
							   plugin_init,
							   "0.1",
							   "BSD",
							   "",
							   "paksrc",
							   "");
		g_assert (ret == TRUE);
		registered = true;
	}
}
