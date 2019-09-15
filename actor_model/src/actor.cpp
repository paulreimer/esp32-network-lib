/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "actor.h"

#include "delay.h"

#include "esp_log.h"

namespace ActorModel {

// Helper factory function
auto _actor_spawn(
  const ActorBehaviours&& _actor_behaviours,
  const MaybePid& _initial_link_pid = std::nullopt,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

// Single actor behaviour convenience function
auto spawn(
  const ActorBehaviour&& _actor_behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _actor_spawn(
    {std::move(_actor_behaviour)},
    std::nullopt,
    std::move(_exec_config_callback)
  );
}

auto spawn_link(
  const Pid& _initial_link_pid,
  const ActorBehaviour&& _actor_behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _actor_spawn(
    {std::move(_actor_behaviour)},
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

// Multiple chained actor behaviours
auto spawn(
  const ActorBehaviours&& _actor_behaviours,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _actor_spawn(
    std::move(_actor_behaviours),
    std::nullopt,
    std::move(_exec_config_callback)
  );
}

auto spawn_link(
  const Pid& _initial_link_pid,
  const ActorBehaviours&& _actor_behaviours,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _actor_spawn(
    std::move(_actor_behaviours),
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

auto _actor_spawn(
  const ActorBehaviours&& _actor_behaviours,
  const MaybePid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto&& behaviour = (
    [actor_behaviours{std::move(_actor_behaviours)}]
    (const Pid& pid, Mailbox& mailbox)
      -> ResultUnion
    {
      std::vector<StatePtr> state_ptrs(actor_behaviours.size(), nullptr);

      ResultUnion result;

      // Run forever until error
      while (result.type != Result::Error)
      {
        // Wait for and obtain a reference to a message
        const Mailbox::ReceivedMessagePtr& received_message{mailbox.receive()};

        if (received_message)
        {
          const auto& _message = received_message->ref();
          const auto* message = flatbuffers::GetRoot<Message>(_message.data());

          auto idx = 0;
          for (const auto& actor_behaviour : actor_behaviours)
          {
            auto& state = state_ptrs[idx++];

            result = actor_behaviour(pid, state, *(message));

            if (result.type == Result::Error)
            {
              // Exit behaviours loop immediately
              break;
            }

            // Exit behaviours loop immediately
            // Unless behaviour did not handle this message
            if (
              not (
                result.type == Result::Unhandled
                or result.action == EventTerminationAction::ContinueProcessing
              )
            )
            {
              break;
            }
          }
        }
      }

      return result;
    }
  );

  auto& node = Process::get_default_node();
  if (_initial_link_pid)
  {
    return node.spawn_link(
      *(_initial_link_pid),
      behaviour,
      std::move(_exec_config_callback)
    );
  }
  else {
    return node.spawn(
      behaviour,
      std::move(_exec_config_callback)
    );
  }
}

} // namespace ActorModel
