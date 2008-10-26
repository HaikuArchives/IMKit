#include <Font.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <stdio.h>
#include <string.h>
#include <String.h>
#include <math.h>

//#include "TestIcons.h"
#include "MenuBuilder.h"
#include "BitmapMenuItem.h"

#include "PopUpMenu.h"

//#define NUMX 8
//#define NUMY 10

//#define ICON	22
extern float gEmoticonSize;

float ICON = gEmoticonSize;
#define PAD	4

//#define TOTICON (ICON+PAD)

BMenu* 
MenuBuilder::CreateMenu(BMessage* faces,int32 messid)
{
	
	BMenu* xMenu;
	BBitmap *xBitmap;
	BitmapMenuItem *xItem;
	
	int numFaces=0;
	for (;faces->FindString("face",numFaces); numFaces++)
		;
	
	double s = sqrt(numFaces);
	int NUMX = (int)floor(s+1);
	int NUMY = NUMX;
	
	float TOTICON = gEmoticonSize + PAD;
	
	float menuWidth = NUMX*TOTICON;
	float menuHeight = NUMY*TOTICON;
	
	xMenu = new BMenu("emoticons", menuWidth, menuHeight);
	
	for (int32 i=0; i<numFaces; i++) {
		xBitmap = LoadBitmap(i, faces);
		
		if(xBitmap){
			int x = i % NUMX;
			int y = i / NUMY;
			xItem = new BitmapMenuItem("", xBitmap,new BMessage(messid), 0, 0);
			xMenu->AddItem(xItem, BRect(x*TOTICON,y*TOTICON,x*TOTICON+TOTICON-1,y*TOTICON+TOTICON-1));
		 }
	}
	
	return xMenu;
	
}

BPopUpMenu* 
MenuBuilder::CreateMenuP(BMessage* faces,int32 messid)
{
	
	BPopUpMenu* 	xMenu;
	BBitmap*		xBitmap;
	BitmapMenuItem*	xItem;
	
	int numFaces=0;
	for (;faces->FindString("face",numFaces); numFaces++)
		;
	
	double s = sqrt(numFaces);
	int NUMX = (int)floor(s+1);
	int NUMY = NUMX;
	
	float TOTICON = gEmoticonSize + PAD;
	
	float menuWidth = NUMX*TOTICON;
	float menuHeight = NUMY*TOTICON;
	
	
	xMenu = new BPopUpMenu("emoticons", menuWidth, menuHeight,false,false);
	
	for (int32 i=0; i<numFaces; i++) {
//		for (int32 j=0; j<NUMX; j++) {
		xBitmap = LoadBitmap(i, faces);
		
		if(xBitmap){
			int x = i % NUMX;
			int y = i / NUMY;
			xItem = new BitmapMenuItem("", xBitmap,new BMessage(messid), 0, 0);
			xMenu->AddItem(xItem, BRect(x*TOTICON,y*TOTICON,x*TOTICON+TOTICON-1,y*TOTICON+TOTICON-1));
		}
	}
	return xMenu;
	
}

BBitmap* 
MenuBuilder::LoadBitmap(int32 index, BMessage*faces)
{
	BBitmap* pBitmap;
	BString f;
//	int index=NUMX*i + j;
	faces->FindString("face",index,&f);
	faces->FindPointer(f.String(),(void**)&pBitmap);
		
	return pBitmap;		
}
