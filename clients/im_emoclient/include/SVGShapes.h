#ifndef SVGSHAPES_H_
#define SVGSHAPES_H_

#include "SVGPath.h"
#include "SVGView.h"

class BSVGRect : public BSVGPath {

public:
						BSVGRect(BSVGView *parent);

						BSVGRect(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_RECT; };

virtual	void			FinalizeShape();
virtual	void			RecreateData();

		void			SetX(float x);
		void			SetY(float y);

		void			SetWidth(float width);
		void			SetHeight(float height);

		void			SetRX(float rx);
		void			SetRY(float ry);

private:
		BRect			fRect;
		float			fRX;
		float			fRY;
		bool			fRXSet;
		bool			fRYSet;
};

inline void
BSVGRect::SetX(float x)
{
	fRect.left = x;
}

inline void
BSVGRect::SetY(float y)
{
	fRect.top = y;
}

inline void
BSVGRect::SetWidth(float width)
{
	fRect.right = fRect.left + width;
}

inline void
BSVGRect::SetHeight(float height)
{
	fRect.bottom = fRect.top + height;
}

inline void
BSVGRect::SetRX(float rx)
{
	fRX = rx;
	fRXSet = true;
}

inline void
BSVGRect::SetRY(float ry)
{
	fRY = ry;
	fRYSet = true;
}

// **************************************************************************

class BSVGEllipse : public BSVGPath {

public:
						BSVGEllipse(BSVGView *parent);

						BSVGEllipse(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_ELLIPSE; };

virtual	void			FinalizeShape();
virtual	void			RecreateData();

		void			SetCX(float cx);
		void			SetCY(float cy);

		void			SetRX(float rx);
		void			SetRY(float ry);

private:

friend 	class BSVGCircle;

		BPoint			fCenter;
		float			fRX;
		float			fRY;
		bool			fRXSet;
		bool			fRYSet;
};

inline void
BSVGEllipse::SetCX(float cx)
{
	fCenter.x = cx;
}

inline void
BSVGEllipse::SetCY(float cy)
{
	fCenter.y = cy;
}

inline void
BSVGEllipse::SetRX(float rx)
{
	fRX = rx;
	fRXSet = true;
}

inline void
BSVGEllipse::SetRY(float ry)
{
	fRY = ry;
	fRYSet = true;
}

// **************************************************************************

class BSVGCircle : public BSVGEllipse {

public:
						BSVGCircle(BSVGView *parent);

						BSVGCircle(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_CIRCLE; };

virtual	void			RecreateData();

		void			SetR(float r);
};

inline void
BSVGCircle::SetR(float r)
{
	fRX = r;
	fRY = r;
	fRXSet = true;
	fRYSet = true;
}

// **************************************************************************

class BSVGLine : public BSVGPath {

public:
						BSVGLine(BSVGView *parent);

						BSVGLine(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_LINE; };

virtual	void			FinalizeShape();
virtual void			RecreateData();

		void			SetX1(float x1);
		void			SetY1(float y1);

		void			SetX2(float x2);
		void			SetY2(float y2);

private:
		BPoint			fPoint1;
		BPoint			fPoint2;
};

inline void
BSVGLine::SetX1(float x1)
{
	fPoint1.x = x1;
}

inline void
BSVGLine::SetY1(float y1)
{
	fPoint1.y = y1;
}

inline void
BSVGLine::SetX2(float x2)
{
	fPoint2.x = x2;
}

inline void
BSVGLine::SetY2(float y2)
{
	fPoint2.y = y2;
}

// **************************************************************************

class BSVGPolyline : public BSVGPath {

public:
						BSVGPolyline(BSVGView *parent);

						BSVGPolyline(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_POLYLINE; };

virtual	void			FinalizeShape();
virtual	void			RecreateData();

private:

friend	class BSVGPolygon;
		void			CloseShape();
};

inline void
BSVGPolyline::CloseShape()
{
	fShape.Close();
}

// **************************************************************************

class BSVGPolygon : public BSVGPolyline {

public:
						BSVGPolygon(BSVGView *parent);

						BSVGPolygon(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual shape_t			Type() { return B_SVG_POLYGON; };

virtual	void			FinalizeShape();

private:
typedef	BSVGPolyline	_inherited;
};

#endif
