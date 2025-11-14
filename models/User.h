#pragma once
#include <string>

namespace hinlibs {

enum class Role { Patron, Librarian, Administrator };

class User {
public:
    User(std::string name, Role role);
    virtual ~User() = default;

    int id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    Role role() const noexcept { return role_; }

protected:
    static int user_id;
    int id_;
    std::string name_;
    Role role_;
};

} // namespace hinlibs
