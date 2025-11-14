#include "User.h"

namespace hinlibs {

int User::user_id = 0;

User::User(std::string name, Role role)
    : id_(++user_id), name_(std::move(name)), role_(role) {}

} // namespace hinlibs
