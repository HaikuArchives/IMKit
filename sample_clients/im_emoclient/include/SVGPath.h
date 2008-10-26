#ifndef SVGPATH_H_
#define SVGPATH_H_

#include <String.h>
#include <GraphicsDefs.h>
#include "ObjectList.h"
#include <Shape.h>
#include <View.h>
#include "SVGDefs.h"
#include "Gradient.h"
#include "Matrix.h"

static void MakeValid(BRect *rect)
{
	BRect temp = *rect;
	rect->left = MIN(temp.left, temp.right);
	rect->right = MAX(temp.left, temp.right);
	rect->top = MIN(temp.top, temp.bottom);
	rect->bottom = MAX(temp.top, temp.bottom);
}

class BSVGView;

class BSVGPath : public BView {

public:
						BSVGPath(BSVGView *parent);
						BSVGPath(BSVGView *parent, const char *data);
						BSVGPath(BSVGView *parent, BString data);
						
virtual					~BSVGPath();

						BSVGPath(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_PATH; };

virtual	void			FinalizeShape();
		void			ApplyTransformation();

virtual	void			SetData(BString data);
		void			SetData(const char *data);
		BString			Data() const;
virtual	void			RecreateData();

virtual	void			SetParent(BSVGView *parent);
		BSVGView		*Parent();

virtual	void			SetFillGradient(BString id);
		void			SetFillGradient(const char *id);
		const char		*FillGradient();

virtual	void			SetFillColor(rgb_color color);
		void			SetFillColor(uchar r, uchar g, uchar b);
		rgb_color		FillColor();

virtual	void			SetFillType(fill_t type);
		fill_t			FillType();

virtual	void			SetFillOpacity(float opacity);
		float			FillOpacity();

virtual	void			SetStrokeGradient(BString id);
		void			SetStrokeGradient(const char *id);
		const char		*StrokeGradient();

virtual	void			SetStrokeColor(rgb_color color);
		void			SetStrokeColor(uchar r, uchar g, uchar b);
		rgb_color		StrokeColor();

virtual	void			SetStrokeType(fill_t type);
		fill_t			StrokeType();

virtual	void			SetStrokeWidth(float width);
		float			StrokeWidth();

virtual	void			SetStrokeLineJoin(join_mode mode);
		join_mode		StrokeLineJoin();

virtual	void			SetStrokeLineCap(cap_mode mode);
		cap_mode		StrokeLineCap();

virtual	void			SetStrokeMiterLimit(float limit);
		float			StrokeMiterLimit();

virtual	void			SetStrokeOpacity(float opacity);
		float			StrokeOpacity();

virtual	void			SetTransformation(Matrix2D *transformation);
		Matrix2D		Transformation();

virtual	void			SetState(in_state_s state);
		in_state_s		State();

		BShape			*Shape();
virtual	BRect			ShapeBounds();
		BShape			*TransformedShape();
virtual	BRect			TransformedShapeBounds();

		void			MoveTo(BPoint to);
		void			LineTo(BPoint to);
		void			HLineTo(float x);
		void			VLineTo(float y);
		void			CurveTo(BPoint control1, BPoint control2, BPoint to);
		void			ShortCurveTo(BPoint control, BPoint to);
		void			QuadBezierTo(BPoint control, BPoint to);
		void			ShortBezierTo(BPoint to);
		void			Close();

		BPoint			LastLocation();
		BPoint			LastControl();
		bool			LastWasCurve();
		bool			LastWasQuadBezier();

		void			Render(BView *view);
		void			Render(BWindow *window);
		void			Render(BBitmap *bitmap);	// must accept child views

		void			RenderCommon();

private:

friend	class BSVGRect;
friend	class BSVGEllipse;
friend	class BSVGCircle;
friend	class BSVGLine;
friend	class BSVGPolyline;
friend	class BSVGPolygon;

		BString			fData;
		BShape			fShape;
		BShape			fTransformedShape;
		BShape			*fRenderShape; // points to fShape or fTranslatedShape
		
		BSVGView		*fParent;	// were we are getting gradients from...
		float			fScale;
		
		BPoint			fLastLocation;	// the last pen position is cached
		BPoint			fLastControl;	// the last control point for shorthands
		bool			fLastWasCurve;	// tells us wether we can use fLastControl
		bool			fLastWasQuad;
		
		in_state_s		fState;
};

inline BSVGView *
BSVGPath::Parent()
{
	return fParent;
}

inline const char *
BSVGPath::FillGradient()
{
	return fState.fill_gradient.String();
}

inline fill_t
BSVGPath::FillType()
{
	return fState.fill_type;
}

inline const char *
BSVGPath::StrokeGradient()
{
	return fState.stroke_gradient.String();
}

inline fill_t
BSVGPath::StrokeType()
{
	return fState.stroke_type;
}

inline float
BSVGPath::FillOpacity()
{
	return fState.fill_opacity;
}

inline float
BSVGPath::StrokeOpacity()
{
	return fState.stroke_opacity;
}

inline float
BSVGPath::StrokeWidth()
{
	return fState.stroke_width;
}

inline cap_mode
BSVGPath::StrokeLineCap()
{
	return fState.stroke_linecap;
}

inline join_mode
BSVGPath::StrokeLineJoin()
{
	return fState.stroke_linejoin;
}

inline float
BSVGPath::StrokeMiterLimit()
{
	return fState.stroke_miterlimit;
}

inline rgb_color
BSVGPath::FillColor()
{
	return fState.fill_color;
}

inline rgb_color
BSVGPath::StrokeColor()
{
	return fState.stroke_color;
}

inline BShape *
BSVGPath::Shape()
{
	return &fShape;
}

inline BRect
BSVGPath::ShapeBounds()
{
	//bounds.InsetBy(-StrokeWidth() / 2, -StrokeWidth() / 2);
	return fRenderShape->Bounds();;
}

inline BShape *
BSVGPath::TransformedShape()
{
	if (fRenderShape != &fTransformedShape && !fState.general_transform.IsIdentity())
		ApplyTransformation();
	
	return fRenderShape;
}

inline BRect
BSVGPath::TransformedShapeBounds()
{
	return TransformedShape()->Bounds();
}

inline BPoint
BSVGPath::LastLocation()
{
	return fLastLocation;
}

inline BPoint
BSVGPath::LastControl()
{
	return fLastControl;
}

inline bool
BSVGPath::LastWasCurve()
{
	return fLastWasCurve;
}

inline bool
BSVGPath::LastWasQuadBezier()
{
	return fLastWasQuad;
}

inline Matrix2D
BSVGPath::Transformation()
{
	return fState.general_transform;
}

#endif
