#include "MultiLineStringView.h"

#include <ctype.h>

//#pragma mark Public

MultiLineStringView::MultiLineStringView(const char *name, const char *text,
	float width)
	: BView(BRect(0, 0, width, width), name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fText(text),
	fWidth(width) {
	
	CalculateWrapping(text);
};

MultiLineStringView::~MultiLineStringView(void) {
};

//#pragma mark BView Hooks

void MultiLineStringView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetColorsFromParent();
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif	
};
							
void MultiLineStringView::Draw(BRect rect) {
	int32 lines = fLines.size();
	BFont font;
	font_height fh;
	BPoint point;
	
	GetFont(&font);
	font.GetHeight(&fh);
	point.x = 0;
	point.y = fh.ascent + fh.descent + fh.leading;
		
	for (int32 i = 0; i < lines; i++) {
		DrawString(fLines[i].String(), point);
		point.y += fh.ascent + fh.leading + fh.descent;
	};
};

void MultiLineStringView::GetPreferredSize(float *width, float *height) {
	BFont font;
	font_height fh;

	GetFont(&font);
	font.GetHeight(&fh);

	*width = fWidth;
	*height = (fLines.size() + 1) * (fh.ascent + fh.leading + fh.descent);
};

//#pragma mark Public

void MultiLineStringView::SetText(const char *text) {
	fText = text;
	CalculateWrapping(text);
};

const char *MultiLineStringView::Text(void) {
	return fText.String();
};

float MultiLineStringView::Width(void) {
	return fWidth;
};

void MultiLineStringView::SetWidth(float width) {
	fWidth = width;
	CalculateWrapping(fText.String());
};

//#pragma mark Private

void MultiLineStringView::CalculateWrapping(const char *text) {
	const char spacers[] = " \n-\\/";
	vector<int16> breakPos;
	size_t wordLen = 0;
	int16 length = 0;
	int16 offset = 0;
	int16 spaceCount = 0;
	int16 lastBreak = 0;
	BFont font;

	GetFont(&font);
	
	BString temp = text;
	text = temp.ReplaceAll("\t", "    ").String();
	length = strlen(text);
	
	// Get a list of positions where we can break
	while ((wordLen = strcspn(text + offset, spacers)) < (size_t)(length - offset)) {
		breakPos.push_back(wordLen + offset);
		offset += wordLen + 1;
	};
		
	spaceCount = breakPos.size();
	fLines.clear();
	
	for (int32 i = 0; i < spaceCount; i++) {
		BString line = "";
		bool wasWhiteSpace = isspace(text[lastBreak]);

		// If this space char is a newline, force a new line
		if (text[breakPos[i]] == '\n') {
			line.SetTo(text + lastBreak, breakPos[i] - lastBreak);
			lastBreak = breakPos[i];
		} else {
			line.SetTo(text + lastBreak, breakPos[i] - lastBreak);
			if (font.StringWidth(line.String()) > fWidth) {
				line.SetTo(text + lastBreak, breakPos[i - 1] - lastBreak);
				lastBreak = breakPos[i - 1];
			} else {
				continue;
			};
		};
			
		line.ReplaceAll("\n", "");
		if (wasWhiteSpace) line.Remove(0, 1);
		
		fLines.push_back(line);
	};

	// Trailing content...
	if (lastBreak < length) {
		BString line = "";
		line.SetTo(text + lastBreak, length - lastBreak);
		line.ReplaceAll("\n", "");

		if (isspace(text[lastBreak])) line.Remove(0, 1);
		
		fLines.push_back(line);
	};
	
	// This will occur if the text is small enough to fit on one line - append all of the text
	if (fLines.empty() == true) {
		fLines.push_back(text);
	};
};

