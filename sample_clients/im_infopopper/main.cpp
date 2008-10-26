#include "IMInfoApp.h"

#include <libim/Helpers.h>
#include <libim/Constants.h>
#include <libinfopopper/InfoPopper.h>

#include <Message.h>

// -------------- MAIN -----------------

int main(void) {
	IMInfoApp app;

	// Save settings template
	BMessage autostart;
	autostart.AddString("name", "auto_start");
	autostart.AddString("description", "Auto-start");
	autostart.AddInt32("type", B_BOOL_TYPE);
	autostart.AddBool("default", true);
	autostart.AddString("help", "Should this be started by im_server?");
	
	BMessage appsig;
	appsig.AddString("name", "app_sig");
	appsig.AddString("description", "Application signature");
	appsig.AddInt32("type", B_STRING_TYPE);
	appsig.AddString("default", "application/x-vnd.beclan.IM_InfoPopper");
	
	BMessage status;
	status.AddString("name", "status_text");
	status.AddString("description", "Status change text");
	status.AddInt32("type", B_STRING_TYPE);
	status.AddString("default", "$nickname$ is now $status$");
	status.AddString("help","Here you can alter the way 'status change'\n"
	"messages are displayed"
	);
	
	BMessage msg;
	msg.AddString("name", "msg_text");
	msg.AddString("description", "Message received text");
	msg.AddInt32("type", B_STRING_TYPE);
	msg.AddString("default", "$nickname$ says $shortmsg$");
	msg.AddString("help","Here you can alter the way 'message received'\n"
	"messages are displayed"
	);
	
/*	BMessage iconSize;
	iconSize.AddString("name", "icon_size");
	iconSize.AddString("description", "Icon size");
	iconSize.AddInt32("type", B_INT32_TYPE);
	iconSize.AddInt32("default", 16);
*/	
	BMessage tmplate(IM::SETTINGS_TEMPLATE);
	tmplate.AddMessage("setting", &autostart);
	tmplate.AddMessage("setting", &appsig);
	tmplate.AddMessage("setting", &status);
	tmplate.AddMessage("setting", &msg);
//	tmplate.AddMessage("setting", &iconSize);
	
	im_save_client_template("im_infopopper", &tmplate);
	
	app.Run();
}
