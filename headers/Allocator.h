#pragma once

#include <iostream>
#include <cstdint>

class Allocator
{
public:
	virtual void* alloc(uint64_t size) = 0;
	virtual void free(void* ptr) = 0;

	virtual ~Allocator() = default;
};
