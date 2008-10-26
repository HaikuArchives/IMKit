#ifndef IMIDLEFILTER_H
#define IMIDLEFILTER_H

#include <InputServerFilter.h>
#include <Looper.h>
#include <String.h>

#include <libim/Manager.h>

class IM::Manager;
class BMessageRunner;

class IMIdleFilter : public BLooper, public BInputServerFilter {
	public:
							IMIdleFilter(void);
							~IMIdleFilter(void);

		// BInputServerFilter hooks
		filter_result		Filter(BMessage *message, BList *outlist);
		status_t			InitCheck(void);
		
		// BHandler hooks
		void				MessageReceived(BMessage *msg);
		
	private:
		status_t			SaveSettingsTemplate(void);
		status_t			LoadSettings(void);
		
		IM::Manager			*fMan;
		BMessageRunner		*fRunner;
		int32				fDelay;
		BString				fAwayMsg;
		int32				fReturnType;
		bool				fIdle;
		int8				fStatus;
};

#endif
