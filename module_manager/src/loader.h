/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once

#include "executable.h"

#include "sym.h"

#include "file_buffer_loader.h"
#include "string_view_buffer_loader.h"

#include "elf/elf++.hh"

namespace ModuleManager {

class Loader
{
public:
  using string_view = std::string_view;

  auto load(const string_view elf_bin)
    -> Executable;
  auto load_from_path(const string_view path)
    -> Executable;
  auto load(const elf::elf& parsed_elf)
    -> Executable;

  auto resolve(const string_view name)
    -> Executable::Address;

protected:
  auto validate(const elf::elf& parsed_elf)
    -> bool;
  auto apply_relocs(const elf::elf& parsed_elf, Executable& executable)
    -> bool;
  auto extract_symbols(const elf::elf& parsed_elf, Executable& executable)
    -> bool;

  auto _resolve(
    const string_view name,
    const struct symbol* symbols = sym_functions,
    const int symbols_count = sym_functions_count
  ) -> Executable::Address;

private:
  auto _apply_relocs_for_section(
    const elf::elf& parsed_elf,
    const elf::section& reloc_section,
    Executable& executable,
    uint8_t* reloc_segment_wip
  ) -> bool;

  auto _segment_idx_for_reloc_offset(
    const elf::elf& parsed_elf,
    size_t offset
  ) -> int;
};

} // namespace ModuleManager
