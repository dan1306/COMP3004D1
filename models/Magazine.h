#pragma once
#include "Item.h"
#include <QDate>
#include <string>

namespace hinlibs {

class Magazine : public Item {
public:
    Magazine(
            const std::string& title,
            const std::string& publisher,
            int publicationYear,
            int issueNumber,
            const QDate& publicationDate
            );

    int issueNumber() const noexcept;
    const QDate& publicationDate() const noexcept;

    std::string typeName() const override;
    std::string detailsSummary() const override;

private:
    int issueNumber_;
    QDate publicationDate_;
};

} // namespace hinlibs
