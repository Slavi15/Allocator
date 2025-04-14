#pragma once

#include "Allocator.h"

#define NODISCARD [[nodiscard]]
#define TT template <typename T>

class FreeListAllocator : public Allocator
{
private:
	static constexpr uint64_t MAX_SIZE = 1024;
	static constexpr uint64_t MIN_CHUNK_SIZE = 32;

	uint64_t get_next_power_two(uint64_t n) const;

	struct Header
	{
		uint64_t size;
		Header* xor_ptr;
		bool is_free;

		Header* get_next(Header* prev) const;
		void set_xor(Header* prev, Header* next);
	};

	NODISCARD Header* find_free_block(uint64_t size) const;
	void remove_free_block(Header* block);
	void add_free_block(Header* block);

	void merge(Header* block);

public:
	FreeListAllocator(uint64_t size = MAX_SIZE * MAX_SIZE);

	TT
	NODISCARD T* alloc(uint64_t size);

	NODISCARD void* alloc(uint64_t size) override;
	void free(void* ptr) override;

	void print() const;

	~FreeListAllocator() noexcept;

private:
	void* memory;
	uint64_t size;
	Header* freeHead;
};
