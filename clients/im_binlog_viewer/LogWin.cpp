#include "LogWin.h"

#include <stdio.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

float kPadding = 10.0;

LogWin::LogWin(entry_ref contact, BRect size) 
	: BWindow(size, _T("IM Kit - Log Viewer"), B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS),
	fEntryRef(contact),
	fThreadID(0),
	fContact(&contact) {
	
	fView = new BView(Bounds(), "MainView", B_FOLLOW_ALL,
		B_ASYNCHRONOUS_CONTROLS | B_WILL_DRAW);
		
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetLowUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetHighUIColor(B_UI_PANEL_TEXT_COLOR);
#else
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(0, 0, 0, 0);
#endif	

	AddChild(fView);
	fView->Show();

	BRect listFrame = Bounds();
	listFrame.InsetBy(kPadding, kPadding);

	fCLV = new BColumnListView(listFrame, "Listing", B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW, B_PLAIN_BORDER);
	fView->AddChild(fCLV);

	fDate = new BDateColumn(_T("Date"),
		be_plain_font->StringWidth("XXX, XXX 00 0000, 00:00 AM") + kPadding,
		be_plain_font->StringWidth("00/00/00") + kPadding,
		be_plain_font->StringWidth("Xxxxxxx, Xxxxxxxx XX XXXX, XX:XX:XX XX") + kPadding,
		B_ALIGN_RIGHT);
	fCLV->AddColumn(fDate, coDate);
	
	fProtocol = new BBitmapColumn(_T("Protocol"),
		be_plain_font->StringWidth(_T("Protocol")) + (kPadding * 2),
		kSmallIcon, be_plain_font->StringWidth(_T("Protocol")) + (kPadding * 3), B_ALIGN_CENTER);
	fCLV->AddColumn(fProtocol, coProtocol);

	fSender = new BStringColumn(_T("Sender"),
		be_plain_font->StringWidth(_T("Sender")) + (kPadding * 2),
		be_plain_font->StringWidth(_T("Sender")) + (kPadding * 2),
		150, B_ALIGN_LEFT);
	fCLV->AddColumn(fSender, coSender);
	
	fContents = new BStringColumn(_T("Contents"),
		be_plain_font->StringWidth(_T("Contents")) + (kPadding * 2) + 100,
		be_plain_font->StringWidth(_T("Contents")) + (kPadding * 2),
		300, B_ALIGN_LEFT);
	fCLV->AddColumn(fContents, coContents);
	
	fType = new BStringColumn(_T("Type"),
		be_plain_font->StringWidth("STATUS") + (kPadding * 2),
		be_plain_font->StringWidth("STATUS") + (kPadding * 2),
		100, B_ALIGN_LEFT);
	fCLV->AddColumn(fType, coType); 

	fCLV->SetSortingEnabled(true);
	fCLV->SetSortColumn(fDate, true, false);
	
	fCLV->SetInvocationMessage(new BMessage(lwMsgViewMsg));
	
	BPath iconDir;
	find_directory(B_USER_ADDONS_DIRECTORY, &iconDir, true);
	iconDir.Append("im_kit/protocols");
	BDirectory dir(iconDir.Path());
	dir.Rewind();
	BEntry entry;

	while (dir.GetNextEntry(&entry, true) == B_OK) {
		entry_ref ref;
		char protName[B_FILE_NAME_LENGTH];
		BString path = iconDir.Path();
		BBitmap *icon;
		
		entry.GetName(protName);
		entry.GetRef(&ref);

		path << "/" << protName;

		icon = ReadNodeIcon(path.String());
		fIcons[protName] = icon;
	};
	
	BString title = _T("IM Kit - Log Viewer");
	char name[512];
	char nick[512];
	
	if (fContact.GetName(name, sizeof(name)) != B_OK) {
		fName = _T("Unknown Name");
	} else {
		fName = name;
	};
	if (fContact.GetNickname(nick, sizeof(nick)) != B_OK) {
		fNick = _T("Unknown Nick");
	} else {
		fNick = nick;
	};
	
	title << ": " << fName << " (" << fNick << ")";
	SetTitle(title.String());

	fThreadID = spawn_thread(GenerateContents, "LogViewApp: Lister",
		B_NORMAL_PRIORITY, (void *)this);
	if (fThreadID > B_ERROR) resume_thread(fThreadID);
	
	Show();
	
	fMan = new IM::Manager(BMessenger(this));
};

LogWin::~LogWin(void) {
};

bool LogWin::HandlesRef(entry_ref ref) {
	return (fEntryRef == ref);
};

void LogWin::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case IM::MESSAGE: {
			static int32 j;
			entry_ref contact;
			if (msg->FindRef("contact",&contact) != B_OK) {
				printf("Couldn't extract ze contact! Mein lieben!\n");
				return;
			};
			if (contact != fEntryRef) {
				printf("Contact not ours! Mein lieben! %s vs %s\n", contact.name, fEntryRef.name);
				msg->PrintToStream();
//				return;
			};

			int32 im_what = msg->FindInt32("im_what");
			const char *protocol;
			msg->FindString("protocol", &protocol);
			iconmap::iterator i = fIcons.find(protocol);	
		
			BRow *row = new BRow();
			time_t t = 0;
			msg->FindInt32("time_t", (int32 *)&t);
			
			fMessages[t] = new BMessage(*msg);
			
			row->SetField(new BDateField(&t), coDate);
			row->SetField(new BBitmapField(i->second), coProtocol);

			switch(im_what) {
				case IM::MESSAGE_RECEIVED: {
					row->SetField(new BStringField(fName.String()), coSender);
					const char *message;
					if (msg->FindString("message", &message) == B_OK) {
						row->SetField(new BStringField(message), coContents);
					} else {
						row->SetField(new BStringField(_T("Error")), coContents);
					};
					row->SetField(new BStringField("MSG"), coType);
				} break;
				case IM::MESSAGE_SENT: {
					row->SetField(new BStringField(_T("You")), coSender);
					const char *message;
					if (msg->FindString("message", &message) == B_OK) {
						row->SetField(new BStringField(message), coContents);
					} else {
						row->SetField(new BStringField(_T("Error")), coContents);
					};
					row->SetField(new BStringField("MSG"), coType);
				} break;
				case IM::STATUS_CHANGED: {
					row->SetField(new BStringField(fName.String()), coSender);
					const char *status;
					if (msg->FindString("status", &status) == B_OK) {
						row->SetField(new BStringField(status), coContents);
					} else {
						row->SetField(new BStringField(_T("Unknown")), coContents);
					};
					row->SetField(new BStringField("STATUS"), coType);
				} break;
				default: {
					printf("we were discarded :~(\n");
					msg->PrintToStream();
				};
			};
			
			fCLV->AddRow(row);					
		} break;
		
		case lwMsgViewMsg: {
			printf("All I can dooooooo\n");
			msg->PrintToStream();
			BRow *row = fCLV->FocusRow();
			printf("Focus Row: %p\n", row);
			
		} break;

		default:
			BWindow::MessageReceived(msg);
	};
};

bool LogWin::QuitRequested(void) {
	return BWindow::QuitRequested();;
};

int32 LogWin::GenerateContents(void *arg) {
	LogWin *win = (LogWin *)arg;
	BMessenger msgr = BMessenger(win);
	
	BString path = "/boot/home/Logs/IM/binlog/";

	int32 length = 0;
	char *logPath = ReadAttribute(BNode(&win->fEntryRef), "IM:binarylog", &length);
	if (logPath == NULL) return B_ERROR;
	
	path.Append(logPath, length);
	free(logPath);
	path << "/";
	
	BDirectory logDir(path.String());
	if (logDir.InitCheck() != B_OK) return B_ERROR;

	logDir.Rewind();
	entry_ref logFileRef;
	
	while (logDir.GetNextRef(&logFileRef) == B_OK) {
		BFile logFile(&logFileRef, B_READ_ONLY);
		if (logFile.InitCheck() == B_OK) {
			BMessage msg;
			while (msg.Unflatten(&logFile) == B_OK) {
				msgr.SendMessage(&msg);
			};
		};
	};
};

