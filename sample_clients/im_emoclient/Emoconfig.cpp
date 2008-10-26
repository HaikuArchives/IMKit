#include "Emoconfig.h"

#include <File.h>
#include <stdio.h>
#include <stdlib.h>
#include <Bitmap.h>
#include <String.h>
#include <Path.h>
#include <TranslationUtils.h>

#include "SVGView.h"
#include "SmileTextRender.h"

//tmp
BMessage* faces=NULL;
bool 		valid=false;
bool 		fname=false;
bool		svg=false;
bool		size=true;
BString filename;
BString face;
BPath 	path;
BString	gCharacters;

extern float gEmoticonSize;

Emoconfig::Emoconfig(const char* xmlfile):BMessage()
{
	fParser = XML_ParserCreate(NULL);
	numfaces=0;
	XML_SetUserData(fParser, this);
	XML_SetElementHandler(fParser, StartElement, EndElement);
	XML_SetCharacterDataHandler(fParser, Characters);
	
	//path!
	BPath p(xmlfile);
	p.GetParent(&path);
	//printf("** Path() %s \n",p.Path());
	
	// loading the config file..
	BFile* settings=new BFile(xmlfile,B_READ_ONLY);
	off_t size;
	settings->GetSize(&size);
//	printf("Original file %lld\n",size);
	if(size)
	{
		void *buffer=malloc(size);
		size=settings->Read(buffer,size);
//		printf("In memory file %lld\n",size);
		XML_Parse(fParser, (const char*)buffer, size, true);
		free(buffer);
	}
	delete settings;
	if(fParser)	XML_ParserFree(fParser);
	//PrintToStream();
}

Emoconfig::~Emoconfig()
{
	
}

void 
Emoconfig::StartElement(void * /*pUserData*/, const char * pName, const char ** /*pAttr*/)
{
	//printf("StartElement %s\n",pName);
	BString name(pName);
	if(name.ICompare("emoticon")==0)
	{
		faces=new BMessage();
		svg=false;
	}
	else
	if(name.ICompare("text")==0 && faces)
	{
		valid=true;
	}
	else
	if(name.ICompare("file")==0 && faces)
	{
		fname=true;
	} else
	if(name.ICompare("svg")==0 && faces)
	{
//		printf("File is SVG\n");
		svg=true;
	} else
	if(name.ICompare("size")==0)
	{
		size=true;
		gCharacters = "";
	}
}

void 
Emoconfig::EndElement(void * pUserData, const char * pName)
{
	//printf("EndElement %s\n",pName);
	BString name(pName);
	
	if(name.ICompare("emoticon")==0 && faces)
	{
		//faces->PrintToStream(); //debug
		delete faces;
		faces=NULL;
	
	}
	else
	if(name.ICompare("text")==0 && faces)
	{
		valid=false;
		faces->AddString("face",face);
		//printf("to ]%s[\n",face.String());
		face.SetTo("");
		
	}
	else
	if(name.ICompare("file")==0 && faces)
	{
		//load file
		
		//compose the filename
		BPath p(path);
		p.Append(filename.String());
		BBitmap *icons = NULL;
		
		if ( !svg )
		{ // 
			icons=BTranslationUtils::GetBitmap(p.Path());
		} else 
		{ // svg icon
//			printf("SVG icon\n");
			
			icons = NULL;
			
			static BSVGView * svgView = NULL;
			static BBitmap * renderer = NULL;
			
			if ( !renderer )
			{
				renderer = new BBitmap( BRect(0,0,(int32)gEmoticonSize-1,(int32)gEmoticonSize-1), B_RGB32, true );
				
				uint32 transparent = B_TRANSPARENT_MAGIC_RGBA32;
				
				svgView = new BSVGView( renderer->Bounds(), "SVG renderer", B_FOLLOW_ALL );
				svgView->SetViewColor( *((rgb_color*)&transparent) );
				svgView->SetScaleToFit(true);
				svgView->SetFitContent(true);
				svgView->SetSuperSampling(true);
				svgView->SetSampleSize(4);
				renderer->AddChild( svgView );
				
/*				renderer->Lock();
				printf("Created rendering stuff\n");
				printf("renderer: "); renderer->Bounds().PrintToStream();
				printf("svgView: "); svgView->Bounds().PrintToStream();
				renderer->Unlock();
*/			}
			
			renderer->Lock();
			
			svgView->Clear();
			if ( svgView->LoadFromFile( p.Path() ) != B_OK )
			{
				printf("Error loading SVG\n");
			} else 
			{
//				svgView->Invalidate();
				svgView->Draw( svgView->Bounds().InsetByCopy(-2,-2 ) );
				svgView->Flush();
				
				icons = new BBitmap( renderer->Bounds(), B_RGB32 );
				memcpy( icons->Bits(), renderer->Bits(), renderer->BitsLength() );
			}
			
			renderer->Unlock();
		}
		
		//assign to faces;
		fname=false;
		
//		printf("Filename %s [%s]\n",p.Path(),path.Path());
		if(!icons) return; 
		
		int 		i=0;
		BString s;
		while(faces->FindString("face",i,&s)==B_OK)
		{
		
			if(i==0)
			{
				((Emoconfig*)pUserData)->menu.AddPointer(s.String(),(const void*)icons);
				((Emoconfig*)pUserData)->menu.AddString("face",s.String());
			}
			((BMessage*)pUserData)->AddPointer(s.String(),(const void*)icons);
			((BMessage*)pUserData)->AddString("face",s.String());
			((Emoconfig*)pUserData)->numfaces++;
			i++;
			
		}
		
		
	} else
	if(name.ICompare("size")==0)
	{
		if ( size )
		{
			gEmoticonSize = atoi(gCharacters.String());
		}
		
		size = false;
	}
	 
}

void 
Emoconfig::Characters(void * /*pUserData*/, const char * pString, int pLen)
{
	BString f(pString,pLen);
	//printf("Characters %s\n",f.String());
	if(faces && valid)
	{
		f.RemoveAll(" ");
		f.RemoveAll("\"");
		if(f.Length()>0)
		face.Append(f);
	}
	else
	if(fname)
	{
		f.RemoveAll(" ");
		filename=f;
		
	}
	else
	{
		gCharacters.Append(f);
	}
}



