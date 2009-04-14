#ifndef GRADIENT_H_
#define GRADIENT_H_

#include <Message.h>
#include <Rect.h>
#include <Region.h>
#include <String.h>
#include <SupportDefs.h>
#include "SVGDefs.h"
#include "ObjectList.h"

class BBitmap;
class BGradient;
class BShape;
class BStop;
class BView;

// BGradient serves as a base class for BLinearGradient and BRadialGradient

class BGradient : public BArchivable {

public:
						BGradient();
						BGradient(BRect frame, BPoint start, BPoint end);
virtual					~BGradient();

						BGradient(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

		void			SetName(const char *name);
		void			SetName(BString &name);
		BString			Name();

		gradient_t		Type();

		void			SetVector(BPoint start, BPoint end);
		void			SetStart(BPoint start);
		void			SetEnd(BPoint end);
		BPoint			Start() const;
		BPoint			End() const;

		void			SetTransformation(Matrix2D transform);
		Matrix2D		Transformation() const;

		void			SetCoordinateSystem(Matrix2D system);
		Matrix2D		CoordinateSystem() const;

		void			SetGradientUnits(units_t units);
		units_t			GradientUnits() const;

		void			SetBounds(BRect bounds);
		void			SetOrigin(BPoint origin);

		void			SetScale(float scale);

		BStop			*StopAt(int index);
		void			AddStop(BStop *stop);
		bool			RemoveStop(BStop *stop);
		BStop			*RemoveStopAt(int index);
		int32			CountStops();
		void			ClearStops();
		void			SortStops();

virtual	void			Render(BView *into_view);

private:
friend	class BLinearGradient;
friend	class BRadialGradient;

virtual	void			Recalculate();

		BString			fName;
		gradient_t		fType;
		
		BPoint			fStart;			// these two points give us the vector
		BPoint			fEnd;			// the gradient is drawn along
		BRect			fBounds;
		float			fScale;
		Matrix2D		fTransformation;
		Matrix2D		fCoordinateSystem;
		Matrix2D		fRenderMatrix;	// temporary matrix (transform * system)
		units_t			fUnits;

		BObjectList<BStop>		*fStops;

		int				fLength;
		BPoint			fDelta;
		float			fM;
		float			fStep;
		float			fMin;
		float			fMax;

		BPoint			fOrigin;
		BPoint			fOriginX;
		BPoint			fOriginY;

		BPoint			fEdge;			// the intersecting edge of origin X Y
		bool			fFirstColor;
};


class BLinearGradient : public BGradient
{

public:
						BLinearGradient();
						BLinearGradient(BRect frame, BPoint start, BPoint end);
virtual					~BLinearGradient();

						BLinearGradient(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

virtual	void			Render(BView *into_view);

private:
virtual	void			Recalculate();

typedef	BGradient		_inherited;

};


class BRadialGradient : public BGradient
{

public:
						BRadialGradient();
						BRadialGradient(BRect frame, BPoint start, BPoint end);
virtual					~BRadialGradient();

						BRadialGradient(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

		void			SetFocal(BPoint focal);
		BPoint			Focal() const;

virtual	void			Render(BView *into_view);

private:
virtual	void			Recalculate();
		BPoint			fFocal;
		BRegion			fClip;

typedef	BGradient		_inherited;

};

#endif	// GRADIENT_H_
