#ifndef _SVG_TRANSFORMER_
#define _SVG_TRANSFORMER_

#include <Shape.h>
#include <Point.h>
#include "Matrix.h"

class BSVGTransformer : public BShapeIterator {

public:
						BSVGTransformer(Matrix2D *transformation, BShape *target);
virtual					~BSVGTransformer();

virtual	status_t		IterateMoveTo(BPoint *point);
virtual	status_t		IterateLineTo(int32 lineCount, BPoint *linePts);
virtual	status_t		IterateBezierTo(int32 bezierCount, BPoint *bezierPts);
virtual	status_t		IterateClose();

private:
		Matrix2D		*fTransformation;
		BShape			*fTarget;
};

#endif