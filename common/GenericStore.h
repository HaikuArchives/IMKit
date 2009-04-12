/*
 * Copyright 2009-, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors;
 *   Michael Davidson (slaad@bong.com.au)
 */
#ifndef GENERICSTORE_H
#define GENERICSTORE_H

#include <list>
#include <map>

#include "common/SpecificationFinder.h"

#include <stdio.h>

namespace IM {

	// The type_delete function is called as type_delete<type>([reallyDelete,] instance) - for reference types it's a no-op whilst pointer types will free instance
	template <typename T> void type_delete(T value) { type_delete<T>(true, value); };
	template <typename T> void type_delete(bool performDelete, T value) { };
	template <typename T> void type_delete(T *value) { type_delete<T>(true, value); };
	template <typename T> void type_delete(bool performDelete, T *value) {
		if (performDelete == true) {
			delete value;
		};
	};

	template<class K, class V>
	class GenericMapStore {
		public:
			typedef map<K, V>::iterator Iterator;
		
			GenericMapStore(void)
				: fOwn(true) {
			};
			
			GenericMapStore(bool own)
				: fOwn(own) {
			}
			
			virtual ~GenericMapStore(void) {
				Clear();
			};
			
		
			void Add(K key, V info) {
				if (Contains(key) == true) {
					Remove(key);
				};
				
				fStore[key] = info;
			};
			
			void Remove(K key) {
				Iterator it = fStore.find(key);
				if (it != End()) {
					fStore.erase(it);
					type_delete<V>(fOwn, it->second);
				};
			};
			
			bool Contains(K key) {
				return (fStore.find(key) != fStore.end());
			};
			
			V Find(K key) {
				V info = NULL;
				Iterator it = fStore.find(key);
	
				if (it != fStore.end()) {
					info = it->second;
				};
				
				return info;
			};
					
			void Clear(void) {
				Iterator it;
				
				for (it = Start(); it != End(); it++) {
					type_delete<V>(fOwn, it->second);
				};
				
				fStore.clear();
			};
			
			int32 CountItems(void) {
				return fStore.size();
			};
			
			Iterator Start(void) {
				return fStore.begin();
			};
			Iterator End(void) {
				return fStore.end();
			};
	
		private:
			bool fOwn;
			map<K, V> fStore;
	};
	
	template<class T>
	class GenericListStore {
		public:
			typedef list<T>::iterator Iterator;
		
			GenericListStore(void)
				: fOwn(true) {
			};

			GenericListStore(bool own)
				: fOwn(own) {
			};
			
			virtual ~GenericListStore(void) {
				Clear();
			};

			void Add(T value) {
				fStore.push_back(value);
			};
			
			void Remove(T value) {
				if (Contains(key) == true) {
					type_delete<T>(fOwn, *it);
					fStore.erase(it);
				};
			};
			
			bool Contains(T value) {
				return (find(fStore.begin(), fStore.end(), value) != fStore.end());
			};
			
//			T ItemAt(int32 index) {
//				return fStore[index];
//			};
				
			int32 CountItems(void) const {
				return fStore.size();
			};
			
			
			void Clear(void) {
				for (Iterator it = Start(); it != End(); it++) {
					type_delete<T>(fOwn, *it);
				};
				
				fStore.clear();
			};
			
			Iterator Start(void) {
				return fStore.begin();
			};
			Iterator End(void) {
				return fStore.end();
			};
	
			bool FindFirst(Specification<T> *specification, T *item, bool deleteSpec = true) {
				bool found = false;
			
				for (Iterator it = Start(); it != End(); it++) {
					T current = (*it);
				
					if (specification->IsSatisfiedBy(current) == true) {
						found = true;
						*item = current;
						break;
					};
				};
				
				if (deleteSpec == true) {
					delete specification;
				};
				
				return found;
			};
			
			GenericListStore<T> FindAll(Specification<T> *specification, bool deleteSpec = true) {
				GenericListStore<T> results(false);
				
				for (Iterator it = Start(); it != End(); it++) {
					T current = (*it);
				
					if (specification->IsSatisfiedBy(current) == true) {
						results.Add(current);
						break;
					};
				};
				
				if (deleteSpec == true) {
					delete specification;
				};
				
				return results;
			};
			
		private:
			bool fOwn;
			list<T> fStore;
	};
}

#endif // GENERICSTORE_H
