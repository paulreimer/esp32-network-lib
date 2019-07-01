// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_UUID_UUID_H_
#define FLATBUFFERS_GENERATED_UUID_UUID_H_

#include "flatbuffers/flatbuffers.h"

namespace UUID {

struct UUID;

inline const flatbuffers::TypeTable *UUIDTypeTable();

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) UUID FLATBUFFERS_FINAL_CLASS {
 private:
  uint64_t ab_;
  uint64_t cd_;

 public:
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return UUIDTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "UUID.UUID";
  }
  UUID() {
    memset(static_cast<void *>(this), 0, sizeof(UUID));
  }
  UUID(uint64_t _ab, uint64_t _cd)
      : ab_(flatbuffers::EndianScalar(_ab)),
        cd_(flatbuffers::EndianScalar(_cd)) {
  }
  uint64_t ab() const {
    return flatbuffers::EndianScalar(ab_);
  }
  void mutate_ab(uint64_t _ab) {
    flatbuffers::WriteScalar(&ab_, _ab);
  }
  uint64_t cd() const {
    return flatbuffers::EndianScalar(cd_);
  }
  void mutate_cd(uint64_t _cd) {
    flatbuffers::WriteScalar(&cd_, _cd);
  }
};
FLATBUFFERS_STRUCT_END(UUID, 16);

inline const flatbuffers::TypeTable *UUIDTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_ULONG, 0, -1 },
    { flatbuffers::ET_ULONG, 0, -1 }
  };
  static const int64_t values[] = { 0, 8, 16 };
  static const char * const names[] = {
    "ab",
    "cd"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_STRUCT, 2, type_codes, nullptr, values, names
  };
  return &tt;
}

}  // namespace UUID

#endif  // FLATBUFFERS_GENERATED_UUID_UUID_H_
