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

#include "tasks.h"
#include "game.h"

extern Game g_game;

Task* createTask(std::function<void (void)> f)
{
	return new Task(std::move(f));
}

void Dispatcher::threadMain()
{
	io_service.run();
	g_database.disconnect();
}

void Dispatcher::addTask(std::function<void (void)> functor)
{
	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	#ifdef __cpp_generic_lambdas
	[this, f = std::move(functor)]() {
		++dispatcherCycle;

		// execute it
		(f)();
	});
	#else
		[this, functor]() {
		++dispatcherCycle;

		// execute it
		(functor)();
	});
	#endif
}

void Dispatcher::addTask(Task* task)
{
	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	[this, task]() {
		++dispatcherCycle;

		// execute it
		(*task)();
		delete task;
	});
}

void Dispatcher::shutdown()
{
	#if BOOST_VERSION >= 106600
	boost::asio::post(io_service,
	#else
	io_service.post(
	#endif
	[this]() {
		setState(THREAD_STATE_TERMINATED);
		work.reset();
	});
}
