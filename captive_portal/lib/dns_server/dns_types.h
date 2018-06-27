#pragma once

#include <stdint.h>

// From: https://github.com/espressif/arduino-esp32/blob/master/libraries/DNSServer/src/DNSServer.h

static constexpr int DNS_QR_QUERY = 0;
static constexpr int DNS_QR_RESPONSE = 1;
static constexpr int DNS_OPCODE_QUERY = 0;
static constexpr int DNS_DEFAULT_TTL = 60;        // Default Time To Live : time interval in seconds that the resource record should be cached before being discarded
static constexpr int DNS_OFFSET_DOMAIN_NAME = 12; // Offset in bytes to reach the domain name in the DNS message
static constexpr int DNS_HEADER_SIZE = 12;

enum class DNSReplyCode
{
  NoError   = 0,
  FormError = 1,
  ServerFailure     = 2,
  NonExistentDomain = 3,
  NotImplemented    = 4,
  Refused   = 5,
  YXDomain  = 6,
  YXRRSet   = 7,
  NXRRSet   = 8
};

enum DNSType
{
  DNS_TYPE_A      = 1,  // Host Address
  DNS_TYPE_AAAA   = 28, // IPv6 Address
  DNS_TYPE_SOA    = 6,  // Start Of a zone of Authority
  DNS_TYPE_PTR    = 12, // Domain name PoinTeR
  DNS_TYPE_DNAME  = 39  // Delegation Name
};

enum DNSClass
{
  DNS_CLASS_IN = 1, // INternet
  DNS_CLASS_CH = 3  // CHaos
};

enum DNSRDLength
{
  DNS_RDLENGTH_IPV4 = 4 // 4 bytes for an IPv4 address
};

struct DNSHeader
{
  uint16_t ID;               // identification number
  union {
    struct {
      uint16_t RD : 1;      // recursion desired
      uint16_t TC : 1;      // truncated message
      uint16_t AA : 1;      // authoritive answer
      uint16_t OPCode : 4;  // message_type
      uint16_t QR : 1;      // query/response flag
      uint16_t RCode : 4;   // response code
      uint16_t Z : 3;       // its z! reserved
      uint16_t RA : 1;      // recursion available
    };
    uint16_t Flags;
  };
  uint16_t QDCount;          // number of question entries
  uint16_t ANCount;          // number of answer entries
  uint16_t NSCount;          // number of authority entries
  uint16_t ARCount;          // number of resource entries
};

struct DNSQuestion
{
  uint8_t   QName[255];
  int8_t    QNameLength;
  uint16_t  QType;
  uint16_t  QClass;
};
