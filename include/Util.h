/*
 * Util.h
 *
 *  Created on: Oct 1, 2018
 *      Author: ahueck
 */

#include "llvm/Demangle/Demangle.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Module.h"

namespace irprinter::util {

namespace detail {
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf :
template <class>
struct type_sink {
  using type = void;
};
template <class T>
using type_sink_t = typename type_sink<T>::type;

#define has_member(_NAME_)                  \
  template <class T, class = void>          \
  struct has_##_NAME_ : std::false_type {}; \
  template <class T>                        \
  struct has_##_NAME_<T, type_sink_t<decltype(std::declval<T>()._NAME_())>> : std::true_type {};

// clang-format off
has_member(begin)
has_member(end)
#undef has_member

template <typename T>
using has_begin_end_t = typename std::integral_constant<bool,
      has_begin<T>{} && has_end<T>{}>::type;
// clang-format on

template <typename Val>
inline std::string dump(const Val& s, std::false_type) {
  std::string tmp;
  llvm::raw_string_ostream out(tmp);
  s.print(out);

  return tmp;
}

template <typename Val>
inline std::string dump(const Val& s, std::true_type) {
  auto beg = s.begin();
  auto end = s.end();
  if (beg == end) {
    return "[ ]";
  }

  std::string tmp;
  llvm::raw_string_ostream out(tmp);
  auto next = std::next(beg);
  out << "[ " << *(*(beg));
  std::for_each(next, end, [&out](auto v) { out << " , " << *v; });
  out << " ]";
  return out.str();
}

}  // namespace detail

template <typename Val>
inline std::string dump(const Val& s) {
  using namespace detail;
  return dump(s, has_begin_end_t<Val>{});
}

template <typename String>
inline std::string try_demangle(String s) {
  std::string name = s;
  auto demangle    = llvm::itaniumDemangle(s.data(), nullptr, nullptr, nullptr);
  if (demangle && std::string(demangle) != "") {
    return std::string(demangle);
  }
  return name;
}

inline bool regex_matches(const std::string& regex, const std::string& in, bool case_sensitive = false) {
  using namespace llvm;
  Regex r(regex, !case_sensitive ? Regex::IgnoreCase : Regex::NoFlags);
  return r.match(in);
}

namespace detail {
template <typename Predicate>
inline llvm::SmallVector<const llvm::Function*, 4> find(const llvm::Module& m, Predicate p) {
  llvm::SmallVector<const llvm::Function*, 4> data;

  for (const auto& f : m.functions()) {
    if (p(f)) {
      data.push_back(&f);
    }
  }

  return data;
}
}  // namespace detail

inline llvm::SmallVector<const llvm::Function*, 4> regex_find(const llvm::Module& m, const std::string& regex = ".*",
                                                              bool use_mangle = false) {
  llvm::Regex r(regex);
  if (use_mangle) {
    return detail::find(m, [&](const llvm::Function& f) { return r.match(f.getName()); });
  } else {
    return detail::find(m, [&](const llvm::Function& f) { return r.match(try_demangle(f.getName())); });
  }
}

}  // namespace irprinter::util
