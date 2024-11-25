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

#include "lava_types.hpp"

class LavaJobSystem {

public:

	LavaJobSystem();

	~LavaJobSystem();

	void run_tasks();

private:
	std::vector<std::thread> workers_;

	std::queue<std::function<void()>> tasks_;

	std::mutex lock_;

	std::condition_variable condition_var_;

	bool shutdown_;
};

#endif //__LAVA_JOB_SYSTEM__