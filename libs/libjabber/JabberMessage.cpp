#include "JabberMessage.h"
#include "stdio.h"

JabberMessage::JabberMessage() 
{
	fTo = "";
	fFrom = "";
	fBody = "";
	fStamp = "";
	fId = "";
	fOffline= false;
	fType="";
	fError="";
	fX="";
}

JabberMessage::JabberMessage(const JabberMessage & copy)
{
	fTo = copy.fTo;
	fFrom = copy.fFrom;
	fBody = copy.fBody;
	fStamp = copy.fStamp;
	fOffline= copy.fOffline;
	fId = copy.fId;
	fType=copy.fType;
	fError=copy.fError;
	fX=copy.fX;
}

void
JabberMessage::PrintToStream() 
{
	//by xed uses printf (no good :)
	printf(" ** JabberMessage **\n");
	printf("    To:  %s\n",fTo.String());
	printf("    Id:  %s\n",fId.String());
	printf("  From:  %s\n",fFrom.String());
	printf("  Body:  %s\n",fBody.String());
	printf(" Stamp:  %s\n\n",fStamp.String());
	printf("  Type:  %s\n",fType.String());
	printf(" Error:  %s\n\n",fError.String());
	printf("     X:  %s\n\n",fX.String());
}


JabberMessage::~JabberMessage() 
{
}

void
JabberMessage::operator=(const JabberMessage & copy)
{
	fTo = copy.fTo;
	fFrom = copy.fFrom;
	fBody = copy.fBody;
	fStamp = copy.fStamp;
	fOffline= copy.fOffline;
	fId = copy.fId;
	fType=copy.fType;
	fError=copy.fError;
	fX=copy.fX;
}

BString 
JabberMessage::GetFrom() const
{
	return fFrom;	
}

BString
JabberMessage::GetTo() const
{
	return fTo;	
}

BString
JabberMessage::GetBody() const
{
	return fBody;	
}

BString
JabberMessage::GetStamp() const
{
	return fStamp;
}

BString
JabberMessage::GetID() const
{
	return fId;
}
void
JabberMessage::SetFrom(const BString & from)
{
	fFrom = from;
}

void
JabberMessage::SetTo(const BString & to)
{
	fTo = to;
}

void
JabberMessage::SetBody(const BString & body)
{
	fBody = body;
}

void
JabberMessage::SetStamp(const BString & stamp)
{
	fStamp = stamp;
}
void
JabberMessage::SetID(const BString & id)
{
	fId = id;
}
void
JabberMessage::SetOffline(const bool b)
{
 	fOffline = b;
}
