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

#include "elf/data.hh"

namespace ModuleManager {
namespace XtensaElf32 {

enum RelocType {
  R_XTENSA_NONE = 0,
  R_XTENSA_32 = 1,
  R_XTENSA_RTLD = 2,
  R_XTENSA_GLOB_DAT = 3,
  R_XTENSA_JMP_SLOT = 4,
  R_XTENSA_RELATIVE = 5,
  R_XTENSA_PLT = 6,
  R_XTENSA_OP0 = 8,
  R_XTENSA_OP1 = 9,
  R_XTENSA_OP2 = 10,
  R_XTENSA_ASM_EXPAND = 11,
  R_XTENSA_ASM_SIMPLIFY = 12,
  R_XTENSA_32_PCREL = 14,
  R_XTENSA_GNU_VTINHERIT = 15,
  R_XTENSA_GNU_VTENTRY = 16,
  R_XTENSA_DIFF8 = 17,
  R_XTENSA_DIFF16 = 18,
  R_XTENSA_DIFF32 = 19,
  R_XTENSA_SLOT0_OP = 20,
  R_XTENSA_SLOT1_OP = 21,
  R_XTENSA_SLOT2_OP = 22,
  R_XTENSA_SLOT3_OP = 23,
  R_XTENSA_SLOT4_OP = 24,
  R_XTENSA_SLOT5_OP = 25,
  R_XTENSA_SLOT6_OP = 26,
  R_XTENSA_SLOT7_OP = 27,
  R_XTENSA_SLOT8_OP = 28,
  R_XTENSA_SLOT9_OP = 29,
  R_XTENSA_SLOT10_OP = 30,
  R_XTENSA_SLOT11_OP = 31,
  R_XTENSA_SLOT12_OP = 32,
  R_XTENSA_SLOT13_OP = 33,
  R_XTENSA_SLOT14_OP = 34,
  R_XTENSA_SLOT0_ALT = 35,
  R_XTENSA_SLOT1_ALT = 36,
  R_XTENSA_SLOT2_ALT = 37,
  R_XTENSA_SLOT3_ALT = 38,
  R_XTENSA_SLOT4_ALT = 39,
  R_XTENSA_SLOT5_ALT = 40,
  R_XTENSA_SLOT6_ALT = 41,
  R_XTENSA_SLOT7_ALT = 42,
  R_XTENSA_SLOT8_ALT = 43,
  R_XTENSA_SLOT9_ALT = 44,
  R_XTENSA_SLOT10_ALT = 45,
  R_XTENSA_SLOT11_ALT = 46,
  R_XTENSA_SLOT12_ALT = 47,
  R_XTENSA_SLOT13_ALT = 48,
  R_XTENSA_SLOT14_ALT = 49,
  R_XTENSA_TLSDESC_FN = 50,
  R_XTENSA_TLSDESC_ARG = 51,
  R_XTENSA_TLS_DTPOFF = 52,
  R_XTENSA_TLS_TPOFF = 53,
  R_XTENSA_TLS_FUNC = 54,
  R_XTENSA_TLS_ARG = 55,
  R_XTENSA_TLS_CALL = 56,

  R_XTENSA_max
};

typedef struct __attribute__((__packed__)) RelInfo {
  RelocType reloc_type : 8;
  elf::Elf32::Word symtab_index : 24;
} RelInfo;

typedef struct __attribute__((__packed__)) Rel
{
  elf::Elf32::Addr offset;
  RelInfo info;
} Rel;

typedef struct __attribute__((__packed__)) Rela
{
  elf::Elf32::Addr offset;
  RelInfo info;
  elf::Elf32::Sword addend;
} Rela;

typedef struct __attribute__((__packed__)) Dyn
{
  elf::Elf32::Sword tag;
  union {
    elf::Elf32::Word val;
    elf::Elf32::Addr ptr;
    elf::Elf32::Off off;
  } data;
} Dyn;

} // namespace XtensaElf32
} // namespace ModuleManager
