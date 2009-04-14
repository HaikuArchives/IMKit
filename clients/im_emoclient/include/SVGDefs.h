#ifndef _SVG_DEFS_H_
#define _SVG_DEFS_H_

#include <InterfaceDefs.h>
#include <String.h>
#include "Matrix.h"

// defines
#define EXPORT_LIB_VERSION		"0.5.0"
#define EXPORT_MAX_LINE_WIDTH	85
#define EXPORT_MIN_LINE_WIDTH	20

// some handy macros...
#define SETCOLOR24BIT(x, y)			{ x.red = (y >> 16) & 0xff; x.green = (y >> 8) & 0xff; x.blue = (y >> 0) & 0xff; x.alpha = 255; }
#define SETCOLOR32BIT(x, y)			{ x.red = (y >> 16) & 0xff; x.green = (y >> 8) & 0xff; x.blue = (y >> 0) & 0xff; x.alpha = (y >> 24) & 0xff; }
#define SETCOLORALLTO(x, y)			{ x.red = y; x.green = y; x.blue = y; x.alpha = y; }
#define SETCOLORRGB(x, r, g, b)		{ x.red = r; x.green = g; x.blue = b; x.alpha = 255; }
#define SETCOLORRGBA(x, r, g, b, a)	{ x.red = r; x.green = g; x.blue = b; x.alpha = a; }

#define SETINT32RGB(x, y)			{ x = (y.red << 16) + (y.green << 8) + y.blue; }
#define SETINT32RGBA(x, y)			{ x = (y.alpha << 24) + (y.red << 16) + (y.green << 8) + y.blue; }

#define INDENT(string, level)		for (int _indent_ = 0; _indent_ < level; _indent_++) string << "\t";

// archive macros
#define ADDREPCLASS(x)	if (data->HasString("class")) data->ReplaceString("class", x); else data->AddString("class", x);
#define ADDREPTYPE(x)	if (data->HasInt32("_type")) data->ReplaceInt32("_type", x); else data->AddInt32("_type", x);
#define FINALIZEIF(x)	int32 type; if (data->FindInt32("_type", &type) == B_OK && type == x) FinalizeShape(); ApplyTransformation();

enum shape_t {
	B_SVG_PATH = 0,
	B_SVG_RECT,
	B_SVG_CIRCLE,
	B_SVG_ELLIPSE,
	B_SVG_LINE,
	B_SVG_POLYLINE,
	B_SVG_POLYGON
};

enum gradient_t {
	B_LINEAR_GRADIENT = 0,
	B_RADIAL_GRADIENT
};

enum fill_t {
	B_FILL_UNKNOWN = -1,
	B_FILL_NONE = 0,
	B_FILL_COLOR,
	B_FILL_GRADIENT
};

enum target_t {
	B_TARGET_FILL = 0,
	B_TARGET_STROKE,
	B_TARGET_STOP
};

enum units_t {
	B_OBJECT_BOUNDING_BOX = 0,
	B_USERSPACE_ON_USE
};

struct in_state_s {
	in_state_s() :  fill_type(B_FILL_COLOR), fill_opacity(1), stroke_type(B_FILL_NONE),
					stroke_opacity(1), stroke_width(1), stroke_linecap(B_BUTT_CAP),
					stroke_linejoin(B_MITER_JOIN), stroke_miterlimit(4),
					stop_opacity(1),
					general_opacity(1), general_transform()
					{ SETCOLORALLTO(fill_color, 0); SETCOLORALLTO(stroke_color, 0);
					  SETCOLORALLTO(stop_color, 0); };
	// wrong:	the standard says the join mode defaults to a B_MITER_JOIN but
	//			there seems to be a bug	with beziers if we do set so!

	fill_t		fill_type;
	rgb_color	fill_color;
	BString		fill_gradient;
	float		fill_opacity;

	fill_t		stroke_type;
	rgb_color	stroke_color;
	BString		stroke_gradient;
	float		stroke_opacity;
	float		stroke_width;
	cap_mode	stroke_linecap;
	join_mode	stroke_linejoin;
	float		stroke_miterlimit;
	
	rgb_color	stop_color;
	float		stop_opacity;
	
	rgb_color	general_color;
	float		general_opacity;
	Matrix2D	general_transform;
};

#endif // _SVG_DEFS_H_
