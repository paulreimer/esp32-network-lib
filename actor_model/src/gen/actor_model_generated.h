// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_ACTORMODEL_ACTORMODEL_H_
#define FLATBUFFERS_GENERATED_ACTORMODEL_ACTORMODEL_H_

#include "flatbuffers/flatbuffers.h"

#include "uuid_generated.h"

namespace ActorModel {

struct Message;

struct Ok;

struct Error;

struct Signal;

struct SupervisorFlags;

struct ActorExecutionConfig;

inline const flatbuffers::TypeTable *MessageTypeTable();

inline const flatbuffers::TypeTable *OkTypeTable();

inline const flatbuffers::TypeTable *ErrorTypeTable();

inline const flatbuffers::TypeTable *SignalTypeTable();

inline const flatbuffers::TypeTable *SupervisorFlagsTypeTable();

inline const flatbuffers::TypeTable *ActorExecutionConfigTypeTable();

enum class SupervisionStrategy : int8_t {
  one_for_one = 0,
  one_for_all = 1,
  rest_for_one = 2,
  simple_one_for_one = 3,
  MIN = one_for_one,
  MAX = simple_one_for_one
};

inline const SupervisionStrategy (&EnumValuesSupervisionStrategy())[4] {
  static const SupervisionStrategy values[] = {
    SupervisionStrategy::one_for_one,
    SupervisionStrategy::one_for_all,
    SupervisionStrategy::rest_for_one,
    SupervisionStrategy::simple_one_for_one
  };
  return values;
}

inline const char * const *EnumNamesSupervisionStrategy() {
  static const char * const names[] = {
    "one_for_one",
    "one_for_all",
    "rest_for_one",
    "simple_one_for_one",
    nullptr
  };
  return names;
}

inline const char *EnumNameSupervisionStrategy(SupervisionStrategy e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesSupervisionStrategy()[index];
}

enum class ProcessFlag : int16_t {
  trap_exit = 0,
  MIN = trap_exit,
  MAX = trap_exit
};

inline const ProcessFlag (&EnumValuesProcessFlag())[1] {
  static const ProcessFlag values[] = {
    ProcessFlag::trap_exit
  };
  return values;
}

inline const char * const *EnumNamesProcessFlag() {
  static const char * const names[] = {
    "trap_exit",
    nullptr
  };
  return names;
}

inline const char *EnumNameProcessFlag(ProcessFlag e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesProcessFlag()[index];
}

enum class Result : uint8_t {
  NONE = 0,
  Ok = 1,
  Error = 2,
  MIN = NONE,
  MAX = Error
};

inline const Result (&EnumValuesResult())[3] {
  static const Result values[] = {
    Result::NONE,
    Result::Ok,
    Result::Error
  };
  return values;
}

inline const char * const *EnumNamesResult() {
  static const char * const names[] = {
    "NONE",
    "Ok",
    "Error",
    nullptr
  };
  return names;
}

inline const char *EnumNameResult(Result e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesResult()[index];
}

template<typename T> struct ResultTraits {
  static const Result enum_value = Result::NONE;
};

template<> struct ResultTraits<Ok> {
  static const Result enum_value = Result::Ok;
};

template<> struct ResultTraits<Error> {
  static const Result enum_value = Result::Error;
};

bool VerifyResult(flatbuffers::Verifier &verifier, const void *obj, Result type);
bool VerifyResultVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct Message FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return MessageTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.Message";
  }
  enum {
    VT_TYPE = 4,
    VT_TIMESTAMP = 6,
    VT_FROM_PID = 8,
    VT_PAYLOAD_ALIGNMENT = 10,
    VT_PAYLOAD = 12
  };
  const flatbuffers::String *type() const {
    return GetPointer<const flatbuffers::String *>(VT_TYPE);
  }
  flatbuffers::String *mutable_type() {
    return GetPointer<flatbuffers::String *>(VT_TYPE);
  }
  uint64_t timestamp() const {
    return GetField<uint64_t>(VT_TIMESTAMP, 0);
  }
  bool mutate_timestamp(uint64_t _timestamp) {
    return SetField<uint64_t>(VT_TIMESTAMP, _timestamp, 0);
  }
  const UUID *from_pid() const {
    return GetStruct<const UUID *>(VT_FROM_PID);
  }
  UUID *mutable_from_pid() {
    return GetStruct<UUID *>(VT_FROM_PID);
  }
  uint32_t payload_alignment() const {
    return GetField<uint32_t>(VT_PAYLOAD_ALIGNMENT, 0);
  }
  bool mutate_payload_alignment(uint32_t _payload_alignment) {
    return SetField<uint32_t>(VT_PAYLOAD_ALIGNMENT, _payload_alignment, 0);
  }
  const flatbuffers::Vector<uint8_t> *payload() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_PAYLOAD);
  }
  flatbuffers::Vector<uint8_t> *mutable_payload() {
    return GetPointer<flatbuffers::Vector<uint8_t> *>(VT_PAYLOAD);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_TYPE) &&
           verifier.Verify(type()) &&
           VerifyField<uint64_t>(verifier, VT_TIMESTAMP) &&
           VerifyField<UUID>(verifier, VT_FROM_PID) &&
           VerifyField<uint32_t>(verifier, VT_PAYLOAD_ALIGNMENT) &&
           VerifyOffset(verifier, VT_PAYLOAD) &&
           verifier.Verify(payload()) &&
           verifier.EndTable();
  }
};

struct MessageBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(flatbuffers::Offset<flatbuffers::String> type) {
    fbb_.AddOffset(Message::VT_TYPE, type);
  }
  void add_timestamp(uint64_t timestamp) {
    fbb_.AddElement<uint64_t>(Message::VT_TIMESTAMP, timestamp, 0);
  }
  void add_from_pid(const UUID *from_pid) {
    fbb_.AddStruct(Message::VT_FROM_PID, from_pid);
  }
  void add_payload_alignment(uint32_t payload_alignment) {
    fbb_.AddElement<uint32_t>(Message::VT_PAYLOAD_ALIGNMENT, payload_alignment, 0);
  }
  void add_payload(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> payload) {
    fbb_.AddOffset(Message::VT_PAYLOAD, payload);
  }
  explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  MessageBuilder &operator=(const MessageBuilder &);
  flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline flatbuffers::Offset<Message> CreateMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> type = 0,
    uint64_t timestamp = 0,
    const UUID *from_pid = 0,
    uint32_t payload_alignment = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> payload = 0) {
  MessageBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_payload(payload);
  builder_.add_payload_alignment(payload_alignment);
  builder_.add_from_pid(from_pid);
  builder_.add_type(type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Message> CreateMessageDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *type = nullptr,
    uint64_t timestamp = 0,
    const UUID *from_pid = 0,
    uint32_t payload_alignment = 0,
    const std::vector<uint8_t> *payload = nullptr) {
  return ActorModel::CreateMessage(
      _fbb,
      type ? _fbb.CreateString(type) : 0,
      timestamp,
      from_pid,
      payload_alignment,
      payload ? _fbb.CreateVector<uint8_t>(*payload) : 0);
}

struct Ok FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return OkTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.Ok";
  }
  enum {
    VT__ = 4
  };
  const flatbuffers::String *_() const {
    return GetPointer<const flatbuffers::String *>(VT__);
  }
  flatbuffers::String *mutable__() {
    return GetPointer<flatbuffers::String *>(VT__);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT__) &&
           verifier.Verify(_()) &&
           verifier.EndTable();
  }
};

struct OkBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add__(flatbuffers::Offset<flatbuffers::String> _) {
    fbb_.AddOffset(Ok::VT__, _);
  }
  explicit OkBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  OkBuilder &operator=(const OkBuilder &);
  flatbuffers::Offset<Ok> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Ok>(end);
    return o;
  }
};

inline flatbuffers::Offset<Ok> CreateOk(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> _ = 0) {
  OkBuilder builder_(_fbb);
  builder_.add__(_);
  return builder_.Finish();
}

inline flatbuffers::Offset<Ok> CreateOkDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *_ = nullptr) {
  return ActorModel::CreateOk(
      _fbb,
      _ ? _fbb.CreateString(_) : 0);
}

struct Error FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ErrorTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.Error";
  }
  enum {
    VT_REASON = 4
  };
  const flatbuffers::String *reason() const {
    return GetPointer<const flatbuffers::String *>(VT_REASON);
  }
  flatbuffers::String *mutable_reason() {
    return GetPointer<flatbuffers::String *>(VT_REASON);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_REASON) &&
           verifier.Verify(reason()) &&
           verifier.EndTable();
  }
};

struct ErrorBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_reason(flatbuffers::Offset<flatbuffers::String> reason) {
    fbb_.AddOffset(Error::VT_REASON, reason);
  }
  explicit ErrorBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ErrorBuilder &operator=(const ErrorBuilder &);
  flatbuffers::Offset<Error> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Error>(end);
    return o;
  }
};

inline flatbuffers::Offset<Error> CreateError(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> reason = 0) {
  ErrorBuilder builder_(_fbb);
  builder_.add_reason(reason);
  return builder_.Finish();
}

inline flatbuffers::Offset<Error> CreateErrorDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *reason = nullptr) {
  return ActorModel::CreateError(
      _fbb,
      reason ? _fbb.CreateString(reason) : 0);
}

struct Signal FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return SignalTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.Signal";
  }
  enum {
    VT_FROM_PID = 4,
    VT_REASON = 6
  };
  const UUID *from_pid() const {
    return GetStruct<const UUID *>(VT_FROM_PID);
  }
  UUID *mutable_from_pid() {
    return GetStruct<UUID *>(VT_FROM_PID);
  }
  const flatbuffers::String *reason() const {
    return GetPointer<const flatbuffers::String *>(VT_REASON);
  }
  flatbuffers::String *mutable_reason() {
    return GetPointer<flatbuffers::String *>(VT_REASON);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<UUID>(verifier, VT_FROM_PID) &&
           VerifyOffset(verifier, VT_REASON) &&
           verifier.Verify(reason()) &&
           verifier.EndTable();
  }
};

struct SignalBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_from_pid(const UUID *from_pid) {
    fbb_.AddStruct(Signal::VT_FROM_PID, from_pid);
  }
  void add_reason(flatbuffers::Offset<flatbuffers::String> reason) {
    fbb_.AddOffset(Signal::VT_REASON, reason);
  }
  explicit SignalBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SignalBuilder &operator=(const SignalBuilder &);
  flatbuffers::Offset<Signal> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Signal>(end);
    return o;
  }
};

inline flatbuffers::Offset<Signal> CreateSignal(
    flatbuffers::FlatBufferBuilder &_fbb,
    const UUID *from_pid = 0,
    flatbuffers::Offset<flatbuffers::String> reason = 0) {
  SignalBuilder builder_(_fbb);
  builder_.add_reason(reason);
  builder_.add_from_pid(from_pid);
  return builder_.Finish();
}

inline flatbuffers::Offset<Signal> CreateSignalDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const UUID *from_pid = 0,
    const char *reason = nullptr) {
  return ActorModel::CreateSignal(
      _fbb,
      from_pid,
      reason ? _fbb.CreateString(reason) : 0);
}

struct SupervisorFlags FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return SupervisorFlagsTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.SupervisorFlags";
  }
  enum {
    VT_STRATEGY = 4,
    VT_INTENSITY = 6,
    VT_PERIOD = 8
  };
  SupervisionStrategy strategy() const {
    return static_cast<SupervisionStrategy>(GetField<int8_t>(VT_STRATEGY, 0));
  }
  bool mutate_strategy(SupervisionStrategy _strategy) {
    return SetField<int8_t>(VT_STRATEGY, static_cast<int8_t>(_strategy), 0);
  }
  uint32_t intensity() const {
    return GetField<uint32_t>(VT_INTENSITY, 0);
  }
  bool mutate_intensity(uint32_t _intensity) {
    return SetField<uint32_t>(VT_INTENSITY, _intensity, 0);
  }
  uint32_t period() const {
    return GetField<uint32_t>(VT_PERIOD, 0);
  }
  bool mutate_period(uint32_t _period) {
    return SetField<uint32_t>(VT_PERIOD, _period, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_STRATEGY) &&
           VerifyField<uint32_t>(verifier, VT_INTENSITY) &&
           VerifyField<uint32_t>(verifier, VT_PERIOD) &&
           verifier.EndTable();
  }
};

struct SupervisorFlagsBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_strategy(SupervisionStrategy strategy) {
    fbb_.AddElement<int8_t>(SupervisorFlags::VT_STRATEGY, static_cast<int8_t>(strategy), 0);
  }
  void add_intensity(uint32_t intensity) {
    fbb_.AddElement<uint32_t>(SupervisorFlags::VT_INTENSITY, intensity, 0);
  }
  void add_period(uint32_t period) {
    fbb_.AddElement<uint32_t>(SupervisorFlags::VT_PERIOD, period, 0);
  }
  explicit SupervisorFlagsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SupervisorFlagsBuilder &operator=(const SupervisorFlagsBuilder &);
  flatbuffers::Offset<SupervisorFlags> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SupervisorFlags>(end);
    return o;
  }
};

inline flatbuffers::Offset<SupervisorFlags> CreateSupervisorFlags(
    flatbuffers::FlatBufferBuilder &_fbb,
    SupervisionStrategy strategy = SupervisionStrategy::one_for_one,
    uint32_t intensity = 0,
    uint32_t period = 0) {
  SupervisorFlagsBuilder builder_(_fbb);
  builder_.add_period(period);
  builder_.add_intensity(intensity);
  builder_.add_strategy(strategy);
  return builder_.Finish();
}

struct ActorExecutionConfig FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ActorExecutionConfigTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "ActorModel.ActorExecutionConfig";
  }
  enum {
    VT_TASK_PRIO = 4,
    VT_TASK_STACK_SIZE = 6,
    VT_MAILBOX_SIZE = 8
  };
  int32_t task_prio() const {
    return GetField<int32_t>(VT_TASK_PRIO, 5);
  }
  bool mutate_task_prio(int32_t _task_prio) {
    return SetField<int32_t>(VT_TASK_PRIO, _task_prio, 5);
  }
  uint32_t task_stack_size() const {
    return GetField<uint32_t>(VT_TASK_STACK_SIZE, 4096);
  }
  bool mutate_task_stack_size(uint32_t _task_stack_size) {
    return SetField<uint32_t>(VT_TASK_STACK_SIZE, _task_stack_size, 4096);
  }
  uint32_t mailbox_size() const {
    return GetField<uint32_t>(VT_MAILBOX_SIZE, 4096);
  }
  bool mutate_mailbox_size(uint32_t _mailbox_size) {
    return SetField<uint32_t>(VT_MAILBOX_SIZE, _mailbox_size, 4096);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_TASK_PRIO) &&
           VerifyField<uint32_t>(verifier, VT_TASK_STACK_SIZE) &&
           VerifyField<uint32_t>(verifier, VT_MAILBOX_SIZE) &&
           verifier.EndTable();
  }
};

struct ActorExecutionConfigBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_task_prio(int32_t task_prio) {
    fbb_.AddElement<int32_t>(ActorExecutionConfig::VT_TASK_PRIO, task_prio, 5);
  }
  void add_task_stack_size(uint32_t task_stack_size) {
    fbb_.AddElement<uint32_t>(ActorExecutionConfig::VT_TASK_STACK_SIZE, task_stack_size, 4096);
  }
  void add_mailbox_size(uint32_t mailbox_size) {
    fbb_.AddElement<uint32_t>(ActorExecutionConfig::VT_MAILBOX_SIZE, mailbox_size, 4096);
  }
  explicit ActorExecutionConfigBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ActorExecutionConfigBuilder &operator=(const ActorExecutionConfigBuilder &);
  flatbuffers::Offset<ActorExecutionConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ActorExecutionConfig>(end);
    return o;
  }
};

inline flatbuffers::Offset<ActorExecutionConfig> CreateActorExecutionConfig(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t task_prio = 5,
    uint32_t task_stack_size = 4096,
    uint32_t mailbox_size = 4096) {
  ActorExecutionConfigBuilder builder_(_fbb);
  builder_.add_mailbox_size(mailbox_size);
  builder_.add_task_stack_size(task_stack_size);
  builder_.add_task_prio(task_prio);
  return builder_.Finish();
}

inline bool VerifyResult(flatbuffers::Verifier &verifier, const void *obj, Result type) {
  switch (type) {
    case Result::NONE: {
      return true;
    }
    case Result::Ok: {
      auto ptr = reinterpret_cast<const Ok *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Result::Error: {
      auto ptr = reinterpret_cast<const Error *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return false;
  }
}

inline bool VerifyResultVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyResult(
        verifier,  values->Get(i), types->GetEnum<Result>(i))) {
      return false;
    }
  }
  return true;
}

inline const flatbuffers::TypeTable *SupervisionStrategyTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    SupervisionStrategyTypeTable
  };
  static const char * const names[] = {
    "one_for_one",
    "one_for_all",
    "rest_for_one",
    "simple_one_for_one"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_ENUM, 4, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ProcessFlagTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SHORT, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    ProcessFlagTypeTable
  };
  static const char * const names[] = {
    "trap_exit"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_ENUM, 1, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ResultTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SEQUENCE, 0, -1 },
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_SEQUENCE, 0, 1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    OkTypeTable,
    ErrorTypeTable
  };
  static const char * const names[] = {
    "NONE",
    "Ok",
    "Error"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_UNION, 3, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *MessageTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 0, -1 },
    { flatbuffers::ET_ULONG, 0, -1 },
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_UINT, 0, -1 },
    { flatbuffers::ET_UCHAR, 1, -1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    UUIDTypeTable
  };
  static const char * const names[] = {
    "type",
    "timestamp",
    "from_pid",
    "payload_alignment",
    "payload"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 5, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *OkTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const char * const names[] = {
    "_"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ErrorTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const char * const names[] = {
    "reason"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *SignalTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    UUIDTypeTable
  };
  static const char * const names[] = {
    "from_pid",
    "reason"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *SupervisorFlagsTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_UINT, 0, -1 },
    { flatbuffers::ET_UINT, 0, -1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    SupervisionStrategyTypeTable
  };
  static const char * const names[] = {
    "strategy",
    "intensity",
    "period"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 3, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ActorExecutionConfigTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_INT, 0, -1 },
    { flatbuffers::ET_UINT, 0, -1 },
    { flatbuffers::ET_UINT, 0, -1 }
  };
  static const char * const names[] = {
    "task_prio",
    "task_stack_size",
    "mailbox_size"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 3, type_codes, nullptr, nullptr, names
  };
  return &tt;
}

inline const ActorModel::Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<ActorModel::Message>(buf);
}

inline const ActorModel::Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<ActorModel::Message>(buf);
}

inline Message *GetMutableMessage(void *buf) {
  return flatbuffers::GetMutableRoot<Message>(buf);
}

inline const char *MessageIdentifier() {
  return "Act!";
}

inline bool MessageBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, MessageIdentifier());
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<ActorModel::Message>(MessageIdentifier());
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<ActorModel::Message>(MessageIdentifier());
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<ActorModel::Message> root) {
  fbb.Finish(root, MessageIdentifier());
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<ActorModel::Message> root) {
  fbb.FinishSizePrefixed(root, MessageIdentifier());
}

}  // namespace ActorModel

#endif  // FLATBUFFERS_GENERATED_ACTORMODEL_ACTORMODEL_H_
