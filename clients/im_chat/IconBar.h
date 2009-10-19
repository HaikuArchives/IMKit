#ifndef ICONBAR_H
#define ICONBAR_H

#include <Entry.h>
#include <View.h>
#include <String.h>
#include <vector>

class BDirectory;
class BubbleHelper;
class ImageButton;

typedef std::vector<ImageButton *> button_t;

class IconBar : public BView {
	public:
							IconBar(BRect rect, const char *path,
								BubbleHelper *help, int16 padding, entry_ref ref);
							~IconBar(void);


		// Hooks		
		void				AttachedToWindow(void);
		void 				MessageReceived(BMessage *msg);
		virtual void		Draw(BRect rect);

	private:
		button_t			fButtons;
		BubbleHelper		*fBubbles;
		entry_ref			fContactRef;
		BString				fAppPath;
		int16				fPadding;
		int16				fIconSize;
};

#endif
