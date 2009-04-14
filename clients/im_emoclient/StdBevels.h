/* ===================================================================== *
 * StdBevels.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * History:
 *	1997		Talin
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 *	04/18/2000	cell
 *	Updated DrawBorderBevel() to use BViews' LineArray API
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __StdBevels_H__
#define __StdBevels_H__

#include <InterfaceDefs.h>

class BRect;
class BView;

/**
 *		Standardizes "bevel box" drawing routines.
 *		@author	Talin, Christoper Lenz.  
 */

namespace StdBevels
{
	/**	Constants.	*/
	enum bevel_state {

		NORMAL_BEVEL,

		DEPRESSED_BEVEL,

		DIMMED_BEVEL
	};

	static const rgb_color		NORMAL_GREY[4] =
	{
		{128, 128, 128, 255},
		{255, 255, 255, 255},
		{190, 190, 190, 255},
		{220, 220, 220, 255}
	};


	static const rgb_color		DEPRESSED_GREY[4] =
	{
		{128, 128, 128, 255},
		{140, 140, 140, 255},
		{200, 200, 200, 255},
		{180, 180, 180, 255}
	};

	static const rgb_color		DIMMED_GREY[4] =
	{
		{180, 180, 180, 255},
		{230, 230, 230, 255},
		{210, 210, 210, 255},
		{220, 220, 220, 255}
	};

	/**	Bevels with coloration similar to Be Scrollbars, for placing in
			window borders.	*/
	void DrawBorderBevel(
		BView *view,
		BRect rect,
		bevel_state state = NORMAL_BEVEL);

	/**	Bevels with same coloration as Be buttons.	*/
	void DrawButtonBevel(
		BView *view,
		BRect rect,
		bevel_state state = NORMAL_BEVEL);

	/**	Same as button bevel but with square corners.	*/
	void DrawSquareBevel(
		BView *view,
		BRect rect,
		bevel_state state = NORMAL_BEVEL);
};

#endif /* __StdBevels_H__ */
