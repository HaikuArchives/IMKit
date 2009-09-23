/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_INFO_SPECIFICATION_H
#define _ACCOUNT_INFO_SPECIFICATION_H

#include <String.h>

#include <libim/AccountInfo.h>

#include <common/Specification.h>

typedef IM::Specification<IM::AccountInfo*> AccountInfoSpecification;

class AllAccountInfoSpecification : public AccountInfoSpecification {
public:
	virtual bool IsSatisfiedBy(IM::AccountInfo* info)
	{
		return true;
	};
};


class InstanceAccountInfoSpecification : public AccountInfoSpecification {
public:
	InstanceAccountInfoSpecification(const char* instance)
		: AccountInfoSpecification(),
		fInstance(instance)
	{
	}

	virtual bool IsSatisfiedBy(IM::AccountInfo* info)
	{
		return info->ID() == fInstance;
	}

private:
	BString fInstance;
};

#endif	// _ACCOUNT_INFO_SPECIFICATION_H
