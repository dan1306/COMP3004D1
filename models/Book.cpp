#include "Book.h"
#include <sstream>

namespace hinlibs {

Book::Book(const std::string& title,
           const std::string& author,
           int publicationYear,
           BookType type,
           std::optional<std::string> dewey,
           std::optional<std::string> isbn
           ): Item(title, author, publicationYear, ItemKind::Book), type_(type),
dewey_(std::move(dewey)), isbn_(std::move(isbn)){}

BookType Book::bookType() const noexcept {
    return type_;
}

const std::optional<std::string>& Book::dewey() const noexcept {
    return dewey_;
}

const std::optional<std::string>& Book::isbn() const noexcept {
    return isbn_;
}

std::string Book::typeName() const {
    return (type_ == BookType::Fiction) ? "Fiction Book" : "Non-Fiction Book";
}

std::string Book::detailsSummary() const {
    std::ostringstream oss;
    oss << "Book Title: " << title_
        << " | Author: " << creator_
        << " | Publication Year: " << publicationYear_
        << " | Book Type:  " << Book::typeName();

    if (type_ == BookType::NonFiction && dewey_.has_value()) {
        oss << " | Dewey: " << dewey_.value();
    }
    if(isbn_.has_value()){
        oss << " | ISBN: " << isbn_.value();
    }
    return oss.str();
}

} // namespace hinlibs
