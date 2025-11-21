#pragma once
#include "User.h"

namespace hinlibs {

class Patron : public User {
public:
    explicit Patron(std::string name, int id_)
        : User(std::move(name), Role::Patron, id_) {}
    ~Patron() override = default;
};

} // namespace hinlibs
