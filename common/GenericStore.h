/*
 * Copyright 2009-, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors;
 *   Michael Davidson (slaad@bong.com.au)
 */
#ifndef GENERICSTORE_H
#define GENERICSTORE_H

#include <map>

namespace IM {
template<class K, class V>
class GenericStore {
	public:
		GenericStore(void) {
		};
		
		virtual ~GenericStore(void) {
			Clear();
		};
		
	
		void Add(K key, V *info) {
			if (Contains(key) == true) {
				Remove(key);
			};
			
			fStore[key] = info;
		};
		
		void Remove(K key) {
			map<K, V *>::iterator it = fStore.find(key);
			if (it != End()) {
				fStore.erase(it);
				delete it->second;
			};
		};
		
		bool Contains(K key) {
			return (fStore.find(key) != fStore.end());
		};
		
		V Find(K key) {
			V *info = NULL;
			map<K, V *>::iterator it = fStore.find(key);

			if (it != fStore.end()) {
				info = it->second;
			};
			
			return BMessage(*info);
		};
				
		void Clear(void) {
			map<K, V *>::iterator it;
			
			for (it = Start(); it != End(); it++) {
				delete it->second;
			};
			
			fStore.clear();
		};
		
		map<K, V *>::iterator Start() {
			return fStore.begin();
		};
		map<K, V *>::iterator End() {
			return fStore.end();
		};

	private:
		map<K, V *> fStore;
};
};

#endif // GENERICSTORE_H
