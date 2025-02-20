/**
 * @file lava_job_system.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Job System's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef  __LAVA_JOB_SYSTEM__
#define  __LAVA_JOB_SYSTEM__ 1

#include "lava/common/lava_types.hpp"
#include <future>

class LavaJobSystem {

public:

	LavaJobSystem(); 

	~LavaJobSystem();

	void run_tasks();

	void add_task(std::move_only_function<void()> task);

	template<typename T> typename std::future<std::invoke_result_t<T>> add(T&& f) {
		typedef typename std::invoke_result_t<T> result;
		std::packaged_task<result()> task(std::move(f));
		auto future = task.get_future();

		add_task(std::move(task));
		return future;
	}


private:

	LavaJobSystem(LavaJobSystem&&) = delete;

	LavaJobSystem& operator=(LavaJobSystem&&) = delete;

	std::vector<std::thread> workers_;

	std::queue<std::move_only_function<void()>> tasks_;

	std::mutex lock_;

	std::condition_variable condition_var_;

	bool shutdown_;
};

#endif //__LAVA_JOB_SYSTEM__