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

#ifndef FS_SCHEDULER_H_2905B3D5EAB34B4BA8830167262D2DC1
#define FS_SCHEDULER_H_2905B3D5EAB34B4BA8830167262D2DC1

#include "tasks.h"
#include <atomic>

#include "thread_holder_base.h"

static constexpr int32_t SCHEDULER_MINTICKS = 50;

class SchedulerTask : public Task
{
	public:
		void setEventId(uint64_t id) {
			eventId = id;
		}
		uint64_t getEventId() const {
			return eventId;
		}
		uint32_t getDelay() const {
			return delay;
		}

	private:
		SchedulerTask(uint32_t delay, std::function<void (void)>&& f) : Task(std::move(f)), delay(delay) {}

		uint64_t eventId = 0;
		uint32_t delay = 0;

		friend SchedulerTask* createSchedulerTask(uint32_t, std::function<void (void)>);
};

SchedulerTask* createSchedulerTask(uint32_t delay, std::function<void (void)> f);

class Scheduler : public ThreadHolder<Scheduler>
{
	public:
		#if BOOST_VERSION >= 106600
		Scheduler() : work(boost::asio::make_work_guard(io_service)) {}
		#else
		Scheduler() : work(std::make_shared<boost::asio::io_service::work>(io_service)) {}
		#endif

		uint64_t addEvent(SchedulerTask* task);
		void stopEvent(uint64_t eventId);

		void shutdown();

		void threadMain();

	private:
		std::thread thread;
		std::atomic<uint64_t> lastEventId {0};
		std::map<uint64_t, boost::asio::deadline_timer> eventIds;
		boost::asio::io_service io_service;
		#if BOOST_VERSION >= 106600
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
		#else
		std::shared_ptr<boost::asio::io_service::work> work;
		#endif
};

extern Scheduler g_scheduler;

#endif
