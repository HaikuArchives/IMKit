#include <libim/Constants.h>
#include <libim/Manager.h>

#include <stdio.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Correct usage is:\n\t%s {protocol} {id} {message}\n", argv[0]);
		printf("Eg. %s \"AIM\" \"George\" \"Hi George. I'm too cool for school\"\n",
			argv[0]);
		return -1;
	};

	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::SEND_MESSAGE);
	msg.AddString("protocol", argv[1]);
	msg.AddString("id", argv[2]);
	msg.AddString("message", argv[3]);

	IM::Manager::OneShotMessage(&msg);	

	return 0;
};
