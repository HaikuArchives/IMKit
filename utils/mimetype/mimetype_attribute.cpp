#include <Application.h>
#include <Mime.h>
#include <Message.h>
#include <String.h>
#include <support/SupportDefs.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct value_type_t {
	BString 	name;
	type_code	type;
};

const value_type_t valid_types[] = {
	{"string", B_STRING_TYPE},
	{"boolean", B_BOOL_TYPE},
	{"int32", B_INT32_TYPE},
	{"float", B_FLOAT_TYPE},
	
	// end of list thingie
	{"", B_ANY_TYPE}
};

void print_usage()
{
	printf("usage: mimetype_attribute <options>\n\n");
	printf("Required options:\n");
	printf("   --mime <MIME type to edit, e.g. application/x-person>\n");
	printf("   --internal-name <internal name, e.g. IM:connections>\n");
	printf("   --public-name <public name, e.g. 'IM Connections'>\n");
	printf("   --type <attr type, one of [string, bool, int32, float]>\n");
	printf("Not required options:\n");
	printf("   --width <width in pixels>, sets the width of the attribute column in Tracker, default 60\n");
	printf("   --editable, --not-editable, sets attribute as user editable or not, editable by default.\n");
	printf("   --public, --private, sets attribute as public or private, public by default.\n");
	printf("   --viewable, --non-viewable, sets attribute as viewable or non-viewable, viewable by default.\n");
	printf("   --extra, --non-extra, sets attribute as extra or non-extra, non-extra by default.\n");
}

int main( int numarg, const char * argv[] )
{
	BApplication bapp("application/x-vnd.BeClan.mimetype_attribute");;
	BString mimeType;
	BString attributeInternal;
	BString attributePublic;
	BString attributeType;
	bool isEditable = true;
	bool isPublic = true;
	bool isViewable = true;
	bool isExtra = false;

	int displayWidth=60;
	
	// decode arguments
	for (int i = 1; i<numarg; i++) {
		if (strcasecmp(argv[i], "--mime") == 0) {
			// mime type
			if (i != numarg - 1) {
				mimeType = argv[++i];
			}
		}

		if (strcasecmp(argv[i], "--internal-name") == 0) {
			// internal name
			if ( i != numarg - 1 )
			{
				attributeInternal = argv[++i];
			}
		}
		
		if ( strcasecmp(argv[i], "--public-name") == 0 ) {
			// public name
			if ( i != numarg - 1 )
			{
				attributePublic = argv[++i];
			}
		}
		
		if (strcasecmp(argv[i], "--type") == 0) {
			// type
			if (i != numarg - 1) {
				attributeType = argv[++i];
			}
		}
		
		if (strcasecmp(argv[i], "--width") == 0) {
			// width
			if (i != numarg - 1) {
				displayWidth = atoi(argv[++i]);
			}
		}
		
		if (strcasecmp(argv[i], "--not-editable") == 0) isEditable = false;
		if (strcasecmp(argv[i], "--editable") == 0) isEditable = true;
		
		if (strcasecmp(argv[i], "--public") == 0) isPublic = true;
		if (strcasecmp(argv[i], "--private") == 0) isPublic = false;
		
		if (strcasecmp(argv[i], "--viewable") == 0) isViewable = true;
		if (strcasecmp(argv[i], "--non-viewable") == 0) isViewable = false;
		
		if (strcasecmp(argv[i], "--extra") == 0) isExtra = true;
		if (strcasecmp(argv[i], "--non-extra") == 0) isExtra = false;
	}
	
	// check arguments
	if ( mimeType == "" || attributeInternal == "" || attributePublic == ""	) {
		print_usage();
		return 1;
	}
	
	type_code attributeTypeConst = B_ANY_TYPE;
	for (int i=0; valid_types[i].name != ""; i++ ) {
		if (attributeType == valid_types[i].name) {
			attributeTypeConst = valid_types[i].type;
		};
	}
	
	if ( attributeTypeConst == B_ANY_TYPE) {
		if (attributeType.Length() == 4) {
			attributeTypeConst = (attributeType[0] << 24) + (attributeType[1] << 16) +
				(attributeType[2] << 8) + attributeType[3];
				
			printf("Attribute type was non standard, presuming character "
				"representation: %s -> %i\n", attributeType.String(),
				attributeTypeConst);
		} else {
			print_usage();
			return 1;
		};
	}

	// args ok, fetch current attributes
	BMimeType mime(mimeType.String());
	BMessage msg;
	
	if (mime.InitCheck() != B_OK) {
		printf("Invalid MIME type\n");
		return 2;
	}

	if (mime.IsInstalled() == false) {
		printf("MIME type is not installed, installing...");
		status_t status = mime.Install();
		if (status != B_OK) printf("%s", strerror(status));
		printf("\n");
		
		mime.SetAttrInfo(&msg);
	};

	
	if (mime.GetAttrInfo(&msg) != B_OK) {
		printf("Error getting current attributes: %s\n:", strerror(mime.GetAttrInfo(&msg)));
		msg.PrintToStream();
		return 3;
	}
	
	// check if already set
	const char * name = NULL;

	for (int32 index ; msg.FindString("attr:name", index, &name) == B_OK; index++) {
		if (attributeInternal == name) {
			printf("Attribute already set.\n");
			return 4;
		}
	}
	
	printf("Adding attribute [%s] to MIME type [%s]\n", attributeInternal.String(), mimeType.String() );
	printf("Pretty name: [%s]\n", attributePublic.String() );
	printf("Type: [%s] (%X)\n", attributeType.String(), attributeTypeConst );
	printf("Display width: %ld\n", displayWidth );
	printf("Editable: %s\n", isEditable ? "true" : "false" );
	printf("Public: %s\n", isPublic ? "true" : "false" );
	printf("Viewable: %s\n", isViewable ? "true" : "false" );
	printf("Extra: %s\n", isExtra ? "true" : "false" );
	
	// Ok, let's add it.
	msg.AddString("attr:name", attributeInternal.String() );
	msg.AddString("attr:public_name", attributePublic.String() );
	msg.AddInt32("attr:type", attributeTypeConst );
	msg.AddInt32("attr:width", displayWidth );
	msg.AddBool("attr:editable", isEditable );
	msg.AddBool("attr:public", isPublic );
	msg.AddBool("attr:viewable", isViewable );
	msg.AddBool("attr:extra", isExtra );
	msg.AddInt32("attr:alignment", 0 /* left */ );
	
	status_t setStatus = mime.SetAttrInfo(&msg);
	if (setStatus != B_OK) {
		printf("Error setting attributes: %s\n", strerror(setStatus));
		return 5;
	}

	return 0;
}
