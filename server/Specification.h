#ifndef SPECIFICATION_H
#define SPECIFICATION_H

namespace IM {

	template<class T>
	class Specification {
		public:
			virtual				~Specification(void) {};
			virtual bool		IsSatisfiedBy(T item) = 0;
	};
	
	template<class T>
	class NotSpecification : public Specification<T> {
		public:
								NotSpecification(Specification<T> *negate)
									: fNegate(negate) {
								};
			virtual 			~NotSpecification(void) {
									delete fNegate;
								};
		
			virtual bool		IsSatisfiedBy(T item) {
									return fNegate->IsSatisfiedBy(item) == false;
								};
		
		private:
			Specification<T>	*fNegate;
	};
	
	template<class T>
	class CompositeSpecification : public Specification<T> {
		public:
								CompositeSpecification(Specification<T> *left, Specification<T> *right) 
									: fLeft(left),
									fRight(right) {
								};
			virtual				~CompositeSpecification(void) {
									delete fLeft;
									delete fRight;
								}
								
			Specification<T>	*Left(void) {
									return fLeft;
								};
			Specification<T>	*Right(void) {
									return fRight;
								};
		
		private:
			Specification<T> 	*fLeft;
			Specification<T> 	*fRight;
	};
	
	template<class T>
	class AndSpecification : public CompositeSpecification<T> {
		public:
								AndSpecification(Specification<T> *left, Specification<T> *right)
									: CompositeSpecification<T>(left, right) {
								};
		
			virtual bool		IsSatisfiedBy(T item) {
									return (Left()->IsSatisfiedBy(item) && Right()->IsSatisfiedBy(item));
								};
	};
	
	template<class T>
	class OrSpecification : public CompositeSpecification<T> {
		public:
								OrSpecification(Specification<T> *left, Specification<T> *right)
									: CompositeSpecification<T>(left, right) {
								};
		
			virtual bool		IsSatisfiedBy(T item) {
									return (Left()->IsSatisfiedBy(item) || Right()->IsSatisfiedBy(item));
								};
	};
};

#endif
