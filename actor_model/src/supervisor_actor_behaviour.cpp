/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "supervisor_actor_behaviour.h"

#include "actor_model.h"

#include <unordered_map>

namespace ActorModel {

using string = std::string;
using string_view = std::string_view;

constexpr char TAG[] = "supervisor";

using MutableSupervisorArgsFlatbuffer = std::vector<uint8_t>;

struct SupervisorActorState
{
  SupervisorActorState()
  {
  }

  auto init_child(const string_view child_id, const Pid& self)
    -> bool
  {
    const auto* supervisor_args = flatbuffers::GetRoot<SupervisorArgs>(
      supervisor_args_mutable_buf.data()
    );

    if (
      supervisor_args
      and supervisor_args->child_specs()
      and supervisor_args->child_specs()->size() > 0
    )
    {
      for (const auto* child_spec : *(supervisor_args->child_specs()))
      {
        if (
          child_spec->id()->string_view() == child_id
          and child_spec->start()
        )
        {
          const auto* start = child_spec->start();

          if (
            start
            and start->module_name()
            and start->function_name()
            and start->args()
          )
          {
            const auto result = apply(
              self,
              start->module_name()->string_view(),
              start->function_name()->string_view(),
              string_view{
                reinterpret_cast<const char*>(start->args()->data()),
                start->args()->size()
              }
            );

            if (result.type == Result::Ok)
            {
              const auto& child_pid = result.id;
              children[child_pid] = child_spec->id()->str();
              return true;
            }
          }
        }
      }
    }

    return false;
  }

  std::unordered_map<
    Pid,
    string,
    UUID::UUIDHashFunc,
    UUID::UUIDEqualFunc
  > children;
  MutableSupervisorArgsFlatbuffer supervisor_args_mutable_buf;
};

auto supervisor_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<SupervisorActorState>();
  }
  auto& state = *(std::static_pointer_cast<SupervisorActorState>(_state));

  if (matches(message, "init", state.supervisor_args_mutable_buf))
  {
    if (not state.supervisor_args_mutable_buf.empty())
    {
      const auto* supervisor_args = flatbuffers::GetRoot<SupervisorArgs>(
        state.supervisor_args_mutable_buf.data()
      );

      if (
        supervisor_args
        and supervisor_args->child_specs()
        and supervisor_args->child_specs()->size() > 0
      )
      {
        // Trap exit from all processes linked to our Pid
        process_flag(self, ProcessFlag::trap_exit, true);

        for (const auto* child_spec : *(supervisor_args->child_specs()))
        {
          state.init_child(child_spec->id()->string_view(), self);
        }
      }
    }

    return {Result::Ok};
  }

  if (
    string_view reason;
    matches(message, "kill", reason)
  )
  {
    if (message.from_pid())
    {
      const auto& from_pid = *(message.from_pid());

      const auto& child_iter = state.children.find(from_pid);
      if (child_iter != state.children.end())
      {
        state.init_child(child_iter->second, self);
      }
    }

    return {Result::Ok};
  }

  return {Result::Unhandled};
}

} // namespace ActorModel
