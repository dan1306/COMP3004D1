#include "Magazine.h"
#include <sstream>

namespace hinlibs {

Magazine::Magazine(
        const std::string& title,
        const std::string& publisher,
        int publicationYear,
        int issueNumber,
        const QDate& publicationDate
        ): Item(title, publisher, publicationYear, ItemKind::Magazine),
issueNumber_(issueNumber), publicationDate_(publicationDate){};

int Magazine::issueNumber() const noexcept {
    return issueNumber_;
}

const QDate& Magazine::publicationDate() const noexcept{
    return publicationDate_;
};

std::string Magazine::typeName() const {
    return "Magazine";
}

std::string Magazine::detailsSummary() const {
    std::ostringstream oss;
    oss << "Title: " << title_
        << " | Publisher: " << creator_
        << " | Publication Year: " << publicationYear_
        << " | Issue#: " << issueNumber_
        << " | Publication Date: " << publicationDate_.toString("yyyy-MM-dd").toStdString();
    return oss.str();
}

} // namespace hinlibs
