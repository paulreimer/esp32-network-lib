#include "dns_server.h"

#include "esp_log.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include <string.h>

static constexpr char TAG[] = "dns_server";

auto DNSServer::set_error_reply_code(DNSReplyCode _error_reply_code)
  -> void
{
  error_reply_code = _error_reply_code;
}

auto DNSServer::set_ttl(uint32_t _ttl)
  -> void
{
  ttl = htonl(_ttl);
}

auto DNSServer::start(
  uint16_t _port,
  uint32_t _resolved_ip
) -> bool
{
  bool did_start = false;

  struct sockaddr_in addr = {0};

  port = _port;

  resolved_ip = _resolved_ip;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd >= 0)
  {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;

    auto did_bind = bind(
      sockfd,
      reinterpret_cast<struct sockaddr*>(&addr),
      sizeof(addr)
    );
    if (did_bind == 0)
    {
      did_start = true;
    }
    else {
      ESP_LOGE(TAG, "udp server bind error %d", sockfd);
    }
  }
  else {
    ESP_LOGE(TAG, "udp server create error %d", sockfd);
  }

  return did_start;
}

auto DNSServer::stop()
  -> void
{
  if (sockfd >= 0)
  {
    close(sockfd);
  }
}

auto DNSServer::process_next_request()
  -> bool
{
  bool valid_query = false;

  struct sockaddr_in from_addr = {0};
  socklen_t from_addr_len = sizeof(from_addr);

  auto recv_len = recvfrom(
    sockfd,
    &rx_buffer,
    sizeof(rx_buffer),
    0,
    reinterpret_cast<struct sockaddr*>(&from_addr),
    &from_addr_len
  );

  if (recv_len >= DNS_HEADER_SIZE)
  {
    memcpy(&dns_header, &rx_buffer, DNS_HEADER_SIZE);

    if (request_includes_only_one_question())
    {
      // The QName has a variable length, maximum 255 bytes and is comprised of multiple labels.
      // Each label contains a byte to describe its length and the label itself. The list of
      // labels terminates with a zero-valued byte. In "github.com", we have two labels "github" & "com"
      // Iterate through the labels and copy them as they come into a single buffer (for simplicity's sake)
      dns_question.QNameLength = 0;

      while (rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength] != 0)
      {
        memcpy(
          (void*)&dns_question.QName[dns_question.QNameLength],
          (void*)&rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength],
          rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength] + 1
        );

        const char* cmpstr = "p-rimes";
        if (
          memcmp(
            &rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength + 1],
            &cmpstr[0],
            strlen(cmpstr)
          ) == 0
        )
        {
          printf("valid query\n");
          valid_query = true;
        }

        dns_question.QNameLength += rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength] + 1;

      }
      dns_question.QName[dns_question.QNameLength] = 0;
      dns_question.QNameLength++;

      // Copy the QType and QClass
      memcpy(
        &dns_question.QType,
        (void*)&rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength],
        sizeof(dns_question.QType)
      );
      memcpy(
        &dns_question.QClass,
        (void*)&rx_buffer[DNS_HEADER_SIZE + dns_question.QNameLength + sizeof(dns_question.QType)],
        sizeof(dns_question.QClass)
      );
    }

    if (dns_header.QR == DNS_QR_QUERY)
    {
      if (
        dns_header.OPCode == DNS_OPCODE_QUERY
        and request_includes_only_one_question()
      )
      {
        reply_with_ip(
          &from_addr,
          from_addr_len,
          valid_query? resolved_ip : null_ip
        );
      }
      else {
        reply_with_custom_code(&from_addr, from_addr_len);
      }

      return true;
    }
  }
  else {
    ESP_LOGE(TAG, "udp server recv error %d", recv_len);
  }

  return false;
}

auto DNSServer::request_includes_only_one_question()
  -> bool
{
  return (
    ntohs(dns_header.QDCount) == 1
    and dns_header.ANCount == 0
    and dns_header.NSCount == 0
    //and dns_header.ARCount == 0
  );
}

auto DNSServer::reply_with_ip(
  struct sockaddr_in* to_addr,
  int to_addr_len,
  uint32_t answer_ip
)
  -> bool
{
  auto offset = 0;
  uint8_t b = 0x00;

  // Change the type of message to a response and set the number of answers equal to
  // the number of questions in the header
  dns_header.QR      = DNS_QR_RESPONSE;
  dns_header.ANCount = dns_header.QDCount;
  memcpy(&tx_buffer[offset], (uint8_t*)&dns_header, DNS_HEADER_SIZE);
  offset += DNS_HEADER_SIZE;

  // Write the question
  memcpy(&tx_buffer[offset], dns_question.QName, dns_question.QNameLength);
  offset += dns_question.QNameLength;

  memcpy(&tx_buffer[offset], (uint8_t*)&dns_question.QType, 2);
  offset += 2;

  memcpy(&tx_buffer[offset], (uint8_t*)&dns_question.QClass, 2);
  offset += 2;

  // Write the answer
  // Use DNS name compression : instead of repeating the name in this RNAME occurence,
  // set the two MSB of the byte corresponding normally to the length to 1. The following
  // 14 bits must be used to specify the offset of the domain name in the message
  // (<255 here so the first byte has the 6 LSB at 0)
  b = 0xC0;
  memcpy(&tx_buffer[offset], (uint8_t*)&b, 1);
  offset += 1;

  b = DNS_OFFSET_DOMAIN_NAME;
  memcpy(&tx_buffer[offset], (uint8_t*)&b, 1);
  offset += 1;

  // DNS type A : host address, DNS class IN for INternet, returning an IPv4 address
  uint16_t answerType = htons(DNS_TYPE_A);
  uint16_t answerClass = htons(DNS_CLASS_IN);
  uint16_t answerIPv4 = htons(DNS_RDLENGTH_IPV4);

  memcpy(&tx_buffer[offset], (uint8_t*)&answerType, 2);
  offset += 2;

  memcpy(&tx_buffer[offset], (uint8_t*)&answerClass, 2);
  offset += 2;

  memcpy(&tx_buffer[offset], (uint8_t*)&ttl, 4);        // DNS Time To Live
  offset += 4;

  memcpy(&tx_buffer[offset], (uint8_t*)&answerIPv4, 2);
  offset += 2;

  memcpy(&tx_buffer[offset], &answer_ip, sizeof(answer_ip)); // The IP address to return
  offset += sizeof(answer_ip);

  int actual_sent = sendto(
    sockfd,
    &tx_buffer[0],
    offset,
    0,
    reinterpret_cast<struct sockaddr*>(to_addr),
    to_addr_len
  );

  if (actual_sent < 0)
  {
    ESP_LOGE(TAG, "could not reply with ip: %d", errno);
  }

  return (actual_sent >= 0);
}

auto DNSServer::reply_with_custom_code(
  struct sockaddr_in* to_addr,
  int to_addr_len
)
  -> bool
{
  dns_header.QR = DNS_QR_RESPONSE;
  dns_header.RCode = (uint8_t)error_reply_code;
  dns_header.QDCount = 0;

  auto offset = 0;

  memcpy(&tx_buffer[offset], &dns_header, sizeof(DNSHeader));
  offset += sizeof(DNSHeader);

  int actual_sent = sendto(
    sockfd,
    &tx_buffer[0],
    offset,
    0,
    reinterpret_cast<struct sockaddr*>(to_addr),
    to_addr_len
  );

  if (actual_sent < 0)
  {
    ESP_LOGE(TAG, "could not reply with custom code: %d", actual_sent);
  }

  return (actual_sent >= 0);
}
