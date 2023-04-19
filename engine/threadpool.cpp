#include "threadpool.h"

ThreadPool::ThreadPool(size_t _size) :
	completed(0)
{
	size = _size;
	pool.reserve(size);
	for (size_t i = 0; i < size; i++)
		pool.push_back({});
}

ThreadPool::~ThreadPool()
{
	run = false;
	for (size_t i = 0; i < size; i++)
	{
		pool[i].thread->join();
		delete pool[i].thread;
	}
}

void ThreadPool::ExecuteAndWait()
{
	for (size_t i = 0; i < size; i++)
		pool[i].isDone = false;

	while (completed.load(std::memory_order_acquire) < size);

	completed.store(0, std::memory_order_release);
}