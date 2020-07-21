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

#ifndef FS_RSA_H_C4E277DA8E884B578DDBF0566F504E91
#define FS_RSA_H_C4E277DA8E884B578DDBF0566F504E91

#include <gmp.h>

class RSA
{
	public:
		RSA();
		~RSA();

		// Singleton - ensures we don't accidentally copy it
		RSA(const RSA&) = delete;
		RSA& operator=(const RSA&) = delete;

		static RSA& getInstance() {
			static RSA instance; // Guaranteed to be destroyed.
														// Instantiated on first use.
			return instance;
		}

		void queryNanD(const char* pString, const char* qString);
		void setKey(const char* nString, const char* dString);
		void decrypt(char* msg) const;

	private:
		mpz_t n, d;
};

constexpr auto g_RSA = &RSA::getInstance;

#endif
