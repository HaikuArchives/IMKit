#ifndef _Emoticor_h_
#define _Emoticor_h_


#include <String.h>
#include "RunView.h"
#include "Emoconfig.h"
// This is a proud xeD idea :)

class Emoticor
{
	public:
				Emoticor();
		void		AddText(RunView *fTextView,const char* text,  int16 cols ,int16 font ,int16 cols2 ,int16 font2 );
		void		LoadConfig(const char*);
				
				
				Emoconfig *config;
				
	private:
			
		void		_findTokens(RunView *fTextView,BString text,int tokenstart, int16 cols ,int16 font ,int16 cols2 ,int16 font2);		
		
};

extern Emoticor*	emoticor;

#endif

