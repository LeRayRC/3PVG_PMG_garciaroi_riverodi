/**
 * @file dynamic_ptr.hpp
 * @author Daniel Rivero Díaz(riverodi@esat-alumni.com)
 * @brief Dynamic Pointer's header
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef  __DYNAMIC_PTR_
#define  __DYNAMIC_PTR_ 1

template<class T>
class DynamicPtr {

public:

	T* Get() { return ObjectPtr; };

	void UpdatePointer(T* new_location) { ObjectPtr = new_location; };

private:
	T* ObjectPtr;
};

#endif // ! __DYNAMIC_PTR_