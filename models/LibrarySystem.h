#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <optional>
#include <QDate>

#include "User.h"
#include "Patron.h"
#include "Item.h"
#include "Book.h"
#include "Movie.h"
#include "VideoGame.h"
#include "Magazine.h"
#include "itemInDB.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


namespace hinlibs {

class LibrarySystem {
public:
    LibrarySystem();

    // --- DB OPerations --
    void getUsersFromDB();
    void getItemsFromDB();

    // --- Users ---
    std::shared_ptr<User> findUserByName(const std::string& name) const;
    std::shared_ptr<Patron> getPatronById(int patronId) const;

    // --- Items ---
    std::shared_ptr<Item> getItemById(int itemId) const;
    const std::vector<std::shared_ptr<Item>>& allItems() const { return items_; }

    // --- Patron operations ---
    bool borrowItem(int patronId, int itemId);                       // today = QDate::currentDate()
    bool returnItem(int patronId, int itemId);
    bool placeHold(int patronId, int itemId);
    bool cancelHold(int patronId, int itemId);
    bool isLoanedBy(int itemId, int patronId) const;

//    // log of patron transaction operations
//    void logUserActivity(int patronID, const std::string& activity);
//    std::vector<std::string> getPatronUserActivities(int patronID) const;

    struct AccountLoan {
        int itemId;
        std::string title;
        QDate dueDate;
        int daysRemaining; // can be negative if overdue (no fines in D1)
    };
    struct AccountHold {
        int itemId;
        std::string title;
        int queuePosition; // 1-based
    };

    std::vector<AccountLoan> getAccountLoans(int patronId, const QDate& today = QDate::currentDate()) const;
    std::vector<AccountHold> getAccountHolds(int patronId) const;

    // Libraraian Operations
    bool removeItemFromCatalogue(int librarianID, int itemId);
    bool addItemToCatalogue(int librarianID, const ItemInDB& data);
    void removeItemByID(int itemid_);

    // Constants
    static constexpr int MAX_ACTIVE_LOANS = 3;
    static constexpr int LOAN_PERIOD_DAYS = 14;




private:
    QSqlDatabase db_;
    struct Loan {
        int itemId{};
        int patronId{};
        QDate checkout{};
        QDate due{};
    };

    // state
    std::vector<std::shared_ptr<Item>> items_;
    std::unordered_map<int, std::shared_ptr<User>> usersById_;
    std::unordered_map<std::string, int> userIdByName_;           // case-sensitive exact match (D1)
    std::unordered_map<int, Loan> loansByItemId_;                 // itemId -> loan
    std::unordered_map<int, std::deque<int>> holdsByItemId_;      // itemId -> FIFO patronIds
//    std::unordered_map<int, std::vector<std::string>> log_of_user_activites;  // Track patron system activites

    // helpers
    void seed();
    int countLoansForPatron(int patronId) const;

    static int daysBetween(const QDate& a, const QDate& b) { return a.daysTo(b); }
};

} // namespace hinlibs
