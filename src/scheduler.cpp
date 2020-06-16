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

#include "otpch.h"

#include "scheduler.h"

void Scheduler::threadMain()
{
	io_service.run();
}

uint64_t Scheduler::addEvent(SchedulerTask* task)
{
	if (task->getEventId() == 0) {
		task->setEventId(++lastEventId);
	}

	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	[this, task]() {
		auto res = eventIds.emplace(std::piecewise_construct, std::forward_as_tuple(task->getEventId()), std::forward_as_tuple(io_service));

		boost::asio::deadline_timer& timer = res.first->second;
		timer.expires_from_now(boost::posix_time::milliseconds(task->getDelay()));
		timer.async_wait([this, task](const boost::system::error_code& error) {
			eventIds.erase(task->getEventId());

			if (error == boost::asio::error::operation_aborted || getState() == THREAD_STATE_TERMINATED) {
				delete task;
				return;
			}

			g_dispatcher.addTask(task);
		});
	});

	return task->getEventId();
}

void Scheduler::stopEvent(uint64_t eventId)
{
	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	[this, eventId]() {
		auto it = eventIds.find(eventId);
		if (it != eventIds.end()) {
			it->second.cancel();
		}
	});
}

void Scheduler::shutdown()
{
	setState(THREAD_STATE_TERMINATED);
	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	[this]() {
		for (auto& it : eventIds) {
			it.second.cancel();
		}

		io_service.stop();
	});
}

SchedulerTask* createSchedulerTask(uint32_t delay, std::function<void (void)> f)
{
	return new SchedulerTask(delay, std::move(f));
}
