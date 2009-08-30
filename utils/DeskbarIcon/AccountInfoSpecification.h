#ifndef ACCOUNTINFOSPECIFICATION_H
#define ACCOUNTINFOSPECIFICATION_H

#include <support/String.h>

#include <libim/AccountInfo.h>

#include "common/Specification.h"

typedef IM::Specification<IM::AccountInfo *> AccountInfoSpecification;

class AllAccountInfoSpecification : public AccountInfoSpecification {
	public:
		virtual bool		IsSatisfiedBy(IM::AccountInfo *info) {
								return true;
							};
};

class InstanceAccountInfoSpecification : public AccountInfoSpecification {
	public:
							InstanceAccountInfoSpecification(const char *instance)
								: AccountInfoSpecification(),
								fInstance(instance) {
							};
							
		virtual bool		IsSatisfiedBy(IM::AccountInfo *info) {
								return (info->ID() == fInstance);
							};
	
	private:
		BString				fInstance;
};

#endif //ACCOUNTINFOSPECIFICATION_H
