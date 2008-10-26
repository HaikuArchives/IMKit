#include <Application.h>
#include <Entry.h>
#include <TranslationUtils.h>

#include <stdio.h>

#include <common/IMKitUtilities.h>
#include <libim/Contact.h>

//#pragma mark -

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage is\n\t%s {path to People file} {protocol} {path to image}\n",
			argv[0]);
		printf("Note: Protocol can be \"general\" to give a fall back icon\n");
		return -1;
	};

	BApplication app("application/x-vnd.BeClan.IMKit.IconSetter");

	entry_ref ref;
	if (get_ref_for_path(argv[1], &ref) == B_OK) {
		IM::Contact contact(&ref);
		BBitmap *originalIcon = BTranslationUtils::GetBitmap(argv[3]);
		
		if ( !originalIcon )
		{
			printf("Couldn't load image\n");
			return -1;
		}
		
		status_t ret = contact.SetBuddyIcon(argv[2], originalIcon);
		
		if (ret >= B_OK) {
			return 0;
		} else {
			printf("Error setting icon: %s (%ld)\n", strerror(ret), ret);
		};
	} else {
		printf("Couldn't find the People file you specified\n");
		return -1;
	};
};
