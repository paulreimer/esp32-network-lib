// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SHEETS_GOOGLEAPIS_SHEETS_H_
#define FLATBUFFERS_GENERATED_SHEETS_GOOGLEAPIS_SHEETS_H_

#include "flatbuffers/flatbuffers.h"

#include "uuid_generated.h"

namespace googleapis {
namespace Sheets {

struct InsertRowIntent;

inline const flatbuffers::TypeTable *InsertRowIntentTypeTable();

struct InsertRowIntent FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return InsertRowIntentTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "googleapis.Sheets.InsertRowIntent";
  }
  enum {
    VT_ID = 4,
    VT_TO_PID = 6,
    VT_SPREADSHEET_ID = 8,
    VT_SHEET_NAME = 10,
    VT_VALUES_JSON = 12
  };
  const UUID::UUID *id() const {
    return GetStruct<const UUID::UUID *>(VT_ID);
  }
  UUID::UUID *mutable_id() {
    return GetStruct<UUID::UUID *>(VT_ID);
  }
  const UUID::UUID *to_pid() const {
    return GetStruct<const UUID::UUID *>(VT_TO_PID);
  }
  UUID::UUID *mutable_to_pid() {
    return GetStruct<UUID::UUID *>(VT_TO_PID);
  }
  const flatbuffers::String *spreadsheet_id() const {
    return GetPointer<const flatbuffers::String *>(VT_SPREADSHEET_ID);
  }
  flatbuffers::String *mutable_spreadsheet_id() {
    return GetPointer<flatbuffers::String *>(VT_SPREADSHEET_ID);
  }
  const flatbuffers::String *sheet_name() const {
    return GetPointer<const flatbuffers::String *>(VT_SHEET_NAME);
  }
  flatbuffers::String *mutable_sheet_name() {
    return GetPointer<flatbuffers::String *>(VT_SHEET_NAME);
  }
  const flatbuffers::String *values_json() const {
    return GetPointer<const flatbuffers::String *>(VT_VALUES_JSON);
  }
  flatbuffers::String *mutable_values_json() {
    return GetPointer<flatbuffers::String *>(VT_VALUES_JSON);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<UUID::UUID>(verifier, VT_ID) &&
           VerifyField<UUID::UUID>(verifier, VT_TO_PID) &&
           VerifyOffsetRequired(verifier, VT_SPREADSHEET_ID) &&
           verifier.VerifyString(spreadsheet_id()) &&
           VerifyOffsetRequired(verifier, VT_SHEET_NAME) &&
           verifier.VerifyString(sheet_name()) &&
           VerifyOffsetRequired(verifier, VT_VALUES_JSON) &&
           verifier.VerifyString(values_json()) &&
           verifier.EndTable();
  }
};

struct InsertRowIntentBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(const UUID::UUID *id) {
    fbb_.AddStruct(InsertRowIntent::VT_ID, id);
  }
  void add_to_pid(const UUID::UUID *to_pid) {
    fbb_.AddStruct(InsertRowIntent::VT_TO_PID, to_pid);
  }
  void add_spreadsheet_id(flatbuffers::Offset<flatbuffers::String> spreadsheet_id) {
    fbb_.AddOffset(InsertRowIntent::VT_SPREADSHEET_ID, spreadsheet_id);
  }
  void add_sheet_name(flatbuffers::Offset<flatbuffers::String> sheet_name) {
    fbb_.AddOffset(InsertRowIntent::VT_SHEET_NAME, sheet_name);
  }
  void add_values_json(flatbuffers::Offset<flatbuffers::String> values_json) {
    fbb_.AddOffset(InsertRowIntent::VT_VALUES_JSON, values_json);
  }
  explicit InsertRowIntentBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  InsertRowIntentBuilder &operator=(const InsertRowIntentBuilder &);
  flatbuffers::Offset<InsertRowIntent> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<InsertRowIntent>(end);
    fbb_.Required(o, InsertRowIntent::VT_SPREADSHEET_ID);
    fbb_.Required(o, InsertRowIntent::VT_SHEET_NAME);
    fbb_.Required(o, InsertRowIntent::VT_VALUES_JSON);
    return o;
  }
};

inline flatbuffers::Offset<InsertRowIntent> CreateInsertRowIntent(
    flatbuffers::FlatBufferBuilder &_fbb,
    const UUID::UUID *id = 0,
    const UUID::UUID *to_pid = 0,
    flatbuffers::Offset<flatbuffers::String> spreadsheet_id = 0,
    flatbuffers::Offset<flatbuffers::String> sheet_name = 0,
    flatbuffers::Offset<flatbuffers::String> values_json = 0) {
  InsertRowIntentBuilder builder_(_fbb);
  builder_.add_values_json(values_json);
  builder_.add_sheet_name(sheet_name);
  builder_.add_spreadsheet_id(spreadsheet_id);
  builder_.add_to_pid(to_pid);
  builder_.add_id(id);
  return builder_.Finish();
}

inline flatbuffers::Offset<InsertRowIntent> CreateInsertRowIntentDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const UUID::UUID *id = 0,
    const UUID::UUID *to_pid = 0,
    const char *spreadsheet_id = nullptr,
    const char *sheet_name = nullptr,
    const char *values_json = nullptr) {
  return googleapis::Sheets::CreateInsertRowIntent(
      _fbb,
      id,
      to_pid,
      spreadsheet_id ? _fbb.CreateString(spreadsheet_id) : 0,
      sheet_name ? _fbb.CreateString(sheet_name) : 0,
      values_json ? _fbb.CreateString(values_json) : 0);
}

inline const flatbuffers::TypeTable *InsertRowIntentTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_STRING, 0, -1 },
    { flatbuffers::ET_STRING, 0, -1 },
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    UUID::UUIDTypeTable
  };
  static const char * const names[] = {
    "id",
    "to_pid",
    "spreadsheet_id",
    "sheet_name",
    "values_json"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 5, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

}  // namespace Sheets
}  // namespace googleapis

#endif  // FLATBUFFERS_GENERATED_SHEETS_GOOGLEAPIS_SHEETS_H_
