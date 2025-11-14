#include "Item.h"

namespace hinlibs {

int Item::item_id = 0;

Item::Item(
            const std::string& title,
            const std::string& creator,
            int publicationYear,
            ItemKind kind
        ): id_(++item_id),
           title_(title),
           creator_(creator),
           publicationYear_(publicationYear),
           kind_(kind),
           status_(ItemStatus::Available),
           condition_(Condition::Undamaged){}

// Getters
int Item::id() const noexcept {
    return id_;
}

const std::string& Item::title() const noexcept {
    return title_;
}

const std::string& Item::creator() const noexcept {
    return creator_;
}

int Item::publicationYear() const noexcept {
    return publicationYear_;
}

ItemKind Item::kind() const noexcept {
    return kind_;
}

Condition Item::condition() const noexcept {
    return condition_;
}

ItemStatus Item::status() const noexcept {
    return  status_;
}

// Setters
void Item::setStatus(ItemStatus s) noexcept {
    status_ = s;
}

void Item::setCondition(Condition c) noexcept {
    condition_ = c;
}

} // namespace hinlibs
