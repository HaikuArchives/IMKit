#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "main.h"

#include <List.h>

class BView;

class StatusBar : public BView {
	public:
						StatusBar(BRect rect);
		virtual			~StatusBar();
		
		virtual void 	MessageReceived(BMessage *msg);

				int32	AddItem(BView *view);
				BView	*ViewAt(int32 index);
			
		virtual void	Draw(BRect rect);
		
				void	PositionViews();
	private:
				void	DrawSplitter(float x);
				BList	fViews;
};

#endif
