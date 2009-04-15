#ifndef ACCOUNTINFOSPECIFICATION_H
#define ACCOUNTINFOSPECIFICATION_H

#include "AccountInfo.h"
#include "common/Specification.h"

#include <support/String.h>

typedef IM::Specification<AccountInfo *> AccountInfoSpecification;

class AllAccountInfoSpecification : public AccountInfoSpecification {
	public:
		virtual bool		IsSatisfiedBy(AccountInfo *info) {
								return true;
							};
};

class InstanceAccountInfoSpecification : public AccountInfoSpecification {
	public:
							InstanceAccountInfoSpecification(const char *instance)
								: AccountInfoSpecification(),
								fInstance(instance) {
							};
							
		virtual bool		IsSatisfiedBy(AccountInfo *info) {
								return (info->ID() == fInstance);
							};
	
	private:
		BString				fInstance;
};

#endif //ACCOUNTINFOSPECIFICATION_H
