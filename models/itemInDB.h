#pragma once

#include <string>
#include <optional>
#include <QDate>
#include "Item.h"  

namespace hinlibs {

struct ItemInDB {
    std::string kind_;
    std::string title_;
    std::string creator_;
    int publicationYear_;

    std::optional<std::string> dewey_;
    std::optional<std::string> isbn_;

    std::optional<int> issueNumber_;
    std::optional<QDate> publicationDate_;

    std::optional<std::string> genre_;
    std::optional<std::string> rating_;

    ItemStatus status_;
};

} 
