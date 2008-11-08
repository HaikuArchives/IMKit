#include "StatusIcon.h"

#include <stdlib.h>

using namespace IM;

//#pragma mark Constrcutor

StatusIcon::StatusIcon(void)
	: fVectorIcon(NULL),
	fVectorIconSize(0),
	fMiniIcon(NULL),
	fMiniIconSize(0),
	fLargeIcon(NULL),
	fLargeIconSize(0) {
}
	
StatusIcon::~StatusIcon(void) {
	if (fVectorIcon != NULL) free(fVectorIcon);
	if (fMiniIcon != NULL) free(fMiniIcon);
	if (fLargeIcon != NULL)	free(fLargeIcon);
}

//#pragma mark Public - Vector Icons
	
void StatusIcon::SetVectorIcon(const void* data, ssize_t size) {
	fVectorIcon = malloc(size);
	memcpy(fVectorIcon, data, size);
	fVectorIconSize = size;
}
	
const void *StatusIcon::VectorIcon(void) {
	return fVectorIcon;
}

ssize_t StatusIcon::VectorIconSize(void) {
	return fVectorIconSize;
}
	
//#pragma mark Public - Mini Icons

void StatusIcon::SetMiniIcon(const void* data, ssize_t size) {
	fMiniIcon = malloc(size);
	memcpy(fMiniIcon, data, size);
	fMiniIconSize = size;
}
	
const void *StatusIcon::MiniIcon(void) {
	return fMiniIcon;
}

ssize_t StatusIcon::MiniIconSize(void) {
	return fMiniIconSize;
}

//#pragma mark Public - Large Icons
	
void StatusIcon::SetLargeIcon(const void* data, ssize_t size) {
	fLargeIcon = malloc(size);
	memcpy(fLargeIcon, data, size);
	fLargeIconSize = size;
}
	
const void *StatusIcon::LargeIcon(void) {
	return fLargeIcon;
}

ssize_t StatusIcon::StatusIcon::LargeIconSize(void) {
	return fLargeIconSize;
}

//#pragma mark Public Methods
	
bool StatusIcon::IsEmpty(void) {
	return ((fVectorIcon == NULL) && (fMiniIcon == NULL) && (fLargeIcon == NULL));
}
