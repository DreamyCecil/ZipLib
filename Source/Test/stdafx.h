// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include "ZipTools/IZipTool.h"
#include "ZipTools/SevenZipTool.h"

#include "Utils.h"

#include "../ZipLib/ZipFile.h"
#include "../ZipLib/methods/ZipMethodResolver.h"
#include "../ZipLib/methods/compression/StoreMethod.h"
#include "../ZipLib/methods/compression/DeflateMethod.h"
#include "../ZipLib/methods/compression/Bzip2Method.h"
#include "../ZipLib/methods/compression/LzmaMethod.h"
#include "../ZipLib/methods/encryption/ZipCryptoMethod.h"
#include "../ZipLib/methods/encryption/AesCryptoMethod.h"
#include "../ZipLib/streams/crc32stream.h"
#include "../ZipLib/streams/nullstream.h"
#include "../ZipLib/utils/stream/copy.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <regex>
#include <cstdint>
#include <cstdio>

#include <windows.h>
