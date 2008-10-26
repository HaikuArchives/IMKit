/* ===================================================================== *
 * StdBevels.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StdBevels.h"

// Interface Kit
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_OPERATION(x) //PRINT (x)			// Operations

// ---------------------------------------------------------------------------
// Operations

void
StdBevels::DrawBorderBevel(
	BView *target,
	BRect rect,
	bevel_state state)
{
	D_OPERATION(("StdBevels::DrawBorderBevel()\n"));

	const rgb_color *colors;
	switch (state)
	{
		case DIMMED_BEVEL:
		{
			colors = DIMMED_GREY;
			break;
		}
		case DEPRESSED_BEVEL:
		{
			colors = DEPRESSED_GREY;
			break;
		}
		default: // NORMAL_BEVEL
		{
			colors = NORMAL_GREY;
			break;
		}
	}

	target->BeginLineArray(10);
		target->AddLine(rect.LeftTop(), rect.RightTop(), colors[0]);
		target->AddLine(rect.RightTop(), rect.RightBottom(), colors[0]);
		target->AddLine(rect.RightBottom(), rect.LeftBottom(), colors[0]);
		target->AddLine(rect.LeftBottom(), rect.LeftTop(), colors[0]);
		rect.InsetBy(1.0, 1.0);
		target->AddLine(rect.LeftBottom(), rect.LeftTop(), colors[1]);
		target->AddLine(rect.LeftTop(), rect.RightTop(), colors[1]);
		target->AddLine(rect.RightTop(), rect.RightBottom(), colors[2]);
		target->AddLine(rect.RightBottom(), rect.LeftBottom(), colors[2]);
		target->AddLine(rect.LeftBottom(), rect.LeftBottom(), colors[3]);
		target->AddLine(rect.RightTop(), rect.RightTop(), colors[3]);
	target->EndLineArray();

	rect.InsetBy(1.0, 1.0);
	target->SetHighColor(colors[3]);
	target->FillRect(rect);
}

void
StdBevels::DrawButtonBevel(
	BView *target,
	BRect rect,
	bevel_state state)
{
	D_OPERATION(("StdBevels::DrawBorderBevel()\n"));

	target->SetHighColor( 80, 80, 80 );
	target->FillRect( BRect( rect.left + 1, rect.top, rect.right - 1, rect.top ) );
	target->FillRect( BRect( rect.left + 1, rect.bottom, rect.right - 1, rect.bottom ) );
	target->FillRect( BRect( rect.left, rect.top + 1, rect.left, rect.bottom - 1 ) );
	target->FillRect( BRect( rect.right, rect.top + 1, rect.right, rect.bottom - 1 ) );
	rect.InsetBy( 1.0, 1.0 );

	switch (state) {
		case DEPRESSED_BEVEL:
		{
				// 1st highlight
			target->SetHighColor( 96, 96, 96 );
			
			target->FillRect( BRect( rect.left, rect.top, rect.right, rect.top ) );
			target->FillRect( BRect( rect.left, rect.top, rect.left, rect.bottom ) );
				// 2nd highlight
			target->SetHighColor( 120, 120, 120 );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 1 ) );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.left + 1, rect.bottom - 1 ) );
				// Deep shadow
			target->SetHighColor( 184, 184, 184 );
			target->FillRect( BRect( rect.left + 1, rect.bottom, rect.right, rect.bottom ) );
			target->FillRect( BRect( rect.right, rect.top + 1, rect.right, rect.bottom ) );
				// Mild Shadow
			target->SetHighColor( 176, 176, 176 );
			target->FillRect( BRect( rect.left + 2, rect.bottom - 1, rect.right - 1, rect.bottom - 1 ) );
			target->FillRect( BRect( rect.right - 1, rect.top + 2, rect.right - 1, rect.bottom - 1 ) );
				// Fill
			target->SetHighColor( 168, 168, 168 );
			target->FillRect( BRect( rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2 ) );
			break;
		}
		case DIMMED_BEVEL:
		{
			target->SetHighColor( 220, 220, 220 );
			target->FillRect( rect );
			break;
		}
		default:
		{
				// 1st highlight
			target->SetHighColor( 235, 235, 235 );
			target->FillRect( BRect( rect.left, rect.top, rect.right, rect.top ) );
			target->FillRect( BRect( rect.left, rect.top, rect.left, rect.bottom ) );
				// Main highlight
			target->SetHighColor( 255, 255, 255 );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 1 ) );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.left + 1, rect.bottom - 1 ) );
				// Deep shadow
			target->SetHighColor( 168, 168, 168 );
			target->FillRect( BRect( rect.left + 1, rect.bottom, rect.right, rect.bottom ) );
			target->FillRect( BRect( rect.right, rect.top + 1, rect.right, rect.bottom ) );
				// Mild shadow
			target->SetHighColor( 200, 200, 200 );
			target->FillRect( BRect( rect.left + 2, rect.bottom - 1, rect.right - 1, rect.bottom - 1 ) );
			target->FillRect( BRect( rect.right - 1, rect.top + 2, rect.right - 1, rect.bottom - 1 ) );

				// Fill
			target->SetHighColor( 235, 235, 235 );
			target->FillRect( BRect( rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2 ) );
			break;
		}
	}
}

void
StdBevels::DrawSquareBevel(
	BView *target,
	BRect rect,
	bevel_state state)
{
	D_OPERATION(("StdBevels::DrawBorderBevel()\n"));

	target->SetHighColor( 80, 80, 80 );
	target->StrokeRect( rect );
	rect.InsetBy( 1.0, 1.0 );

	switch (state) {
		case DEPRESSED_BEVEL:
		{
			// 1st highlight
			target->SetHighColor( 96, 96, 96 );
			target->FillRect( BRect( rect.left, rect.top, rect.right, rect.top ) );
			target->FillRect( BRect( rect.left, rect.top, rect.left, rect.bottom ) );
			// 2nd highlight
			target->SetHighColor( 120, 120, 120 );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 1 ) );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.left + 1, rect.bottom - 1 ) );
			// Deep shadow
			target->SetHighColor( 184, 184, 184 );
			target->FillRect( BRect( rect.left + 1, rect.bottom, rect.right, rect.bottom ) );
			target->FillRect( BRect( rect.right, rect.top + 1, rect.right, rect.bottom ) );
			// Mild Shadow
			target->SetHighColor( 176, 176, 176 );
			target->FillRect( BRect( rect.left + 2, rect.bottom - 1, rect.right - 1, rect.bottom - 1 ) );
			target->FillRect( BRect( rect.right - 1, rect.top + 2, rect.right - 1, rect.bottom - 1 ) );
			// Fill
			target->SetHighColor( 168, 168, 168 );
			target->FillRect( BRect( rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2 ) );
			break;
		}
		case DIMMED_BEVEL:
		{
			target->SetHighColor( 220, 220, 220 );
			target->FillRect( rect );
			break;
		}
		default: // NORMAL_BEVEL
		{
				// 1st highlight
			target->SetHighColor( 235, 235, 235 );
			target->FillRect( BRect( rect.left, rect.top, rect.right, rect.top ) );
			target->FillRect( BRect( rect.left, rect.top, rect.left, rect.bottom ) );
				// Main highlight
			target->SetHighColor( 255, 255, 255 );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 1 ) );
			target->FillRect( BRect( rect.left + 1, rect.top + 1, rect.left + 1, rect.bottom - 1 ) );
				// Deep shadow
			target->SetHighColor( 168, 168, 168 );
			target->FillRect( BRect( rect.left + 1, rect.bottom, rect.right, rect.bottom ) );
			target->FillRect( BRect( rect.right, rect.top + 1, rect.right, rect.bottom ) );
				// Mild shadow
			target->SetHighColor( 200, 200, 200 );
			target->FillRect( BRect( rect.left + 2, rect.bottom - 1, rect.right - 1, rect.bottom - 1 ) );
			target->FillRect( BRect( rect.right - 1, rect.top + 2, rect.right - 1, rect.bottom - 1 ) );

				// Fill
			target->SetHighColor( 235, 235, 235 );
			target->FillRect( BRect( rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2 ) );
			break;
		}
	}
}

// END - StdBevels.cpp
