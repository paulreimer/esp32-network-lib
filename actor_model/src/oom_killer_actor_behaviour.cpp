/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "oom_killer_actor_behaviour.h"

#include "actor_model.h"

#include "trace.h"

#include <unordered_map>

#include "esp_debug_helpers.h"
#include "esp_heap_task_info.h"

namespace ActorModel {

constexpr char TAG[] = "oom_killer";

struct TaskMemoryInfo
{
  ssize_t stack_usage_bytes = 0;
  ssize_t heap_usage_bytes = 0;
  ssize_t heap_alloc_count= 0;
};

struct OomKillerActorState
{
  OomKillerActorState()
  {
  }
};

auto oom_killer_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<OomKillerActorState>();
  }
  auto& state = *(std::static_pointer_cast<OomKillerActorState>(_state));

  if (matches(message, "heap_check"))
  {
    utils::heap_check("heap_check");

    std::unordered_map<TaskHandle_t, TaskMemoryInfo/*, std::hash<TaskHandle_t>*/> tasks_mem_info;

    const auto num_tasks = uxTaskGetNumberOfTasks();

    // Check stack usage
    std::vector<TaskSnapshot_t> stack_task_totals;
    stack_task_totals.resize(num_tasks);

    UBaseType_t tcb_sz;

    uxTaskGetSnapshotAll(
      &stack_task_totals[0],
      stack_task_totals.size(),
      &tcb_sz
    );

    for (auto& total : stack_task_totals)
    {
      auto len = (
        reinterpret_cast<uint32_t>(total.pxEndOfStack)
        - reinterpret_cast<uint32_t>(total.pxTopOfStack)
      );
      tasks_mem_info[total.pxTCB].stack_usage_bytes = len;
    }

    // Check heap usage
    std::vector<heap_task_totals_t> heap_task_totals;
    heap_task_totals.resize(num_tasks);

    heap_task_info_params_t heap_task_info_params;
    // Collect overall heap totals only
    heap_task_info_params.mask[0] = 0;
    heap_task_info_params.caps[0] = 0;
    // Do not collect block-level statistics
    heap_task_info_params.blocks = nullptr;
    heap_task_info_params.max_blocks = 0;
    // Use the std::vector buffer
    heap_task_info_params.totals = &(heap_task_totals[0]);
    heap_task_info_params.max_totals = heap_task_totals.size();
    // Start with 0 prefilled totals
    size_t num_totals = 0;
    heap_task_info_params.num_totals = &num_totals;

    heap_caps_get_per_task_info(&heap_task_info_params);
    for (const auto& total : heap_task_totals)
    {
      auto* task = total.task;
      auto existing_task_mem_info = tasks_mem_info.find(task);
      // Only update stats for tasks which have existing info already
      if (existing_task_mem_info != tasks_mem_info.end())
      {
        existing_task_mem_info->second.heap_usage_bytes = total.size[0];
        existing_task_mem_info->second.heap_alloc_count = total.count[0];
      }
    }

    for (const auto& task_iter : tasks_mem_info)
    {
      const char* task_name = pcTaskGetTaskName(const_cast<TaskHandle_t>(task_iter.first));
      printf(
        "task '%s', heap_usage_bytes=%u, heap_alloc_count=%u, stack_usage_bytes=%u\n",
        task_name,
        task_iter.second.heap_usage_bytes,
        task_iter.second.heap_alloc_count,
        task_iter.second.stack_usage_bytes
      );
    }

    return {Result::Ok};
  }

  return {Result::Unhandled};
}

} // namespace ActorModel
