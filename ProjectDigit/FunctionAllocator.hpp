#pragma once

#include <set>
#include <vector>
#include "Function.hpp"

using namespace std;

class FunctionAllocator {
public:

	FunctionAllocator() {}

	void initalaize() { if (functionNames.empty()) _initalaize(functionNames); }

	const bool isAvailable(const string name) {
		if (functionNames.find(name) == functionNames.end())
			return false;
		else
			return true;
	}

	shared_ptr<Function> allocate(const string& name) { return shared_ptr<Function>(_allocate(name)); }

private:

	virtual void _initalaize(set<string>&) = 0;
	virtual Function* _allocate(const string& name) = 0;

	set<string> functionNames;
};

class FunctionAllocatorManager {
public:

	void addAllocator(FunctionAllocator* allocator) { 
		allocator->initalaize();
		allocators.push_back(allocator);
	}

	shared_ptr<Function> allocate(const string name) {
		shared_ptr<Function> func = nullptr;
		for (FunctionAllocator* i : allocators) {
			if (i->isAvailable(name)) {
				func = i->allocate(name);
				break;
			}
		}
		return func;
	}

private:
	vector<FunctionAllocator*> allocators;
};

FunctionAllocatorManager functionAllocatorManager;
