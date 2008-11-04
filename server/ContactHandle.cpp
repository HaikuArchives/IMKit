#include "ContactHandle.h"

//#pragma mark Constructor

ContactHandle::ContactHandle(void) {
};
		
ContactHandle::ContactHandle(const ContactHandle & c)
	: node(c.node) {
	entry.directory = c.entry.directory;
	entry.device = c.entry.device;
	entry.set_name(c.entry.name);
};

//#pragma mark Operators		
bool ContactHandle::operator < (const ContactHandle &c) const {
	if (entry.device != c.entry.device) {
		return entry.device < c.entry.device;
	};
	
	return node < c.node;
}

bool ContactHandle::operator == (const ContactHandle &c) const {
	return (node == c.node && entry.device == c.entry.device);
}
