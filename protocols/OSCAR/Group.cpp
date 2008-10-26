#include "Group.h"

//#pragma mark Constructor

Group::Group(int id, const char *name)
	: fId(id),
	fName(name) {
};

Group::~Group(void) {
};

//#pragma mark Public Methods	

int16 Group::Id(void) {
	return fId;
};

const char *Group::Name(void) {
	return fName.String();
};
		
int16 Group::ItemsInGroup(void) {
	return fChildren.size();
};

int16 Group::ItemAt(int16 index) {
	return fChildren[index];
};

void Group::AddItem(int16 item) {
	fChildren.push_back(item);
};

