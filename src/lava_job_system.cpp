/**
 * @file lava_job_system.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Job System's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava_job_system.hpp"

LavaJobSystem::LavaJobSystem()
{
	shutdown_ = false;
	int max_thread = std::thread::hardware_concurrency();

	for (int i = 0; i < max_thread; i++) 
		workers_.push_back(std::thread{ [this]() {run_tasks(); } });
}

LavaJobSystem::~LavaJobSystem()
{
	{
		// Unblock any threads and tell them to stop
		std::unique_lock <std::mutex> l(lock_);
	
		shutdown_ = true;
		condition_var_.notify_all();
	}

	for (int i = 0; i < workers_.size(); i++) {
		workers_[i].join();
	}
}

void LavaJobSystem::run_tasks()
{
	std::move_only_function<void()> task;

	while (1) {
		{
			std::unique_lock<std::mutex> unq_lock(lock_);
			condition_var_.wait(unq_lock, [this] { return !tasks_.empty() || shutdown_; });
			if (tasks_.empty()) return;
			task = std::move(tasks_.front());
			tasks_.pop();
		}
		task();
	}
}

void LavaJobSystem::add_task(std::move_only_function<void()> task)
{
	{
		std::lock_guard<std::mutex> lock{ lock_ };
		tasks_.emplace(std::move(task));
	}
	condition_var_.notify_one();
}