/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "loader.h"
#include "delay.h"
#include "request_manager_actor.h"

#include "xtensa_elf.h"

#include "gsl/span"

#include "esp_heap_caps.h"
#include "esp_log.h"

#include <chrono>

namespace ModuleManager {

using namespace std::chrono_literals;

using string_view = std::string_view;
using XtensaElf32::RelocType;

constexpr char TAG[] = "Loader";

auto Loader::load(const string_view elf_bin)
  -> Executable
{
  auto loader = std::make_shared<StringViewBufferLoader>(elf_bin);
  elf::elf parsed_elf(loader);

  return load(parsed_elf);
}

auto Loader::load_from_path(const string_view path)
  -> Executable
{
  Executable executable;

  auto loader = std::make_shared<FileBufferLoader>(path);
  elf::elf parsed_elf(loader);

  if (validate(parsed_elf))
  {
    auto idx = 0;
    for (const auto &seg : parsed_elf.segments())
    {
      // Load segment data from file on first time accessing data
      seg.data();

      auto &hdr = seg.get_hdr();
      auto* segment_data = loader->extract(hdr.offset, hdr.memsz);
      if (segment_data != nullptr)
      {
        executable.segments.emplace_back(
          std::make_pair(segment_data, hdr.flags)
        );
        if (
          static_cast<std::underlying_type_t<elf::pf>>(hdr.flags & elf::pf::x)
        )
        {
          executable.executable_segment_idx = idx;
          executable.executable_segment_size = hdr.memsz;

          // Get 32-bit aligned value for IRAM memory
          const auto align = sizeof(size_t);
          const auto rem = hdr.memsz % align;
          executable.executable_segment_aligned_size = (
            rem
            ? (hdr.memsz + (align - rem))
            : (hdr.memsz)
          );
        }
      }
      else {
        ESP_LOGE(
          TAG,
          "Could not find segment %llu, %llu",
          hdr.offset,
          hdr.memsz
        );
      }

      idx++;
    }

    auto did_link = apply_relocs(parsed_elf, executable);
    if (did_link)
    {
      extract_symbols(parsed_elf, executable);
    }
    else {
      executable.clear();
    }
  }

  return executable;
}

auto Loader::load(const elf::elf& parsed_elf)
  -> Executable
{
  Executable executable;

  if (validate(parsed_elf))
  {
    auto& text = parsed_elf.get_section(".text");
    if (text.size() > 0)
    {
      apply_relocs(parsed_elf, executable);
      extract_symbols(parsed_elf, executable);
    }
  }

  return executable;
}

auto Loader::validate(const elf::elf& parsed_elf)
  -> bool
{
  // Require ALL of these
  auto& dynsym   = parsed_elf.get_section(".dynsym");
  auto& dynstr   = parsed_elf.get_section(".dynstr");

  // Require ALL of these
  auto& rela_dyn = parsed_elf.get_section(".rela.dyn");
  auto& rela_plt = parsed_elf.get_section(".rela.plt");

  // Require ALL of these
  auto& text = parsed_elf.get_section(".text");

  // Validate all sections needed for dynamic linking are present
  return (
    (
      (dynsym.valid()       and dynsym.get_hdr().type   == elf::sht::dynsym)
      and (dynstr.valid()   and dynstr.get_hdr().type   == elf::sht::strtab)
    )
    and (
      (rela_dyn.valid()     and rela_dyn.get_hdr().type == elf::sht::rela)
      and (rela_plt.valid() and rela_plt.get_hdr().type == elf::sht::rela)
    )
    and (
      (text.valid()         and text.get_hdr().type     == elf::sht::progbits)
    )
  );
}

auto Loader::apply_relocs(const elf::elf& parsed_elf, Executable& executable)
  -> bool
{
  bool did_apply_all_relocs = true;

  auto valid = validate(parsed_elf);

  if (valid and executable.executable_segment_idx > -1)
  {
    auto& rela_dyn = parsed_elf.get_section(".rela.dyn");
    auto& rela_plt = parsed_elf.get_section(".rela.plt");

    auto& text = parsed_elf.get_section(".text");

    const auto& executable_segment = parsed_elf.get_segment(
      executable.executable_segment_idx
    );
    auto* reloc_segment_wip = reinterpret_cast<uint8_t*>(
      const_cast<void*>(executable_segment.data())
    );

    executable.loaded_executable_segment_addr = static_cast<uint8_t*>(
      heap_caps_malloc(
        executable.executable_segment_aligned_size,
        MALLOC_CAP_EXEC
      )
    );

    if (did_apply_all_relocs)
    {
      // Relocate rela.dyn section (objects)
      did_apply_all_relocs = _apply_relocs_for_section(
        parsed_elf,
        rela_dyn,
        executable,
        reloc_segment_wip
      );
    }

    if (did_apply_all_relocs)
    {
      // Relocate rela.plt section (functions)
      did_apply_all_relocs = _apply_relocs_for_section(
        parsed_elf,
        rela_plt,
        executable,
        reloc_segment_wip
      );
    }

    if (did_apply_all_relocs)
    {
      // Copy the relocated buffer to executable memory
      memcpy(
        (void*)executable.loaded_executable_segment_addr,
        reloc_segment_wip,
        executable.executable_segment_aligned_size
      );
    }
  }
  else {
    ESP_LOGE(TAG, "Invalid ELF binary\n");
  }

  return (valid and did_apply_all_relocs);
}

auto Loader::_segment_idx_for_reloc_offset(
  const elf::elf& parsed_elf,
  size_t offset
) -> int
{
  int idx = -1;
  for (const auto &seg : parsed_elf.segments())
  {
    ++idx;
    auto &hdr = seg.get_hdr();
    const auto begin = hdr.vaddr;
    const auto end = hdr.vaddr + hdr.memsz;

    if (begin <= offset && offset < end)
    {
      break;
    }
  }

  return idx;
}

auto Loader::_apply_relocs_for_section(
  const elf::elf& parsed_elf,
  const elf::section& reloc_section,
  Executable& executable,
  uint8_t* __reloc_segment_wip
) -> bool
{
  bool did_apply_all_relocs = true;
  auto& dynsym = parsed_elf.get_section(".dynsym");

  // List of relocations
  const auto &reloc_hdr = reloc_section.get_hdr();
  auto reloc_count = (reloc_section.size() / reloc_hdr.entsize);
  gsl::span<XtensaElf32::Rela> relocs(
    const_cast<XtensaElf32::Rela*>(
      static_cast<const XtensaElf32::Rela*>(reloc_section.data())
    ),
    reloc_count
  );

  // Iterate and apply each relocation
  for (const auto& reloc : relocs)
  {
    const auto segment_idx = _segment_idx_for_reloc_offset(
      parsed_elf,
      reloc.offset
    );
    const auto& reloc_segment = parsed_elf.get_segment(segment_idx);
    const auto reloc_segment_base_offset = reloc_segment.get_hdr().paddr;
    auto* reloc_segment_wip = const_cast<void*>(reloc_segment.data());

    switch (reloc.info.reloc_type)
    {
      case XtensaElf32::R_XTENSA_RELATIVE:
      {
        // Extract current contents from to-be-reloc'd offset
        const uint32_t adjusted_offset = (
          reloc.offset
          - reloc_segment_base_offset
        );

        // Read contents of memory at relocation site
        uint32_t S;
        memcpy((void*)&S, reloc_segment_wip + adjusted_offset, 4);

        // Determine the correct relocation data to write
        uint32_t addr = (
          reinterpret_cast<uint32_t>(reloc_segment.data())
          + S
          - reloc_segment_base_offset
        );

        // Apply the reloc
        memcpy(reloc_segment_wip + adjusted_offset, &addr, 4);
        break;
      }

      case XtensaElf32::R_XTENSA_JMP_SLOT:
      case XtensaElf32::R_XTENSA_GLOB_DAT:
      {
        const auto& symtab = dynsym.as_symtab();
        auto symbol_index = reloc.info.symtab_index;

        auto i = 0;
        for (const auto& sym : symtab)
        {
          if (i++ == symbol_index)
          {
            const auto addr = resolve(sym.get_name());
            if (addr)
            {
              const auto segment_idx = _segment_idx_for_reloc_offset(
                parsed_elf,
                reloc.offset
              );
              const auto& reloc_segment = parsed_elf.get_segment(segment_idx);

              const auto adjusted_offset = (
                reloc.offset
                - reloc_segment_base_offset
              );

              // Read contents of memory at relocation site
              uint32_t S;
              memcpy((void*)&S, reloc_segment_wip + adjusted_offset, 4);

              memcpy(reloc_segment_wip + adjusted_offset, &addr, 4);
            }
            else {
              did_apply_all_relocs = false;
              ESP_LOGE(TAG, "Failed to resolve %s\n", sym.get_name().c_str());
            }
            break;
          }
        }

        break;
      }

      case XtensaElf32::R_XTENSA_RTLD:
        // Ignore dynamic link relocs, as we are already dynamic loading
        break;

      default:
        ESP_LOGW(TAG, "Unknown reloc type: %d>\n", reloc.info.reloc_type);
        break;
    }
  }

  return did_apply_all_relocs;
}

auto Loader::extract_symbols(const elf::elf& parsed_elf, Executable& executable)
  -> bool
{
  auto& dynsym = parsed_elf.get_section(".dynsym");

  auto valid = (
    dynsym.valid() and dynsym.get_hdr().type == elf::sht::dynsym
    and executable.executable_segment_idx > -1
  );

  if (valid)
  {
    auto& executable_segment = parsed_elf.get_segment(
      executable.executable_segment_idx
    );
    const auto reloc_segment_base_offset = executable_segment.get_hdr().paddr;

    const auto& symtab = dynsym.as_symtab();
    for (const auto& sym : symtab)
    {
      switch (sym.get_data().type())
      {
        case elf::stt::func:
        {
          auto func_addr = sym.get_data().value - reloc_segment_base_offset;
          executable.symbols[sym.get_name()] = func_addr;
          break;
        }

        case elf::stt::object:
        {
          auto object_addr = sym.get_data().value - reloc_segment_base_offset;
          executable.symbols[sym.get_name()] = object_addr;
          break;
        }

        case elf::stt::notype:
          break;

        default:
          ESP_LOGW(
            TAG,
            "Unknown elf::stt type %d",
            static_cast<std::underlying_type_t<elf::stt>>(sym.get_data().type())
          );
          break;
      }
    }
  }

  return valid;
}

auto Loader::resolve(const string_view name)
  -> Executable::Address
{
  Executable::Address addr = _resolve(name, sym_functions, sym_functions_count);
  if (addr == 0)
  {
    addr = _resolve(name, sym_objects, sym_objects_count);
  }

  return addr;
}

auto Loader::_resolve(
  const string_view name,
  const struct symbol* symbols,
  const int symbols_count
) -> Executable::Address
{
  int start, middle, end;
  int r;

  start = 0;
  end = symbols_count - 1;

  // Binary search for name
  while (start <= end)
  {
    // Check middle, split in half, and choose correct side
    middle = (start + end) / 2;
    r = strncmp(name.data(), symbols[middle].name, name.size());
    if (r < 0)
    {
      end = middle - 1;
    }
    else if (r > 0)
    {
      start = middle + 1;
    }
    else {
      return reinterpret_cast<Executable::Address>(symbols[middle].u.obj);
    }
  }

  // Could not resolve
  return 0;
}

} // namespace ModuleManager
