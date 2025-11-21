#pragma once
#include "Item.h"
#include <string>

namespace hinlibs {

class Movie : public Item {
public:
    Movie(
            int id_,
            const std::string& title,
            const std::string& director,
            int publicationYear,
            const std::string& genre,
            const std::string& rating,
            ItemStatus circulationstatus = ItemStatus::Available
         );

    const std::string& genre() const noexcept;
    const std::string& rating() const noexcept;

    std::string typeName() const override;
    std::string detailsSummary() const override;

private:
    std::string genre_;
    std::string rating_;
};

} // namespace hinlibs
