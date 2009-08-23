#ifndef MULTILINESTRINGVIEW_H
#define MULTILINESTRINGVIEW_H

#include <String.h>
#include <View.h>
#include <vector>

typedef std::vector<BString> line_t;

class MultiLineStringView : public BView {
	public:
							MultiLineStringView(const char *name, const char *text,
								float width);
							~MultiLineStringView(void);
							
		// BView hooks
		void				AttachedToWindow(void);
		void				Draw(BRect rect);
		void				GetPreferredSize(float *width, float *height);
	
		// Public
		void				SetText(const char *text);
		const char			*Text(void);
		float				Width(void);
		void				SetWidth(float width);
		
	
	private:
		void				CalculateWrapping(const char *text);
		
		BString				fText;
		float				fWidth;
		line_t				fLines;			
};

#endif
