#include "JabberElement.h"

#include <string.h>

JabberElement::JabberElement()
{
	fName = "";
	fData = "";
	fAttr = NULL;
	fAttrCount = -1;
}


JabberElement::~JabberElement()
{
	Free();
}

void 
JabberElement::SetAttr(const char ** attr)
{
	Free();
	if (attr)
	{
		const char ** a = attr;
		fAttrCount = 0;
		while (*a)
		{
			fAttrCount++;
			a++;
		}
		fAttr = new char *[fAttrCount + 1];
		for (int32 i = 0; i < fAttrCount; i++)
		{
			fAttr[i] = new char[strlen(attr[i]) + 1];
			strcpy(fAttr[i], attr[i]);
		}
	}
}

void
JabberElement::Free()
{
	if (fAttrCount != -1)
	{
		for (int32 i = 0; i < fAttrCount; i++)
			delete [] fAttr[i];
		delete fAttr;
		fAttr = NULL;
		fAttrCount = -1;
	}
}
