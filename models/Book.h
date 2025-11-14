#pragma once
#include "Item.h"
#include <optional>
#include <string>

namespace hinlibs {

enum class BookType { Fiction, NonFiction };

class Book : public Item {
public:
    // For NonFiction, provide dewey (e.g., "641.59"). For Fiction, leave std::nullopt.
    Book(
         const std::string& title,
         const std::string& author,
         int publicationYear,
         BookType type,
         std::optional<std::string> dewey = std::nullopt,
         std::optional<std::string> isbn = std::nullopt
        );

    BookType bookType() const noexcept;
    const std::optional<std::string>& dewey() const noexcept;
    const std::optional<std::string>& isbn() const noexcept;


    std::string typeName() const override;
    std::string detailsSummary() const override;

private:
    BookType type_;
    std::optional<std::string> dewey_;
    std::optional<std::string> isbn_;
};

} // namespace hinlibs
