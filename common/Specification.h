/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 */
#ifndef _COMMON_SPECIFICATION_H
#define _COMMON_SPECIFICATION_H

namespace IM {
	template<class T>
	class Specification {
	public:
		virtual			~Specification() {}
		virtual	bool	IsSatisfiedBy(T item) = 0;
	};

	template<class T>
	class NotSpecification : public Specification<T> {
	public:
							NotSpecification(Specification<T> *negate)
								: fNegate(negate)
							{
							}
		virtual 			~NotSpecification()
							{
								delete fNegate;
							}
		virtual bool		IsSatisfiedBy(T item)
							{
								return !fNegate->IsSatisfiedBy(item);
							}

	private:
		Specification<T>*	fNegate;
	};

	template<class T>
	class CompositeSpecification : public Specification<T> {
	public:
										CompositeSpecification(Specification<T>* left,
															   Specification<T>* right)
											: fLeft(left),
											fRight(right)
										{
										}
		virtual							~CompositeSpecification()
										{
											delete fLeft;
											delete fRight;
										}

					Specification<T>*	Left()
										{
											return fLeft;
										}
					Specification<T>*	Right()
										{
											return fRight;
										}

	private:
		Specification<T>* 				fLeft;
		Specification<T>* 				fRight;
	};

	template<class T>
	class AndSpecification : public CompositeSpecification<T> {
	public:
						AndSpecification(Specification<T>* left, Specification<T>* right)
							: CompositeSpecification<T>(left, right)
						{
						}

						Specification<T>*	Left()
											{
												return CompositeSpecification<T>::Left();
											}

						Specification<T>*	Right()
											{
												return CompositeSpecification<T>::Right();
											}

		virtual bool	IsSatisfiedBy(T item)
						{
							return Left()->IsSatisfiedBy(item) && Right()->IsSatisfiedBy(item);
						}
	};

	template<class T>
	class OrSpecification : public CompositeSpecification<T> {
	public:
						OrSpecification(Specification<T>* left, Specification<T>* right)
							: CompositeSpecification<T>(left, right)
						{
						}

						Specification<T>*	Left()
											{
												return CompositeSpecification<T>::Left();
											}

						Specification<T>*	Right()
											{
												return CompositeSpecification<T>::Right();
											}

		virtual bool	IsSatisfiedBy(T item)
						{
							return Left()->IsSatisfiedBy(item) || Right()->IsSatisfiedBy(item);
						}
	};
};

#endif	// _COMMON_SPECIFICATION_H
