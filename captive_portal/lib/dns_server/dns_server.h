#pragma once

#include "dns_types.h"

#include <lwip/inet.h>

#include <stdlib.h>
#include <stdint.h>

class DNSServer
{
public:
  DNSServer() = default;

  auto set_error_reply_code(DNSReplyCode _error_reply_code)
    -> void;
  auto set_ttl(uint32_t _ttl)
    -> void;

  auto start(
    uint16_t _port,
    uint32_t _resolved_ip
  ) -> bool;

  auto stop()
    -> void;

  auto process_next_request()
    -> bool;

protected:
  auto request_includes_only_one_question()
    -> bool;

  auto reply_with_ip(
    struct sockaddr_in* to_addr,
    int to_addr_len,
    uint32_t answer_ip
  )
    -> bool;

  auto reply_with_custom_code(
    struct sockaddr_in* to_addr,
    int to_addr_len
  )
    -> bool;

private:
  int sockfd = -1;
  uint16_t port = 53;
  uint32_t resolved_ip;
  uint32_t null_ip = 0;

  static constexpr int rx_buffer_len = 1460;
  uint8_t rx_buffer[rx_buffer_len];

  static constexpr int tx_buffer_len = 1460;
  uint8_t tx_buffer[tx_buffer_len];

  DNSHeader dns_header;
  uint32_t ttl = htonl(DNS_DEFAULT_TTL);
  DNSReplyCode error_reply_code = DNSReplyCode::NonExistentDomain;
  DNSQuestion dns_question;
};
