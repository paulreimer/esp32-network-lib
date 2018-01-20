/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "network.h"

// FreeRTOS event group to signal when we are connected & ready
EventGroupHandle_t network_event_group;

EventBits_t
wait_for_network(const EventBits_t bits, TickType_t ticks_to_wait)
{
  return xEventGroupWaitBits(
    network_event_group,
    bits, // BitsToWaitFor
    false, // ClearOnExit
    true, // WaitForAllBits
    ticks_to_wait // TicksToWait
    );
}

EventBits_t
set_network(const EventBits_t bits)
{
  return xEventGroupSetBits(network_event_group, bits);
}

EventBits_t
reset_network(const EventBits_t bits)
{
  return xEventGroupClearBits(network_event_group, bits);
}
