#include "../headers/FreeListAllocator.h"

uint64_t FreeListAllocator::get_next_power_two(uint64_t n) const
{
	if (n < MIN_CHUNK_SIZE) return MIN_CHUNK_SIZE;

	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	n++;

	return n;
}

NODISCARD FreeListAllocator::Header* FreeListAllocator::find_free_block(uint64_t size) const
{
	Header* curr = this->freeHead, * prev = nullptr, * next = nullptr;

	while (curr)
	{
		if (curr->is_free && curr->size >= size)
		{
			return curr;
		}

		next = curr->get_next(prev);
		prev = curr;
		curr = next;
	}

	return nullptr;
}

void FreeListAllocator::remove_free_block(Header* block)
{
	Header* current = this->freeHead, * prev = nullptr;

	while (current && current != block)
	{
		Header* next = current->get_next(prev);
		prev = current;
		current = next;
	}

	if (current)
	{
		Header* next = current->get_next(prev);

		if (prev)
		{
			prev->set_xor(prev->get_next(current), next);
		}
		else
		{
			freeHead = next;
		}

		if (next)
		{
			next->set_xor(prev, next->get_next(current));
		}
	}
}

void FreeListAllocator::add_free_block(Header* block)
{
	block->set_xor(nullptr, this->freeHead);

	if (this->freeHead)
	{
		Header* next = this->freeHead->get_next(nullptr);
		this->freeHead->set_xor(block, next);
	}

	this->freeHead = block;
}

void FreeListAllocator::merge(Header* block)
{
	Header* rhs = reinterpret_cast<Header*>(reinterpret_cast<char*>(block) + sizeof(Header) + block->size);

	if (rhs < reinterpret_cast<Header*>(reinterpret_cast<char*>(this->memory) + this->size) && rhs->is_free)
	{
		remove_free_block(rhs);
		block->size += sizeof(Header) + rhs->size;
	}
}

FreeListAllocator::FreeListAllocator(uint64_t size) :
	memory(::operator new(size)),
	size(size),
	freeHead(nullptr)
{
	this->freeHead = static_cast<Header*>(this->memory);
	this->freeHead->size = this->size - sizeof(Header);
	this->freeHead->xor_ptr = nullptr;
	this->freeHead->is_free = true;
}

NODISCARD void* FreeListAllocator::alloc(uint64_t size)
{
	if (this->size == 0) return nullptr;

	uint64_t totalSize = std::max(get_next_power_two(size + sizeof(Header)), MIN_CHUNK_SIZE);
	uint64_t allocSize = totalSize - sizeof(Header);

	Header* block = find_free_block(allocSize);

	if (!block) return nullptr;

	remove_free_block(block);

	uint64_t remainingSize = block->size - allocSize;

	if (remainingSize > sizeof(Header) + MIN_CHUNK_SIZE)
	{
		Header* newBlock = reinterpret_cast<Header*>(reinterpret_cast<char*>(block) + sizeof(Header) + allocSize);

		newBlock->size = remainingSize - sizeof(Header);
		newBlock->is_free = true;
		add_free_block(newBlock);

		block->size = allocSize;

		this->freeHead = newBlock;
	}

	block->is_free = false;

	return reinterpret_cast<void*>(reinterpret_cast<char*>(block) + sizeof(Header));
}

TT
NODISCARD T* FreeListAllocator::alloc(uint64_t size)
{
	return reinterpret_cast<T*>(alloc(size * sizeof(T)));
}

void FreeListAllocator::free(void* ptr)
{
	Header* block = reinterpret_cast<Header*>(reinterpret_cast<char*>(ptr) - sizeof(Header));
	block->is_free = true;

	add_free_block(block);

	merge(block);
}

void FreeListAllocator::print() const
{
	std::cout << this->memory << " - " << reinterpret_cast<uintptr_t>(this->memory) + this->size << std::endl;

	Header* curr = reinterpret_cast<Header*>(memory);

	while (curr < reinterpret_cast<Header*>(reinterpret_cast<char*>(this->memory) + this->size))
	{
		std::cout << "Block at " << curr << " size: " << curr->size << " status: " << (curr->is_free ? "free" : "allocated") << std::endl;
		curr = reinterpret_cast<Header*>(reinterpret_cast<char*>(curr) + sizeof(Header) + curr->size);
	}
}

FreeListAllocator::~FreeListAllocator() noexcept
{
	::operator delete(this->memory);
}

FreeListAllocator::Header* FreeListAllocator::Header::get_next(Header* prev) const
{
	return reinterpret_cast<Header*>(reinterpret_cast<uintptr_t>(xor_ptr) ^ reinterpret_cast<uintptr_t>(prev));
}

void FreeListAllocator::Header::set_xor(Header* prev, Header* next)
{
	this->xor_ptr = reinterpret_cast<Header*>(reinterpret_cast<uintptr_t>(prev) ^ reinterpret_cast<uintptr_t>(next));
}
