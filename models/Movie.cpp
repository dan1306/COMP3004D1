#include "Movie.h"
#include <sstream>

namespace hinlibs {

Movie::Movie(
        const std::string& title,
        const std::string& director,
        int publicationYear,
        const std::string& genre,
        const std::string& rating
        ): Item(title, director, publicationYear, ItemKind::Movie),
    genre_(genre), rating_(rating){}

const std::string& Movie::genre() const noexcept {
    return genre_;
}

const std::string& Movie::rating() const noexcept {
    return rating_;
}

std::string Movie::typeName() const {
    return "Movie";
}

std::string Movie::detailsSummary() const {
    std::ostringstream oss;
    oss << "Title: " << title_
        << " | Director: " << creator_
        << " | Publication Year: " << publicationYear_
        << " | Genre: " << genre_
        << " | Rating: " << rating_;
    return oss.str();
}

} // namespace hinlibs
