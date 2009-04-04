#include "SettingsInfo.h"

#include <interface/View.h>

#include "SettingsController.h"

//#pragma mark Constructor

SettingsInfo::SettingsInfo(settings_type type, BPath path, BString name)
	: fType(type),
	fPath(path),
	fName(name),
	fView(NULL),
	fController(NULL) {
};

//#pragma mark Public

settings_type SettingsInfo::Type(void) const {
	return fType;
};

BPath SettingsInfo::Path(void) const {
	return fPath;
};

const char *SettingsInfo::Name(void) const {
	return fName.String();
};

BView *SettingsInfo::View(void) const {
	return fView;
};

void SettingsInfo::View(BView *view) {
	fView = view;
};

SettingsController *SettingsInfo::Controller(void) const {
	return fController;
};

void SettingsInfo::Controller(SettingsController *controller) {
	fController = controller;
};

BMessage SettingsInfo::Template(void) const {
	return fTemplate;
};

void SettingsInfo::Template(BMessage tmplate) {
	fTemplate = tmplate;
};

BMessage SettingsInfo::Settings(void) const {
	return fSettings;
};

void SettingsInfo::Settings(BMessage settings) {
	fSettings = settings;
};
