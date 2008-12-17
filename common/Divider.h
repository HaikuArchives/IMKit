#ifndef COMMON_DIVIDER_H
#define COMMON_DIVIDER_H

#include <View.h>

class BMessage;
class BMessenger;

class Divider : public BView {
	public:
					    	Divider(BRect frame, const char *name = "Divider", uint32 resize = B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, uint32 flags = 0);
					    	Divider(BMessage *archive);
	    virtual				~Divider(void);
	    
	    // BView Hooks
    	virtual void		Draw(BRect updateRect);
    	
		// BArchivable Hooks
		status_t			Archive(BMessage *archive, bool deep = true) const;
		static BArchivable	*Instantiate(BMessage *archive);
    	
    	// Public
    	orientation			Orientation(void);
    	void				Orientation(orientation orient);
 
	private:
		orientation			fOrient;
};

#endif COMMON_DIVIDER_H
