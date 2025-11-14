#pragma once
#include "User.h"

namespace hinlibs {

class Patron : public User {
public:
    explicit Patron(std::string name)
        : User(std::move(name), Role::Patron) {}
    ~Patron() override = default;
};

} // namespace hinlibs
