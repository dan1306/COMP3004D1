#include "VideoGame.h"
#include <sstream>

namespace hinlibs {

VideoGame::VideoGame(
        const std::string& title,
        const std::string& developer,
        int publicationYear,
        const std::string& genre,
        const std::string& rating
    )
    : Item(title, developer, publicationYear, ItemKind::VideoGame),
      genre_(genre),
      rating_(rating){}

const std::string& VideoGame::genre() const noexcept { return genre_; }
const std::string& VideoGame::rating() const noexcept { return rating_; }

std::string VideoGame::typeName() const { return "Video Game"; }

std::string VideoGame::detailsSummary() const {
    std::ostringstream oss;
    oss << "Title: " << title_
        << " | Studio: " << creator_
        << " | Publication Year: " << publicationYear_
        << " | Genre: " << genre_
        << " | Rating: " << rating_;
    return oss.str();
}

} // namespace hinlibs
