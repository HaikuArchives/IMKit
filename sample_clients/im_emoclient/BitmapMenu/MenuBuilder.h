#ifndef _MenuBuilder_h
#define _MenuBuilder_h

class MenuBuilder
{		
public:
	BMenu*			CreateMenu(BMessage*,int32 messid);
	BPopUpMenu*			CreateMenuP(BMessage*,int32 messid);	
private:
	BBitmap*		LoadBitmap(int32 i, BMessage*);

};

#endif 

