// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_HTTPSERVER_HTTPSERVER_H_
#define FLATBUFFERS_GENERATED_HTTPSERVER_HTTPSERVER_H_

#include "flatbuffers/flatbuffers.h"

#include "uuid_generated.h"

struct HTTPServerConfiguration;

inline const flatbuffers::TypeTable *HTTPServerConfigurationTypeTable();

struct HTTPServerConfiguration FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return HTTPServerConfigurationTypeTable();
  }
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "HTTPServerConfiguration";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PORT = 4,
    VT_TO_PID = 6
  };
  uint16_t port() const {
    return GetField<uint16_t>(VT_PORT, 80);
  }
  bool mutate_port(uint16_t _port) {
    return SetField<uint16_t>(VT_PORT, _port, 80);
  }
  const UUID::UUID *to_pid() const {
    return GetStruct<const UUID::UUID *>(VT_TO_PID);
  }
  UUID::UUID *mutable_to_pid() {
    return GetStruct<UUID::UUID *>(VT_TO_PID);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint16_t>(verifier, VT_PORT) &&
           VerifyField<UUID::UUID>(verifier, VT_TO_PID) &&
           verifier.EndTable();
  }
};

struct HTTPServerConfigurationBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_port(uint16_t port) {
    fbb_.AddElement<uint16_t>(HTTPServerConfiguration::VT_PORT, port, 80);
  }
  void add_to_pid(const UUID::UUID *to_pid) {
    fbb_.AddStruct(HTTPServerConfiguration::VT_TO_PID, to_pid);
  }
  explicit HTTPServerConfigurationBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  HTTPServerConfigurationBuilder &operator=(const HTTPServerConfigurationBuilder &);
  flatbuffers::Offset<HTTPServerConfiguration> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<HTTPServerConfiguration>(end);
    return o;
  }
};

inline flatbuffers::Offset<HTTPServerConfiguration> CreateHTTPServerConfiguration(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint16_t port = 80,
    const UUID::UUID *to_pid = 0) {
  HTTPServerConfigurationBuilder builder_(_fbb);
  builder_.add_to_pid(to_pid);
  builder_.add_port(port);
  return builder_.Finish();
}

inline const flatbuffers::TypeTable *HTTPServerConfigurationTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_USHORT, 0, -1 },
    { flatbuffers::ET_SEQUENCE, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    UUID::UUIDTypeTable
  };
  static const char * const names[] = {
    "port",
    "to_pid"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, names
  };
  return &tt;
}

inline const HTTPServerConfiguration *GetHTTPServerConfiguration(const void *buf) {
  return flatbuffers::GetRoot<HTTPServerConfiguration>(buf);
}

inline const HTTPServerConfiguration *GetSizePrefixedHTTPServerConfiguration(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<HTTPServerConfiguration>(buf);
}

inline HTTPServerConfiguration *GetMutableHTTPServerConfiguration(void *buf) {
  return flatbuffers::GetMutableRoot<HTTPServerConfiguration>(buf);
}

inline const char *HTTPServerConfigurationIdentifier() {
  return "H11S";
}

inline bool HTTPServerConfigurationBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, HTTPServerConfigurationIdentifier());
}

inline bool VerifyHTTPServerConfigurationBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<HTTPServerConfiguration>(HTTPServerConfigurationIdentifier());
}

inline bool VerifySizePrefixedHTTPServerConfigurationBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<HTTPServerConfiguration>(HTTPServerConfigurationIdentifier());
}

inline const char *HTTPServerConfigurationExtension() {
  return "fb";
}

inline void FinishHTTPServerConfigurationBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<HTTPServerConfiguration> root) {
  fbb.Finish(root, HTTPServerConfigurationIdentifier());
}

inline void FinishSizePrefixedHTTPServerConfigurationBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<HTTPServerConfiguration> root) {
  fbb.FinishSizePrefixed(root, HTTPServerConfigurationIdentifier());
}

#endif  // FLATBUFFERS_GENERATED_HTTPSERVER_HTTPSERVER_H_
