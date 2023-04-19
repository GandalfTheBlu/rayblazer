#pragma once

template<typename T>
class MemoryPool
{
private:
	T* pool = nullptr;
	int size = 0;
	int topIndex = -1;

public:
	MemoryPool(int _size)
	{
		size = _size;
		pool = new T[size];
	}
	~MemoryPool()
	{
		delete[] pool;
	}

	T* GetNew()
	{
		if (++topIndex < size)
			return pool + topIndex;

		return nullptr;
	}

	T* operator[](int i)
	{
		if (i >= 0 && i <= topIndex)
			return pool + i;

		return nullptr;
	}

	int Count()
	{
		return topIndex+1;
	}
};