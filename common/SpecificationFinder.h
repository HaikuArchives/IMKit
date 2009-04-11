#ifndef COMMON_SPECIFICATION_FINDER_H
#define COMMON_SPECIFICATION_FINDER_H

#include "common/GenericStore.h"
#include "common/Specification.h"

namespace IM {
	template<class T>
	class SpecificationFinder {
		public:
			virtual T			FindFirst(Specification<T> *specfication, bool deleteSpec = true) = 0;
			virtual GenericListStore<T>
								FindAll(Specification<T> *specification, bool deleteSpec = true) = 0;
	};	
};

#endif // COMMON_SPECIFICATION_FINDER_H
