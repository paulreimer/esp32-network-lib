#pragma once

#define htonll(x) ((((uint64_t)htonl(x) & 0xFFFFFFFF) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x) & 0xFFFFFFFF) << 32) + ntohl((x) >> 32))
