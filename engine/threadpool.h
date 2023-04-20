#pragma once
#include <vector>
#include <thread>
#include <atomic>

class ThreadPool
{
public:
	struct Worker
	{
		std::thread* thread = nullptr;
		bool isDone = true;
	};

	std::vector<Worker> pool;
	size_t size = 0;
	bool run = true;
	std::atomic<int> completed;

	ThreadPool(size_t _size);
	~ThreadPool();

	template<typename ARGTYPE>
	void InitThread(void(*work)(const ARGTYPE&), const ARGTYPE& arguments, size_t threadIndex)
	{
		void(*wrapper)(ThreadPool*, void(*)(const ARGTYPE&), const ARGTYPE&, size_t) =
			[](ThreadPool* self, void(*function)(const ARGTYPE&), const ARGTYPE& args, size_t index)
		{
			while (self->run)
			{
				if (!self->pool[index].isDone)
				{
					function(args);
					self->pool[index].isDone = true;;
					self->completed++;
				}
				else
				{
					std::this_thread::yield();
				}
			}
		};
		
		pool[threadIndex].thread = new std::thread(wrapper, this, work, arguments, threadIndex);
	}


	void ExecuteAndWait();
};