#pragma once
#include "Item.h"

namespace hinlibs {

class VideoGame : public Item {
public:
    VideoGame(
            const std::string& title,
            const std::string& developer,
            int publicationYear,
            const std::string& genre,
            const std::string& rating
        );

    const std::string& genre() const noexcept;
    const std::string& rating() const noexcept ;

    std::string typeName() const override;
    std::string detailsSummary() const override;

private:
    std::string genre_;
    std::string rating_;
};

} // namespace hinlibs
