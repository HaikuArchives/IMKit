#include "main.h"

#define SIGN	"application/x-vnd.xeD.im_emoclient"

// defined in SmileTextRender.h
float gEmoticonSize = 16.0;


void
setAttributeIfNotPresent( entry_ref ref, const char * attr, const char * value )
{
//	printf("Setting attribute %s to %s\n", attr, value );
	
	BNode node(&ref);
	char data[512];
	
	if ( node.InitCheck() != B_OK )
	{
		LOG("im_emoclient", liHigh, "Invalid entry_ref in setAttributeIfNotSet");
		return;
	}
	
	if ( node.ReadAttr(attr,B_STRING_TYPE,0,data,sizeof(data)) > 1 )
	{
//		LOG("  value already present");
		return;
	}
	
	int32 num_written = node.WriteAttr(
		attr, B_STRING_TYPE, 0,
		value, strlen(value)+1
	);
	
	if ( num_written != (int32)strlen(value) + 1 )
	{
		LOG("im_emoclient", liHigh, "Error writing attribute %s (%s)\n",attr,value);
	} else
	{
		//LOG("Attribute set");
	}
}

extern "C" void process_refs(entry_ref dir_ref, BMessage *msg, void *) {
	msg->what = B_REFS_RECEIVED;
	msg->AddRef("dir_ref", &dir_ref);
	
	be_roster->Launch(SIGN, msg);
};

int main(void)
{
	ChatApp app;
	
	app.Run();
	
	return 0;
}
