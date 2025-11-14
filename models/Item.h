#pragma once
#include <string>
#include <QQueue>
#include <QDate>

namespace hinlibs {

enum class ItemStatus { Available, CheckedOut, Unavailable };
enum class ItemKind   { Book, Movie, VideoGame, Magazine };
enum class Condition {Damaged, Undamaged };

class Item {
public:
    Item(
         const std::string& title,
         const std::string& creator,
         int publicationYear,
         ItemKind kind
        );

    virtual ~Item() = default;

    // Getters
    int id() const noexcept;
    const std::string& title() const noexcept;
    const std::string& creator() const noexcept;
    int publicationYear() const noexcept;
    ItemKind kind() const noexcept;
    ItemStatus status() const noexcept;
    Condition condition() const noexcept;

    // Setters
    void setStatus(ItemStatus s) noexcept;
    void setCondition(Condition c) noexcept;

    // For UI display
    virtual std::string typeName() const = 0;
    virtual std::string detailsSummary() const = 0;

protected:
    static int item_id;

    int id_;
    std::string title_;
    std::string creator_;
    int publicationYear_;
    ItemKind kind_;
    ItemStatus status_;
    Condition condition_;
};

} // namespace hinlibs
