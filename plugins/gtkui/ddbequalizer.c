/* ddbequalizer.c generated by valac, the Vala compiler
 * generated from ddbequalizer.vala, do not modify */

/*
    SuperEQ GTK Widget for for DeaDBeeF
    Copyright (C) 2010 Viktor Semykin <thesame.ml@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/* sripped down and polished by Alexey Yakovenko <waker@users.sourceforge.net>*/

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <float.h>
#include <math.h>
#include <gdk/gdk.h>
#include <drawing.h>
#include <pango/pango.h>


#define DDB_TYPE_EQUALIZER (ddb_equalizer_get_type ())
#define DDB_EQUALIZER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), DDB_TYPE_EQUALIZER, DdbEqualizer))
#define DDB_EQUALIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), DDB_TYPE_EQUALIZER, DdbEqualizerClass))
#define DDB_IS_EQUALIZER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DDB_TYPE_EQUALIZER))
#define DDB_IS_EQUALIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DDB_TYPE_EQUALIZER))
#define DDB_EQUALIZER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), DDB_TYPE_EQUALIZER, DdbEqualizerClass))

typedef struct _DdbEqualizer DdbEqualizer;
typedef struct _DdbEqualizerClass DdbEqualizerClass;
typedef struct _DdbEqualizerPrivate DdbEqualizerPrivate;
#define _gdk_cursor_unref0(var) ((var == NULL) ? NULL : (var = (gdk_cursor_unref (var), NULL)))
#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _pango_font_description_free0(var) ((var == NULL) ? NULL : (var = (pango_font_description_free (var), NULL)))

struct _DdbEqualizer {
	GtkDrawingArea parent_instance;
	DdbEqualizerPrivate * priv;
};

struct _DdbEqualizerClass {
	GtkDrawingAreaClass parent_class;
};

struct _DdbEqualizerPrivate {
	double* values;
	gint values_length1;
	gint _values_size_;
	double preamp;
	gint mouse_y;
	gboolean curve_hook;
	gboolean preamp_hook;
	gint margin_bottom;
	gint margin_left;
	GdkCursor* pointer_cursor;
};


static gpointer ddb_equalizer_parent_class = NULL;

GType ddb_equalizer_get_type (void);
#define DDB_EQUALIZER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DDB_TYPE_EQUALIZER, DdbEqualizerPrivate))
enum  {
	DDB_EQUALIZER_DUMMY_PROPERTY
};
#define DDB_EQUALIZER_bands 18
#define DDB_EQUALIZER_spot_size 3
static inline double ddb_equalizer_scale (DdbEqualizer* self, double val);
static gboolean ddb_equalizer_real_expose_event (GtkWidget* base, GdkEventExpose* event);
static gboolean ddb_equalizer_in_curve_area (DdbEqualizer* self, double x, double y);
static void ddb_equalizer_update_eq_drag (DdbEqualizer* self, double x, double y);
static gboolean ddb_equalizer_real_button_press_event (GtkWidget* base, GdkEventButton* event);
static gboolean ddb_equalizer_real_button_release_event (GtkWidget* base, GdkEventButton* event);
static gboolean ddb_equalizer_real_leave_notify_event (GtkWidget* base, GdkEventCrossing* event);
static gboolean ddb_equalizer_real_motion_notify_event (GtkWidget* base, GdkEventMotion* event);
void ddb_equalizer_set_band (DdbEqualizer* self, gint band, double v);
double ddb_equalizer_get_band (DdbEqualizer* self, gint band);
void ddb_equalizer_set_preamp (DdbEqualizer* self, double v);
double ddb_equalizer_get_preamp (DdbEqualizer* self);
void ddb_equalizer_color_changed (DdbEqualizer* self);
DdbEqualizer* ddb_equalizer_new (void);
DdbEqualizer* ddb_equalizer_construct (GType object_type);
static GObject * ddb_equalizer_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static void ddb_equalizer_finalize (GObject* obj);

const char* freqs[18] = {"55 Hz", "77 Hz", "110 Hz", "156 Hz", "220 Hz", "311 Hz", "440 Hz", "622 Hz", "880 Hz", "1.2 kHz", "1.8 kHz", "2.5 kHz", "3.5 kHz", "5 kHz", "7 kHz", "10 kHz", "14 kHz", "20 kHz"};


static gpointer _g_object_ref0 (gpointer self) {
	return self ? g_object_ref (self) : NULL;
}


static gboolean ddb_equalizer_real_expose_event (GtkWidget* base, GdkEventExpose* event) {
	DdbEqualizer * self;
	gboolean result = FALSE;
	GdkColor _tmp0_ = {0};
	GdkColor fore_bright_color;
	GdkColor c1;
	GdkColor _tmp1_ = {0};
	GdkColor c2;
	GdkColor fore_dark_color;
	gint width;
	gint height;
	GdkDrawable* d;
	GdkGCValues _tmp3_;
	GdkGCValues _tmp2_ = {0};
	GdkGC* gc;
	double step;
	gint i = 0;
	double vstep;
	PangoLayout* l;
	PangoContext* ctx;
	PangoFontDescription* fd;
	gint fontsize;
	gboolean _tmp7_ = FALSE;
	char* tmp;
	double val;
	const char* _tmp9_;
	char* _tmp10_;
	const char* _tmp11_;
	char* _tmp12_;
	GdkRectangle _tmp14_;
	GdkRectangle _tmp13_ = {0};
	gint count;
	GdkRectangle _tmp17_;
	GdkRectangle _tmp16_ = {0};
	gint bar_w;
	GdkRectangle _tmp23_;
	GdkRectangle _tmp22_ = {0};
	self = (DdbEqualizer*) base;
	fore_bright_color = (gtkui_get_bar_foreground_color (&_tmp0_), _tmp0_);
	c1 = fore_bright_color;
	c2 = (gtkui_get_bar_background_color (&_tmp1_), _tmp1_);
	fore_dark_color = c2;
	fore_dark_color.red = fore_dark_color.red + ((guint16) ((gint16) ((c1.red - c2.red) * 0.5)));
	fore_dark_color.green = fore_dark_color.green + ((guint16) ((gint16) ((c1.green - c2.green) * 0.5)));
	fore_dark_color.blue = fore_dark_color.blue + ((guint16) ((gint16) ((c1.blue - c2.blue) * 0.5)));
	width = ((GtkWidget*) self)->allocation.width;
	height = ((GtkWidget*) self)->allocation.height;
	d = _g_object_ref0 ((GdkDrawable*) gtk_widget_get_window ((GtkWidget*) self));
	gc = _g_object_ref0 (GDK_DRAWABLE_GET_CLASS (d)->create_gc (d, (_tmp3_ = (memset (&_tmp2_, 0, sizeof (GdkGCValues)), _tmp2_), &_tmp3_), 0));
	gdk_gc_set_rgb_fg_color (gc, &fore_dark_color);
	step = ((double) (width - self->priv->margin_left)) / ((double) (DDB_EQUALIZER_bands + 1));
	{
		gboolean _tmp4_;
		i = 0;
		_tmp4_ = TRUE;
		while (TRUE) {
			if (!_tmp4_) {
				i++;
			}
			_tmp4_ = FALSE;
			if (!(i < DDB_EQUALIZER_bands)) {
				break;
			}
			gdk_draw_line (d, gc, ((gint) ((i + 1) * step)) + self->priv->margin_left, 0, ((gint) ((i + 1) * step)) + self->priv->margin_left, height - self->priv->margin_bottom);
		}
	}
	vstep = (double) (height - self->priv->margin_bottom);
	{
		double di;
		di = (double) 0;
		{
			gboolean _tmp5_;
			_tmp5_ = TRUE;
			while (TRUE) {
				if (!_tmp5_) {
					di = di + 0.25;
				}
				_tmp5_ = FALSE;
				if (!(di < 2)) {
					break;
				}
				gdk_draw_line (d, gc, self->priv->margin_left, (gint) ((di - self->priv->preamp) * vstep), width, (gint) ((di - self->priv->preamp) * vstep));
			}
		}
	}
	gdk_gc_set_rgb_fg_color (gc, &fore_bright_color);
	l = gtk_widget_create_pango_layout ((GtkWidget*) self, NULL);
	ctx = _g_object_ref0 (pango_layout_get_context (l));
	fd = pango_font_description_copy (gtk_widget_get_style ((GtkWidget*) self)->font_desc);
	pango_font_description_set_size (fd, (gint) (pango_font_description_get_size (gtk_widget_get_style ((GtkWidget*) self)->font_desc) * 0.7));
	pango_context_set_font_description (ctx, fd);
	{
		gboolean _tmp6_;
		i = 0;
		_tmp6_ = TRUE;
		while (TRUE) {
			PangoRectangle ink = {0};
			PangoRectangle log = {0};
			gint offs;
			if (!_tmp6_) {
				i++;
			}
			_tmp6_ = FALSE;
			if (!(i < DDB_EQUALIZER_bands)) {
				break;
			}
			pango_layout_set_text (l, freqs[i], (gint) g_utf8_strlen (freqs[i], -1));
			pango_layout_get_pixel_extents (l, &ink, &log);
			offs = 2;
			if ((i % 2) != 0) {
				offs = offs + 2;
			}
			gdk_draw_layout (d, gc, (((gint) ((i + 1) * step)) + self->priv->margin_left) - (ink.width / 2), (height - self->priv->margin_bottom) + offs, l);
		}
	}
	pango_font_description_set_size (fd, (gint) pango_font_description_get_size (gtk_widget_get_style ((GtkWidget*) self)->font_desc));
	pango_context_set_font_description (ctx, fd);
	pango_layout_set_width (l, self->priv->margin_left - 1);
	pango_layout_set_alignment (l, PANGO_ALIGN_RIGHT);
	fontsize = (gint) ((pango_units_to_double (pango_font_description_get_size (fd)) * gdk_screen_get_resolution (gdk_screen_get_default ())) / 72);
	if (self->priv->mouse_y != (-1)) {
		_tmp7_ = self->priv->mouse_y < (height - self->priv->margin_bottom);
	} else {
		_tmp7_ = FALSE;
	}
	if (_tmp7_) {
		double db;
		const char* _tmp8_;
		char* tmp;
		db = ddb_equalizer_scale (self, ((double) (self->priv->mouse_y - 1)) / ((double) ((height - self->priv->margin_bottom) - 2)));
		_tmp8_ = NULL;
		if (db > 0) {
			_tmp8_ = "+";
		} else {
			_tmp8_ = "";
		}
		tmp = g_strdup_printf ("%s%.1fdB", _tmp8_, db);
		pango_layout_set_text (l, tmp, (gint) g_utf8_strlen (tmp, -1));
		gdk_draw_layout (d, gc, self->priv->margin_left - 1, self->priv->mouse_y - 3, l);
		_g_free0 (tmp);
	}
	tmp = NULL;
	val = ddb_equalizer_scale (self, (double) 1);
	_tmp9_ = NULL;
	if (val > 0) {
		_tmp9_ = "+";
	} else {
		_tmp9_ = "";
	}
	tmp = (_tmp10_ = g_strdup_printf ("%s%.1fdB", _tmp9_, val), _g_free0 (tmp), _tmp10_);
	pango_layout_set_text (l, tmp, (gint) g_utf8_strlen (tmp, -1));
	gdk_draw_layout (d, gc, self->priv->margin_left - 1, (height - self->priv->margin_bottom) - fontsize, l);
	val = ddb_equalizer_scale (self, (double) 0);
	_tmp11_ = NULL;
	if (val > 0) {
		_tmp11_ = "+";
	} else {
		_tmp11_ = "";
	}
	tmp = (_tmp12_ = g_strdup_printf ("%s%.1fdB", _tmp11_, val), _g_free0 (tmp), _tmp12_);
	pango_layout_set_text (l, tmp, (gint) g_utf8_strlen (tmp, -1));
	gdk_draw_layout (d, gc, self->priv->margin_left - 1, 1, l);
	pango_layout_set_text (l, "+0dB", 4);
	gdk_draw_layout (d, gc, self->priv->margin_left - 1, ((gint) ((1 - self->priv->preamp) * (height - self->priv->margin_bottom))) - (fontsize / 2), l);
	pango_layout_set_text (l, "preamp", 6);
	pango_layout_set_alignment (l, PANGO_ALIGN_LEFT);
	gdk_draw_layout (d, gc, 1, (height - self->priv->margin_bottom) + 2, l);
	gdk_draw_rectangle (d, gc, FALSE, self->priv->margin_left, 0, (width - self->priv->margin_left) - 1, (height - self->priv->margin_bottom) - 1);
	gdk_gc_set_line_attributes (gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
	gdk_gc_set_clip_rectangle (gc, (_tmp14_ = (_tmp13_.x = 0, _tmp13_.y = (gint) (self->priv->preamp * (height - self->priv->margin_bottom)), _tmp13_.width = 11, _tmp13_.height = height, _tmp13_), &_tmp14_));
	gdk_gc_set_rgb_fg_color (gc, &fore_bright_color);
	count = ((gint) ((height - self->priv->margin_bottom) / 6)) + 1;
	{
		gint j;
		j = 0;
		{
			gboolean _tmp15_;
			_tmp15_ = TRUE;
			while (TRUE) {
				if (!_tmp15_) {
					j++;
				}
				_tmp15_ = FALSE;
				if (!(j < count)) {
					break;
				}
				gdk_draw_rectangle (d, gc, TRUE, 1, ((height - self->priv->margin_bottom) - (j * 6)) - 6, 11, 4);
			}
		}
	}
	gdk_gc_set_clip_rectangle (gc, (_tmp17_ = (_tmp16_.x = self->priv->margin_left + 1, _tmp16_.y = 1, _tmp16_.width = (width - self->priv->margin_left) - 2, _tmp16_.height = (height - self->priv->margin_bottom) - 2, _tmp16_), &_tmp17_));
	gdk_gc_set_rgb_fg_color (gc, &fore_bright_color);
	bar_w = 11;
	if (step < bar_w) {
		bar_w = ((gint) step) - 1;
	}
	{
		gboolean _tmp18_;
		i = 0;
		_tmp18_ = TRUE;
		while (TRUE) {
			GdkRectangle _tmp20_;
			GdkRectangle _tmp19_ = {0};
			if (!_tmp18_) {
				i++;
			}
			_tmp18_ = FALSE;
			if (!(i < DDB_EQUALIZER_bands)) {
				break;
			}
			gdk_gc_set_clip_rectangle (gc, (_tmp20_ = (_tmp19_.x = (((gint) ((i + 1) * step)) + self->priv->margin_left) - (bar_w / 2), _tmp19_.y = (gint) (self->priv->values[i] * (height - self->priv->margin_bottom)), _tmp19_.width = 11, _tmp19_.height = height, _tmp19_), &_tmp20_));
			count = ((gint) (((height - self->priv->margin_bottom) * (1 - self->priv->values[i])) / 6)) + 1;
			{
				gint j;
				j = 0;
				{
					gboolean _tmp21_;
					_tmp21_ = TRUE;
					while (TRUE) {
						if (!_tmp21_) {
							j++;
						}
						_tmp21_ = FALSE;
						if (!(j < count)) {
							break;
						}
						gdk_draw_rectangle (d, gc, TRUE, (((gint) ((i + 1) * step)) + self->priv->margin_left) - (bar_w / 2), ((height - self->priv->margin_bottom) - (j * 6)) - 6, bar_w, 4);
					}
				}
			}
		}
	}
	gdk_gc_set_clip_rectangle (gc, (_tmp23_ = (_tmp22_.x = 0, _tmp22_.y = 0, _tmp22_.width = width, _tmp22_.height = height, _tmp22_), &_tmp23_));
	gdk_gc_set_line_attributes (gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
	gdk_draw_line (d, gc, self->priv->margin_left + 1, self->priv->mouse_y, width, self->priv->mouse_y);
	result = FALSE;
	_g_object_unref0 (d);
	_g_object_unref0 (gc);
	_g_object_unref0 (l);
	_g_object_unref0 (ctx);
	_pango_font_description_free0 (fd);
	_g_free0 (tmp);
	return result;
}


static inline double ddb_equalizer_scale (DdbEqualizer* self, double val) {
	double result = 0.0;
	double k;
	double d;
	g_return_val_if_fail (self != NULL, 0.0);
	k = (double) (-40);
	d = (double) 20;
	result = (((val + self->priv->preamp) - 0.5) * k) + d;
	return result;
}


static gboolean ddb_equalizer_in_curve_area (DdbEqualizer* self, double x, double y) {
	gboolean result = FALSE;
	gboolean _tmp0_ = FALSE;
	gboolean _tmp1_ = FALSE;
	gboolean _tmp2_ = FALSE;
	g_return_val_if_fail (self != NULL, FALSE);
	if (x > self->priv->margin_left) {
		_tmp2_ = x < (((GtkWidget*) self)->allocation.width - 1);
	} else {
		_tmp2_ = FALSE;
	}
	if (_tmp2_) {
		_tmp1_ = y > 1;
	} else {
		_tmp1_ = FALSE;
	}
	if (_tmp1_) {
		_tmp0_ = y < (((GtkWidget*) self)->allocation.height - self->priv->margin_bottom);
	} else {
		_tmp0_ = FALSE;
	}
	result = _tmp0_;
	return result;
}


static void ddb_equalizer_update_eq_drag (DdbEqualizer* self, double x, double y) {
	double band_width;
	gint band;
	gboolean _tmp0_ = FALSE;
	g_return_if_fail (self != NULL);
	band_width = ((double) (((GtkWidget*) self)->allocation.width - self->priv->margin_left)) / ((double) (DDB_EQUALIZER_bands + 1));
	band = (gint) floor (((x - self->priv->margin_left) / band_width) - 0.5);
	if (band < 0) {
		band = 0;
	}
	if (band >= DDB_EQUALIZER_bands) {
		band = band - 1;
	}
	if (band >= 0) {
		_tmp0_ = band < DDB_EQUALIZER_bands;
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		self->priv->values[band] = y / ((double) (((GtkWidget*) self)->allocation.height - self->priv->margin_bottom));
		if (self->priv->values[band] > 1) {
			self->priv->values[band] = (double) 1;
		} else {
			if (self->priv->values[band] < 0) {
				self->priv->values[band] = (double) 0;
			}
		}
		g_signal_emit_by_name (self, "on-changed");
	}
}


static gboolean ddb_equalizer_real_button_press_event (GtkWidget* base, GdkEventButton* event) {
	DdbEqualizer * self;
	gboolean result = FALSE;
	gboolean _tmp0_ = FALSE;
	gboolean _tmp1_ = FALSE;
	gboolean _tmp2_ = FALSE;
	self = (DdbEqualizer*) base;
	if (ddb_equalizer_in_curve_area (self, (double) ((gint) (*event).x), (double) ((gint) (*event).y))) {
		self->priv->curve_hook = TRUE;
		ddb_equalizer_update_eq_drag (self, (double) ((gint) (*event).x), (double) ((gint) (*event).y));
		self->priv->mouse_y = (gint) (*event).y;
		gtk_widget_queue_draw ((GtkWidget*) self);
		result = FALSE;
		return result;
	}
	if ((*event).x <= 11) {
		_tmp2_ = (*event).y > 1;
	} else {
		_tmp2_ = FALSE;
	}
	if (_tmp2_) {
		_tmp1_ = (*event).y <= (((GtkWidget*) self)->allocation.height - self->priv->margin_bottom);
	} else {
		_tmp1_ = FALSE;
	}
	if (_tmp1_) {
		_tmp0_ = (*event).button == 1;
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		self->priv->preamp = (*event).y / ((double) (((GtkWidget*) self)->allocation.height - self->priv->margin_bottom));
		g_signal_emit_by_name (self, "on-changed");
		self->priv->preamp_hook = TRUE;
		self->priv->mouse_y = (gint) (*event).y;
		gtk_widget_queue_draw ((GtkWidget*) self);
	}
	result = FALSE;
	return result;
}


static gboolean ddb_equalizer_real_button_release_event (GtkWidget* base, GdkEventButton* event) {
	DdbEqualizer * self;
	gboolean result = FALSE;
	self = (DdbEqualizer*) base;
	self->priv->curve_hook = FALSE;
	self->priv->preamp_hook = FALSE;
	gdk_window_set_cursor (gtk_widget_get_window ((GtkWidget*) self), self->priv->pointer_cursor);
	result = FALSE;
	return result;
}


static gboolean ddb_equalizer_real_leave_notify_event (GtkWidget* base, GdkEventCrossing* event) {
	DdbEqualizer * self;
	gboolean result = FALSE;
	self = (DdbEqualizer*) base;
	self->priv->mouse_y = -1;
	gtk_widget_queue_draw ((GtkWidget*) self);
	result = FALSE;
	return result;
}


static gboolean ddb_equalizer_real_motion_notify_event (GtkWidget* base, GdkEventMotion* event) {
	DdbEqualizer * self;
	gboolean result = FALSE;
	double y;
	self = (DdbEqualizer*) base;
	y = (*event).y / ((double) (((GtkWidget*) self)->allocation.height - self->priv->margin_bottom));
	if (y < 0) {
		y = (double) 0;
	}
	if (y > 1) {
		y = (double) 1;
	}
	if (self->priv->preamp_hook) {
		self->priv->preamp = y;
		g_signal_emit_by_name (self, "on-changed");
		gtk_widget_queue_draw ((GtkWidget*) self);
		result = FALSE;
		return result;
	}
	if (!ddb_equalizer_in_curve_area (self, (double) ((gint) (*event).x), (double) ((gint) (*event).y))) {
		self->priv->mouse_y = -1;
	} else {
		self->priv->mouse_y = (gint) (*event).y;
	}
	if (self->priv->curve_hook) {
		ddb_equalizer_update_eq_drag (self, (double) ((gint) (*event).x), (double) ((gint) (*event).y));
		self->priv->mouse_y = (gint) (*event).y;
	}
	gtk_widget_queue_draw ((GtkWidget*) self);
	result = FALSE;
	return result;
}


void ddb_equalizer_set_band (DdbEqualizer* self, gint band, double v) {
	g_return_if_fail (self != NULL);
	self->priv->values[band] = 1 - ((v + 20.0) / 40.0);
}


double ddb_equalizer_get_band (DdbEqualizer* self, gint band) {
	double result = 0.0;
	g_return_val_if_fail (self != NULL, 0.0);
	result = ((1 - self->priv->values[band]) * 40.0) - 20.0;
	return result;
}


void ddb_equalizer_set_preamp (DdbEqualizer* self, double v) {
	g_return_if_fail (self != NULL);
	self->priv->preamp = 1 - ((v + 20.0) / 40.0);
}


double ddb_equalizer_get_preamp (DdbEqualizer* self) {
	double result = 0.0;
	g_return_val_if_fail (self != NULL, 0.0);
	result = ((1 - self->priv->preamp) * 40.0) - 20.0;
	return result;
}


void ddb_equalizer_color_changed (DdbEqualizer* self) {
	GdkColor _tmp1_;
	GdkColor _tmp0_ = {0};
	g_return_if_fail (self != NULL);
	gtkui_init_theme_colors ();
	gtk_widget_modify_bg ((GtkWidget*) self, GTK_STATE_NORMAL, (_tmp1_ = (gtkui_get_bar_background_color (&_tmp0_), _tmp0_), &_tmp1_));
}


DdbEqualizer* ddb_equalizer_construct (GType object_type) {
	DdbEqualizer * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


DdbEqualizer* ddb_equalizer_new (void) {
	return ddb_equalizer_construct (DDB_TYPE_EQUALIZER);
}


static GObject * ddb_equalizer_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GObjectClass * parent_class;
	DdbEqualizer * self;
	parent_class = G_OBJECT_CLASS (ddb_equalizer_parent_class);
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = DDB_EQUALIZER (obj);
	{
		gtk_widget_add_events ((GtkWidget*) self, (gint) (((GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK) | GDK_LEAVE_NOTIFY_MASK) | GDK_POINTER_MOTION_MASK));
		self->priv->margin_bottom = (gint) (((pango_units_to_double (pango_font_description_get_size (gtk_widget_get_style ((GtkWidget*) self)->font_desc)) * gdk_screen_get_resolution (gdk_screen_get_default ())) / 72) + 4);
		self->priv->margin_left = self->priv->margin_bottom * 4;
		ddb_equalizer_color_changed (self);
	}
	return obj;
}


static void ddb_equalizer_class_init (DdbEqualizerClass * klass) {
	ddb_equalizer_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (DdbEqualizerPrivate));
	GTK_WIDGET_CLASS (klass)->expose_event = ddb_equalizer_real_expose_event;
	GTK_WIDGET_CLASS (klass)->button_press_event = ddb_equalizer_real_button_press_event;
	GTK_WIDGET_CLASS (klass)->button_release_event = ddb_equalizer_real_button_release_event;
	GTK_WIDGET_CLASS (klass)->leave_notify_event = ddb_equalizer_real_leave_notify_event;
	GTK_WIDGET_CLASS (klass)->motion_notify_event = ddb_equalizer_real_motion_notify_event;
	G_OBJECT_CLASS (klass)->constructor = ddb_equalizer_constructor;
	G_OBJECT_CLASS (klass)->finalize = ddb_equalizer_finalize;
	g_signal_new ("on_changed", DDB_TYPE_EQUALIZER, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void ddb_equalizer_instance_init (DdbEqualizer * self) {
	self->priv = DDB_EQUALIZER_GET_PRIVATE (self);
	self->priv->values = g_new0 (double, DDB_EQUALIZER_bands);
	self->priv->values_length1 = DDB_EQUALIZER_bands;
	self->priv->_values_size_ = self->priv->values_length1;
	self->priv->preamp = 0.5;
	self->priv->curve_hook = FALSE;
	self->priv->preamp_hook = FALSE;
	self->priv->margin_bottom = -1;
	self->priv->margin_left = -1;
	self->priv->pointer_cursor = gdk_cursor_new (GDK_LEFT_PTR);
}


static void ddb_equalizer_finalize (GObject* obj) {
	DdbEqualizer * self;
	self = DDB_EQUALIZER (obj);
	self->priv->values = (g_free (self->priv->values), NULL);
	_gdk_cursor_unref0 (self->priv->pointer_cursor);
	G_OBJECT_CLASS (ddb_equalizer_parent_class)->finalize (obj);
}


GType ddb_equalizer_get_type (void) {
	static volatile gsize ddb_equalizer_type_id__volatile = 0;
	if (g_once_init_enter (&ddb_equalizer_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (DdbEqualizerClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ddb_equalizer_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (DdbEqualizer), 0, (GInstanceInitFunc) ddb_equalizer_instance_init, NULL };
		GType ddb_equalizer_type_id;
		ddb_equalizer_type_id = g_type_register_static (GTK_TYPE_DRAWING_AREA, "DdbEqualizer", &g_define_type_info, 0);
		g_once_init_leave (&ddb_equalizer_type_id__volatile, ddb_equalizer_type_id);
	}
	return ddb_equalizer_type_id__volatile;
}




