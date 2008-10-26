#ifndef _SVG_COLORS_H_
#define _SVG_COLORS_H_

#include <GraphicsDefs.h>
#include <String.h>
#include "SVGDefs.h"

fill_t		HandleAttributeValue(const BString *value, target_t target, in_state_s current_state, rgb_color &color, BString &gradient);
bool		GetColorFromString(const BString *value, rgb_color &result);
bool		ExtractGradientName(const BString *value, BString &result);

#endif