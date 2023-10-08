/*
 *   Copyright (C) 2021 GaryOderNichts
 *   Copyright (C) 2022 Pretendo Network contributors
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <mocha/mocha.h>

#include <vpad/input.h>

#include "patches.h"
#include "log.h"

#include "checkInkayStatus.hpp"

//thanks @Gary#4139 :p
static void write_string(uint32_t addr, const char* str)
{
    int len = strlen(str) + 1;
    int remaining = len % 4;
    int num = len - remaining;

    for (int i = 0; i < (num / 4); i++) {
        Mocha_IOSUKernelWrite32(addr + i * 4, *(uint32_t*)(str + i * 4));
    }

    if (remaining > 0) {
        uint8_t buf[4];
        Mocha_IOSUKernelRead32(addr + num, (uint32_t*)&buf);

        for (int i = 0; i < remaining; i++) {
            buf[i] = *(str + num + i);
        }

        Mocha_IOSUKernelWrite32(addr + num, *(uint32_t*)&buf);
    }
}

int main(int argc, char** argv)
{
	int mochaResult;
	if ((mochaResult = Mocha_InitLibrary()) != MOCHA_RESULT_SUCCESS) {
        log(std::format("Mocha_InitLibrary() failed with code {}", mochaResult).c_str());
    }
	
#ifdef DEBUG
    WHBLogUdpInit();
#endif

    log("Hello world from Nimble!");

    //check Gamepad input to maybe skip patches
    VPADStatus status;
    VPADReadError error = VPAD_READ_SUCCESS;
    VPADRead(VPAD_CHAN_0, &status, 1, &error);

	if (mochaResult != MOCHA_RESULT_SUCCESS) {
		log("Nimble patches failed!");
	}
    else if (skipPatches())
    {
        log("Nimble patches skipped.");
    }
    else
    {
		/* URL patch */
		for (int i = 0; i < sizeof(url_patches) / sizeof(URL_Patch); i++) {
			write_string(url_patches[i].address, url_patches[i].url);
		}

        log("Nimble patches completed!");
    }
	
    if (mochaResult == MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
    }

#ifdef DEBUG
    WHBLogUdpDeinit();
#endif

    return 0;
}
