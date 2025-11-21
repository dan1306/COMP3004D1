#include "User.h"

namespace hinlibs {


User::User(std::string name, Role role, int id_)
    : id_(id_), name_(std::move(name)), role_(role) {}

} // namespace hinlibs
