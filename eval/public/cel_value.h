#ifndef THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_CEL_VALUE_H_
#define THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_CEL_VALUE_H_

// CelValue is a holder, capable of storing all kinds of data
// supported by CEL.
// CelValue defines explicitly typed/named getters/setters.
// When storing pointers to objects, CelValue does not accept ownership
// to them and does not control their lifecycle. Instead objects are expected
// to be either external to expression evaluation, and controlled beyond the
// scope or to be allocated and associated with some allocation/ownership
// controller (Arena).
// Usage examples:
// (a) For primitive types:
//    CelValue value = CelValue::CreateInt64(1);
// (b) For string:
//    string* msg = google::protobuf::Arena::Create<string>(arena,"test");
//    CelValue value = CelValue::CreateString(msg);
// (c) For messages:
//    const MyMessage * msg = google::protobuf::Arena::CreateMessage<MyMessage>(arena);
//    CelValue value = CelValue::CreateMessage(msg, &arena);

#include "google/protobuf/duration.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "eval/public/cel_value_internal.h"
#include "internal/proto_util.h"
#include "base/statusor.h"

namespace google {
namespace api {
namespace expr {
namespace runtime {

using CelError = absl::Status;

// Break cyclic depdendencies for container types.
class CelList;
class CelMap;
class UnknownSet;

class CelValue {
 public:
  // This class is a container to hold strings/bytes.
  // Template parameter N is an artificial discriminator, used to create
  // distinct types for String and Bytes (we need distinct types for Oneof).
  template <int N>
  class StringHolderBase {
   public:
    StringHolderBase() : value_() {}

    StringHolderBase(const StringHolderBase &) = default;
    StringHolderBase &operator=(const StringHolderBase &) = default;

    // string parameter is passed through pointer to ensure string_view is not
    // initialized with string rvalue. Also, according to Google style guide,
    // passing pointers conveys the message that the reference to string is kept
    // in the constructed holder object.
    explicit StringHolderBase(const std::string *str) : value_(*str) {}

    absl::string_view value() const { return value_; }

    // Group of comparison operations.
    friend bool operator==(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ == value2.value_;
    }

    friend bool operator!=(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ != value2.value_;
    }

    friend bool operator<(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ < value2.value_;
    }

    friend bool operator<=(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ <= value2.value_;
    }

    friend bool operator>(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ > value2.value_;
    }

    friend bool operator>=(StringHolderBase value1, StringHolderBase value2) {
      return value1.value_ >= value2.value_;
    }

    friend class CelValue;

   private:
    explicit StringHolderBase(absl::string_view other) : value_(other) {}

    absl::string_view value_;
  };

  // Helper structure for String datatype.
  using StringHolder = StringHolderBase<0>;

  // Helper structure for Bytes datatype.
  using BytesHolder = StringHolderBase<1>;

 private:
  using ValueHolder = internal::ValueHolder<
      bool, int64_t, uint64_t, double, StringHolder, BytesHolder,
      const google::protobuf::Message *, absl::Duration, absl::Time, const CelList *,
      const CelMap *, const UnknownSet *, const CelError *>;

 public:
  // Metafunction providing positions corresponding to specific
  // types. If type is not supported, compile-time error will occur.
  template <class T>
  using IndexOf = ValueHolder::IndexOf<T>;

  // Enum for types supported.
  enum class Type {
    kBool = IndexOf<bool>::value,
    kInt64 = IndexOf<int64_t>::value,
    kUint64 = IndexOf<uint64_t>::value,
    kDouble = IndexOf<double>::value,
    kString = IndexOf<StringHolder>::value,
    kBytes = IndexOf<BytesHolder>::value,
    kMessage = IndexOf<const google::protobuf::Message *>::value,
    kDuration = IndexOf<absl::Duration>::value,
    kTimestamp = IndexOf<absl::Time>::value,
    kList = IndexOf<const CelList *>::value,
    kMap = IndexOf<const CelMap *>::value,
    kUnknownSet = IndexOf<const UnknownSet *>::value,
    kError = IndexOf<const CelError *>::value,
    kAny  // Special value. Used in function descriptors.
  };

  // Default constructor.
  // Creates CelValue with null data type.
  CelValue() : CelValue(static_cast<const google::protobuf::Message *>(nullptr)) {}

  // Returns Type that describes the type of value stored.
  Type type() const { return Type(value_.index()); }

  // We will use factory methods instead of public constructors
  // The reason for this is the high risk of implicit type conversions
  // between bool/int/pointer types.
  // We rely on copy elision to avoid extra copying.
  static CelValue CreateNull() {
    return CelValue(static_cast<const google::protobuf::Message *>(nullptr));
  }

  static CelValue CreateBool(bool value) { return CelValue(value); }

  static CelValue CreateInt64(int64_t value) { return CelValue(value); }

  static CelValue CreateUint64(uint64_t value) { return CelValue(value); }

  static CelValue CreateDouble(double value) { return CelValue(value); }

  static CelValue CreateString(StringHolder holder) { return CelValue(holder); }

  static CelValue CreateStringView(absl::string_view value) {
    return CelValue(StringHolder(value));
  }

  static CelValue CreateString(const std::string *str) {
    return CelValue(StringHolder(str));
  }

  static CelValue CreateBytes(BytesHolder holder) { return CelValue(holder); }

  static CelValue CreateBytes(const std::string *str) {
    return CelValue(BytesHolder(str));
  }

  // CreateMessage creates CelValue from google::protobuf::Message.
  // As some of CEL basic types are subclassing google::protobuf::Message,
  // this method contains type checking and downcasts.
  static CelValue CreateMessage(const google::protobuf::Message *value,
                                google::protobuf::Arena *arena);

  static CelValue CreateDuration(const google::protobuf::Duration *value) {
    return CelValue(expr::internal::DecodeDuration(*value));
  }

  static CelValue CreateDuration(absl::Duration value) {
    return CelValue(value);
  }

  static CelValue CreateTimestamp(const google::protobuf::Timestamp *value) {
    return CelValue(expr::internal::DecodeTime(*value));
  }

  static CelValue CreateTimestamp(absl::Time value) { return CelValue(value); }

  static CelValue CreateList(const CelList *value) {
    CheckNullPointer(value, Type::kList);
    return CelValue(value);
  }

  static CelValue CreateMap(const CelMap *value) {
    CheckNullPointer(value, Type::kMap);
    return CelValue(value);
  }

  static CelValue CreateUnknownSet(const UnknownSet *value) {
    CheckNullPointer(value, Type::kUnknownSet);
    return CelValue(value);
  }

  static CelValue CreateError(const CelError *value) {
    CheckNullPointer(value, Type::kError);
    return CelValue(value);
  }

  // Methods for accessing values of specific type
  // They have the common usage pattern - prior to accessing the
  // value, the caller should check that the value of this type is indeed
  // stored in CelValue, using type() or Is...() methods.

  // Returns stored boolean value.
  // Fails if stored value type is not boolean.
  bool BoolOrDie() const { return GetValueOrDie<bool>(Type::kBool); }

  // Returns stored int64_t value.
  // Fails if stored value type is not int64_t.
  int64_t Int64OrDie() const { return GetValueOrDie<int64_t>(Type::kInt64); }

  // Returns stored uint64_t value.
  // Fails if stored value type is not uint64_t.
  uint64_t Uint64OrDie() const { return GetValueOrDie<uint64_t>(Type::kUint64); }

  // Returns stored double value.
  // Fails if stored value type is not double.
  double DoubleOrDie() const { return GetValueOrDie<double>(Type::kDouble); }

  // Returns stored const string * value.
  // Fails if stored value type is not const string *.
  StringHolder StringOrDie() const {
    return GetValueOrDie<StringHolder>(Type::kString);
  }

  BytesHolder BytesOrDie() const {
    return GetValueOrDie<BytesHolder>(Type::kBytes);
  }

  // Returns stored const Message * value.
  // Fails if stored value type is not const Message *.
  const google::protobuf::Message *MessageOrDie() const {
    return GetValueOrDie<const google::protobuf::Message *>(Type::kMessage);
  }

  // Returns stored duration value.
  // Fails if stored value type is not duration.
  const absl::Duration DurationOrDie() const {
    return GetValueOrDie<absl::Duration>(Type::kDuration);
  }

  // Returns stored timestamp value.
  // Fails if stored value type is not timestamp.
  const absl::Time TimestampOrDie() const {
    return GetValueOrDie<absl::Time>(Type::kTimestamp);
  }

  // Returns stored const CelList * value.
  // Fails if stored value type is not const CelList *.
  const CelList *ListOrDie() const {
    return GetValueOrDie<const CelList *>(Type::kList);
  }

  // Returns stored const CelMap * value.
  // Fails if stored value type is not const CelMap *.
  const CelMap *MapOrDie() const {
    return GetValueOrDie<const CelMap *>(Type::kMap);
  }

  // Returns stored const UnknownAttributeSet * value.
  // Fails if stored value type is not const UnknownAttributeSet *.
  const UnknownSet *UnknownSetOrDie() const {
    return GetValueOrDie<const UnknownSet *>(Type::kUnknownSet);
  }

  // Returns stored const CelError * value.
  // Fails if stored value type is not const CelError *.
  const CelError *ErrorOrDie() const {
    return GetValueOrDie<const CelError *>(Type::kError);
  }

  bool IsNull() const { return value_.template Visit<bool>(NullCheckOp()); }

  bool IsBool() const { return value_.is<bool>(); }

  bool IsInt64() const { return value_.is<int64_t>(); }

  bool IsUint64() const { return value_.is<uint64_t>(); }

  bool IsDouble() const { return value_.is<double>(); }

  bool IsString() const { return value_.is<StringHolder>(); }

  bool IsBytes() const { return value_.is<BytesHolder>(); }

  bool IsMessage() const { return value_.is<const google::protobuf::Message *>(); }

  bool IsDuration() const { return value_.is<absl::Duration>(); }

  bool IsTimestamp() const { return value_.is<absl::Time>(); }

  bool IsList() const { return value_.is<const CelList *>(); }

  bool IsMap() const { return value_.is<const CelMap *>(); }

  bool IsUnknownSet() const { return value_.is<const UnknownSet *>(); }

  bool IsError() const { return value_.is<const CelError *>(); }

  // Invokes op() with the active value, and returns the result.
  // All overloads of op() must have the same return type.
  template <class ReturnType, class Op>
  ReturnType Visit(Op &&op) const {
    return value_.template Visit<ReturnType>(op);
  }

  // Template-style getter.
  // Returns true, if assignment successful
  template <typename Arg>
  bool GetValue(Arg *value) const {
    return this->template Visit<bool>(AssignerOp<Arg>(value));
  }

  // Provides type names for internal logging.
  static std::string TypeName(Type value_type);

 private:
  ValueHolder value_;

  template <typename T>
  struct AssignerOp {
    explicit AssignerOp(T *val) : value(val) {}

    template <typename U>
    bool operator()(const U &) {
      return false;
    }

    bool operator()(const T &arg) {
      *value = arg;
      return true;
    }

    T *value;
  };

  struct NullCheckOp {
    template <typename T>
    bool operator()(const T &) const {
      return false;
    }

    bool operator()(const google::protobuf::Message *arg) const { return arg == nullptr; }
  };

  // Constructs CelValue wrapping value supplied as argument.
  // Value type T should be supported by specification of ValueHolder.
  template <class T>
  explicit CelValue(T value) : value_(value) {}

  // Null pointer checker for pointer-based types.
  static void CheckNullPointer(const void *ptr, Type type) {
    if (ptr == nullptr) {
      GOOGLE_LOG(FATAL) << "Null pointer supplied for " << TypeName(type);  // Crash ok
    }
  }

  // Gets value of type specified
  template <class T>
  T GetValueOrDie(Type requested_type) const {
    auto value_ptr = value_.get<T>();
    if (value_ptr == nullptr) {
      GOOGLE_LOG(FATAL) << "Type mismatch"                            // Crash ok
                 << ": expected " << TypeName(requested_type)  // Crash ok
                 << ", encountered " << TypeName(type());      // Crash ok
    }
    return *value_ptr;
  }
};

// CelList is a base class for list adapting classes.
class CelList {
 public:
  virtual CelValue operator[](int index) const = 0;

  // List size
  virtual int size() const = 0;
  // Default empty check. Can be overridden in subclass for performance.
  virtual bool empty() const { return size() == 0; }

  virtual ~CelList() {}
};

// MapAccessor is a base class for map accessors.
class CelMap {
 public:
  // Map lookup. If value found,
  // returns CelValue in return type.
  // Per Protobuffer specification, acceptable key types are
  // int64_t,uint64,string.
  virtual absl::optional<CelValue> operator[](CelValue key) const = 0;

  // Map size
  virtual int size() const = 0;
  // Default empty check. Can be overridden in subclass for performance.
  virtual bool empty() const { return size() == 0; }

  // Return list of keys. CelList is owned by Arena, so no
  // ownership is passed.
  virtual const CelList *ListKeys() const = 0;

  virtual ~CelMap() {}
};

// Utility method that generates CelValue containing CelError.
// message an error message
// error_code error code
// position location of the error source in CEL expression string the Expr was
// parsed from. -1, if the position can not be determined.
CelValue CreateErrorValue(
    google::protobuf::Arena *arena, absl::string_view message,
    absl::StatusCode error_code = absl::StatusCode::kUnknown,
    int position = -1);

CelValue CreateNoMatchingOverloadError(google::protobuf::Arena *arena);
bool CheckNoMatchingOverloadError(CelValue value);

CelValue CreateNoSuchFieldError(google::protobuf::Arena *arena);

CelValue CreateNoSuchKeyError(google::protobuf::Arena *arena, absl::string_view key);
bool CheckNoSuchKeyError(CelValue value);

// Returns the error indicating that evaluation encountered a value marked
// as unknown, was included in Activation unknown_paths.
CelValue CreateUnknownValueError(google::protobuf::Arena *arena,
                                 absl::string_view unknown_path);

// Returns true if this is unknown value error indicating that evaluation
// encountered a value marked as unknown in Activation unknown_paths.
bool IsUnknownValueError(const CelValue &value);

// Returns error indicating the result of the function is unknown. This is used
// as a signal to create an unknown set if unknown function handling is opted
// into.
CelValue CreateUnknownFunctionResultError(google::protobuf::Arena *arena,
                                          absl::string_view help_message);

// Returns true if this is unknown value error indicating that evaluation
// called an extension function whose value is unknown for the given args.
// This is used as a signal to convert to an UnknownSet if the behavior is opted
// into.
bool IsUnknownFunctionResult(const CelValue &value);

// Returns set of unknown paths for unknown value error. The value must be
// unknown error, see IsUnknownValueError() above, or it dies.
std::set<std::string> GetUnknownPathsSetOrDie(const CelValue &value);

}  // namespace runtime
}  // namespace expr
}  // namespace api
}  // namespace google

#endif  // THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_CEL_VALUE_H_
