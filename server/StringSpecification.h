#ifndef STRINGSPECIFICATION_H
#define STRINGSPECIFICATION_H

#include "common/Specification.h"

#include <support/String.h>

namespace IM {
	typedef Specification<BString> StringSpecification;
	
	class AllStringSpecification : public StringSpecification {
		public:
			virtual bool		IsSatisfiedBy(BString item) {
									return true;
								};
	};
	
	class EqualStringSpecification : public StringSpecification {
		public:
								EqualStringSpecification(const char *compare)
									: StringSpecification(),
									fCompare(compare) {
								};
		
			virtual bool		IsSatisfiedBy(BString item) {
									return (fCompare == item);
								};
		
		private:
			BString				fCompare;
	};
};

#endif // STRINGSPECIFICATION_H
