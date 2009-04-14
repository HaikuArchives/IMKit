#ifndef _SVG_DATA_CREATOR_
#define _SVG_DATA_CREATOR_

#include <Shape.h>
#include <Point.h>
#include <String.h>

class BSVGDataCreator : public BShapeIterator {

public:
						BSVGDataCreator(BString &target);
virtual					~BSVGDataCreator();

virtual	status_t		IterateMoveTo(BPoint *point);
virtual	status_t		IterateLineTo(int32 lineCount, BPoint *linePts);
virtual	status_t		IterateBezierTo(int32 bezierCount, BPoint *bezierPts);
virtual	status_t		IterateClose();

private:
friend	class BSVGPolyDataCreator;

		BString			&fTarget;
		BPoint			*fLast;
};

class BSVGPolyDataCreator : public BSVGDataCreator {

public:
						BSVGPolyDataCreator(BString &target);
virtual					~BSVGPolyDataCreator();

virtual	status_t		IterateMoveTo(BPoint *point);		// used once
virtual	status_t		IterateLineTo(int32 lineCount, BPoint *linePts);
virtual status_t		IterateClose();						// not needed
};

#endif