// Author: Tom Werner (MajorT), 2025

#pragma once

/**
 * Macro to provide a TArray-like interface for a property that needs to be accessed through FFastArraySerializer.
 * This is useful when you want to expose TArray functionality while maintaining the underlying FFastArraySerializer structure.
 *
 * @param	PropertyName	The name of the TArray property to wrap.
 *
 * CREDIT: namrog84 (https://github.com/namrog84)
 */
#define ITEMIZATION_FastArraySerializer_TArray_ACCESSORS(ClassName, PropertyType, PropertyName) \
	/* Const Iterators */ \
	auto begin() const { return PropertyName.begin(); } \
	auto end() const { return PropertyName.end(); } \
	auto rbegin() const { return PropertyName.rbegin(); } \
	auto rend() const { return PropertyName.rend(); } \
	/* Non-const iterators */ \
	auto begin() { return PropertyName.begin(); } \
	auto end() { return PropertyName.end(); } \
	auto rbegin() { return PropertyName.rbegin(); } \
	auto rend() { return PropertyName.rend(); } \
	\
	/* Const Element Access */ \
	const auto& operator[](int32 Index) const { return PropertyName[Index]; } \
	const auto& Last() const { return PropertyName.Last(); } \
	const auto& Top() const { return PropertyName.Top(); } \
	/* Non-const element access */ \
	auto& operator[](int32 Index) { return PropertyName[Index]; } \
	auto& Last() { return PropertyName.Last(); } \
	auto& Top() { return PropertyName.Top(); } \
	\
	/* Capacity */ \
	int32 Num() const { return PropertyName.Num(); } \
	int32 Max() const { return PropertyName.Max(); } \
	bool IsEmpty() const { return PropertyName.IsEmpty(); } \
	/* Capacity Modifiers */ \
	void Empty() { PropertyName.Empty(); } \
	void SetNum(int32 NewNum) { PropertyName.SetNum(NewNum); } \
	void Reserve(int32 NewCapacity) { PropertyName.Reserve(NewCapacity); } \
	void AddUninitialized(int32 Count) { PropertyName.AddUninitialized(Count); } \
	int32 AddDefaulted(int32 Count) { return PropertyName.AddDefaulted(Count); } \
	int32 AddDefaulted() { return PropertyName.AddDefaulted(); } \
	TArray<PropertyType>::TConstIterator CreateConstIterator() const { return PropertyName.CreateConstIterator(); } \
	TArray<PropertyType>::TIterator CreateIterator() { return PropertyName.CreateIterator(); } \
	friend TArray<PropertyType>::TConstIterator begin(const ClassName& Array) { return Array.CreateConstIterator(); } \
	friend TArray<PropertyType>::TConstIterator end(const ClassName& Array) { return TArray<PropertyType>::TConstIterator(Array.PropertyName, Array.PropertyName.Num()); } \
	friend TArray<PropertyType>::TIterator begin(ClassName& Array) { return Array.CreateIterator(); } \
	friend TArray<PropertyType>::TIterator end(ClassName& Array) { return TArray<PropertyType>::TIterator(Array.PropertyName, Array.PropertyName.Num()); }