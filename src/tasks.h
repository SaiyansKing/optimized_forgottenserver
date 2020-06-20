/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2020  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_TASKS_H_A66AC384766041E59DCA059DAB6E1976
#define FS_TASKS_H_A66AC384766041E59DCA059DAB6E1976

#include <condition_variable>
#include "thread_holder_base.h"
#include "enums.h"

class Task
{
	public:
		// DO NOT allocate this class on the stack
		explicit Task(std::function<void (void)>&& f) : func(std::move(f)) {}

		virtual ~Task() = default;
		void operator()() {
			func();
		}

	private:
		// Expiration has another meaning for scheduler tasks,
		// then it is the time the task should be added to the
		// dispatcher
		std::function<void (void)> func;
};

Task* createTask(std::function<void (void)> f);

class Dispatcher : public ThreadHolder<Dispatcher> {
	public:
		#if BOOST_VERSION >= 106600
		Dispatcher() : work(boost::asio::make_work_guard(io_service)) {}
		#else
		Dispatcher() : work(std::make_shared<boost::asio::io_service::work>(io_service)) {}
		#endif

		void addTask(std::function<void (void)> functor);
		void addTask(Task* task);

		void shutdown();

		uint64_t getDispatcherCycle() const {
			return dispatcherCycle;
		}

		void threadMain();

	private:
		std::thread thread;
		uint64_t dispatcherCycle = 0;
		boost::asio::io_service io_service;
		#if BOOST_VERSION >= 106600
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
		#else
		std::shared_ptr<boost::asio::io_service::work> work;
		#endif
};

extern Dispatcher g_dispatcher;

#endif
