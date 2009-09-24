#ifndef ICONTEXTITEM_H
#define ICONTEXTITEM_H

#include <Bitmap.h>
#include <ListItem.h>
#include <String.h>
#include <View.h>

class IconTextItem : public BListItem {
	public:
	
						IconTextItem(const char *name, const char *text, BBitmap *icon = NULL);
						~IconTextItem(void);
						
		virtual void	DrawItem(BView *owner, BRect frame, bool complete);
		virtual void	Update(BView *owner, const BFont *font);
		const char		*Name(void) const;
		const char 		*Text(void) const;
		const BBitmap	*Icon(void) const;
		

	private:
		BBitmap			*fIcon;
		BString			fName;
		BString			fText;
		float			fFontHeight;
		float			fIconHeight;
		float			fIconWidth;
		float			fFontOffset;
};

#endif
