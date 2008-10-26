#ifndef JABBER_ELEMENT_H
#define JABBER_ELEMENT_H

#include <String.h>

class JabberElement 
{
public:
						JabberElement();
						~JabberElement();
						
	void				SetName(const BString & name) { fName = name; }
	BString				GetName() const { return fName; }
	
	void				SetData(const BString & data) { fData = data; }
	BString				GetData() const { return fData; }
	
	void				SetAttr(const char ** attr);
	const char **		GetAttr() const { return (const char **)fAttr; }
	
	int32				GetAttrCount() const { return fAttrCount; }

private:
	BString				fName;
	BString				fData;
	char **				fAttr;
	int32				fAttrCount;
	
	void				Free();
};

#endif	// JABBERELEMENT_H
