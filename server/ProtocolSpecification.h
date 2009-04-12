#ifndef PROTOCOLSPECIFICATION_H
#define PROTOCOLSPECIFICATION_H

#include "ProtocolInfo.h"
#include "common/Specification.h"

#include <support/String.h>

namespace IM {
	typedef Specification<ProtocolInfo *> ProtocolSpecification;
	typedef AndSpecification<ProtocolInfo *> AndProtocolSpecification;
	typedef OrSpecification<ProtocolInfo *> OrProtocolSpecification;
	typedef NotSpecification<ProtocolInfo *> NotProtocolSpecification;
	
	class AllProtocolSpecification : public ProtocolSpecification {
		public:
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return true;
								};
	};
	
	class SignatureProtocolSpecification : public ProtocolSpecification {
		public:
								SignatureProtocolSpecification(const char *signature)
									: ProtocolSpecification(),
									fSignature(signature) {
								};
								
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return (info->Signature() == fSignature);
								};
		
		private:
			BString				fSignature;
	};
	
	class InstanceProtocolSpecification : public ProtocolSpecification {
		public:
								InstanceProtocolSpecification(const char *instance)
									: ProtocolSpecification(),
									fInstance(instance) {
								};
								
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return (info->InstanceID() == fInstance);
								};
		
		private:
			BString				fInstance;
	};
	
	class AccountNameProtocolSpecification : public ProtocolSpecification {
		public:
								AccountNameProtocolSpecification(const char *name)
									: ProtocolSpecification(),
									fName(name) {
								};
								
								AccountNameProtocolSpecification(BString name)
									: ProtocolSpecification(),
									fName(name) {
								};
																
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return (info->AccountName() == fName);
								};
								
		private:
			BString				fName;
	};
	
	class CapabilityProtocolSpecification : public ProtocolSpecification {
		public:
								CapabilityProtocolSpecification(uint32 capability)
									: ProtocolSpecification(),
									fCapability(capability) {
								};
								
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return info->HasCapability(fCapability);
								};
		
		private:
			uint32				fCapability;

	};
	
	class ExitedProtocolSpecification : public ProtocolSpecification {
		public:
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return info->HasExited();
								};
	};
	
	class AllowRestartProtocolSpecification : public ProtocolSpecification {
		public:
			virtual bool		IsSatisfiedBy(ProtocolInfo *info) {
									return info->IsRestartAllowed();
								};
	};
};

#endif
