/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../precompiled.h"
#pragma hdrstop

#include <zlib.h>

void CRC32_InitChecksum(unsigned long& crcvalue)
{
	crcvalue = 0;
}

void CRC32_Update(unsigned long& crcvalue, const byte data)
{
	crcvalue = crc32(crcvalue, &data , 1);
}

void CRC32_UpdateChecksum(unsigned long& crcvalue, const void* data, int length)
{
	crcvalue = crc32(crcvalue, reinterpret_cast<const Bytef*>(data), length);
}

void CRC32_FinishChecksum(unsigned long &crcvalue)
{
}

unsigned long CRC32_BlockChecksum(const void *data, int length)
{
	return crc32(0, reinterpret_cast<const Bytef*>(data), length);
}
