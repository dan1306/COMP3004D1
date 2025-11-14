#include "LibrarySystem.h"
#include <algorithm>

namespace hinlibs {

LibrarySystem::LibrarySystem() {
    seed();
}

std::shared_ptr<User> LibrarySystem::findUserByName(const std::string& name) const {
    auto it = userIdByName_.find(name);
    if (it == userIdByName_.end()) return nullptr;
    auto it2 = usersById_.find(it->second);
    if (it2 == usersById_.end()) return nullptr;
    return it2->second;
}

std::shared_ptr<Patron> LibrarySystem::getPatronById(int patronId) const {
    auto it = usersById_.find(patronId);
    if (it == usersById_.end()) return nullptr;
    if (it->second->role() != Role::Patron) return nullptr;
    return std::static_pointer_cast<Patron>(it->second);
}

std::shared_ptr<Item> LibrarySystem::getItemById(int itemId) const {
    for (auto& it : items_) {
        if (it->id() == itemId) return it;
    }
    return nullptr;
}

// --- Patron operations ---

bool LibrarySystem::borrowItem(int patronId, int itemId) {
    auto patron = getPatronById(patronId);
    if (!patron) return false;

    auto item = getItemById(itemId);
    if (!item) return false;
    if (item->status() != ItemStatus::Available) return false;

    // Enforce queue fairness if holds exist
    auto hit = holdsByItemId_.find(itemId);
    if (hit != holdsByItemId_.end() && !hit->second.empty()) {
        if (hit->second.front() != patronId) {
            // someone else is first in line
            return false;
        } else {
            // the borrower is first in queue -> pop them
            hit->second.pop_front();
        }
    }

    if (countLoansForPatron(patronId) >= MAX_ACTIVE_LOANS) return false;

    item->setStatus(ItemStatus::CheckedOut);
    QDate today = QDate::currentDate();
    Loan loan{ itemId, patronId, today, today.addDays(LOAN_PERIOD_DAYS) };
    loansByItemId_[itemId] = loan;
    return true;
}

bool LibrarySystem::returnItem(int patronId, int itemId) {
    auto item = getItemById(itemId);
    if (!item) return false;

    auto it = loansByItemId_.find(itemId);
    if (it == loansByItemId_.end()) return false;
    if (it->second.patronId != patronId) return false;

    loansByItemId_.erase(it);

    // D1 behavior: simply mark available (no auto-assign to next hold)
    item->setStatus(ItemStatus::Available);
    return true;
}

bool LibrarySystem::placeHold(int patronId, int itemId) {
    auto patron = getPatronById(patronId);
    if (!patron) return false;

    auto item = getItemById(itemId);
    if (!item) return false;

    if(isLoanedBy(itemId, patronId)) return false;
    // Holds allowed only on checked-out items
    if (item->status() != ItemStatus::CheckedOut) return false;

    auto& queue = holdsByItemId_[itemId];
    auto it = std::find(queue.begin(), queue.end(), patronId);
    if (it != queue.end()) return false; // already in queue

    queue.push_back(patronId);
    return true;
}

bool LibrarySystem::cancelHold(int patronId, int itemId) {
    auto it = holdsByItemId_.find(itemId);
    if (it == holdsByItemId_.end()) return false;

    auto& queue = it->second;
    auto qit = std::find(queue.begin(), queue.end(), patronId);
    if (qit == queue.end()) return false;

    queue.erase(qit); // positions shift automatically
    // If queue becomes empty, we can optionally erase the entry; not required.
    if (queue.empty()) holdsByItemId_.erase(it);
    return true;
}

std::vector<LibrarySystem::AccountLoan>
LibrarySystem::getAccountLoans(int patronId, const QDate& today) const {
    std::vector<AccountLoan> out;
    for (const auto& kv : loansByItemId_) {
        const auto& loan = kv.second;
        if (loan.patronId == patronId) {
            auto item = getItemById(loan.itemId);
            if (!item) continue;
            AccountLoan al;
            al.itemId = loan.itemId;
            al.title = item->title();
            al.dueDate = loan.due;
            al.daysRemaining = today.daysTo(loan.due);
            out.push_back(std::move(al));
        }
    }
    return out;
}

std::vector<LibrarySystem::AccountHold>
LibrarySystem::getAccountHolds(int patronId) const {
    std::vector<AccountHold> out;
    for (const auto& kv : holdsByItemId_) {
        int itemId = kv.first;
        const auto& queue = kv.second;
        auto it = std::find(queue.begin(), queue.end(), patronId);
        if (it != queue.end()) {
            auto item = getItemById(itemId);
            if (!item) continue;
            AccountHold ah;
            ah.itemId = itemId;
            ah.title = item->title();
            ah.queuePosition = static_cast<int>(std::distance(queue.begin(), it)) + 1;
            out.push_back(std::move(ah));
        }
    }
    return out;
}

// --- helpers ---

int LibrarySystem::countLoansForPatron(int patronId) const {
    int count = 0;
    for (const auto& kv : loansByItemId_) {
        if (kv.second.patronId == patronId) ++count;
    }
    return count;
}

bool LibrarySystem::isLoanedBy(int itemId, int patronId) const {
    auto it = loansByItemId_.find(itemId);
    return (it != loansByItemId_.end() && it->second.patronId == patronId);
}


// log of patron transaction operations
//void LibrarySystem::logUserActivity(int patronID, const std::string& activity) {
//    QDateTime currentDateAndTime= QDateTime::currentDateTime();
//    std::string currentDateAndTimeStringWithActivity = currentDateAndTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() + " " + activity;
//    log_of_user_activites[patronID].push_back(currentDateAndTimeStringWithActivity);
//};

//std::vector<std::string> LibrarySystem::getPatronUserActivities(int patronID) const {
//    auto find_id = log_of_user_activites.find(patronID);
//    if(find_id != log_of_user_activites.end()){
//        return find_id->second;
//    };
//    std::vector<std::string>  empty;
//    return empty;
//};





// --- seed data ---

void LibrarySystem::seed() {
    // Users: 5 patrons + 1 librarian + 1 admin
    // IDs: 1..7
    auto addUser = [&](std::shared_ptr<User> u) {
        int id = u->id(); // id auto generated by user constructor
        userIdByName_[u->name()] = id;
        usersById_[id] = std::move(u);
    };

    addUser(std::make_shared<Patron>("Alice"));
    addUser(std::make_shared<Patron>("Bob"));
    addUser(std::make_shared<Patron>("Carmen"));
    addUser(std::make_shared<Patron>("Dinesh"));
    addUser(std::make_shared<Patron>("Eve"));
    addUser(std::make_shared<User>("Librarian", Role::Librarian));
    addUser(std::make_shared<User>("Admin", Role::Administrator));

    // Items: total 20

    // 5 Fiction books
    items_.push_back(std::make_shared<Book>("The Silent Forest", "J. Rivera", 2016, BookType::Fiction));
    items_.push_back(std::make_shared<Book>("City of Glass", "P. Daniels", 2010, BookType::Fiction));
    items_.push_back(std::make_shared<Book>("North Star", "A. Patel", 2018, BookType::Fiction));
    items_.push_back(std::make_shared<Book>("Shadows & Light", "M. Hassan", 2021, BookType::Fiction));
    items_.push_back(std::make_shared<Book>("The Last Harbor", "K. Wong", 2012, BookType::Fiction));

    // 5 Non-fiction books (with Dewey)
    items_.push_back(std::make_shared<Book>("Quantum Realities", "L. Chen", 2019, BookType::NonFiction, std::string("530.12")));
    items_.push_back(std::make_shared<Book>("Creative Cooking", "R. Singh", 2015, BookType::NonFiction, std::string("641.59")));
    items_.push_back(std::make_shared<Book>("World History Abridged", "T. Romero", 2013, BookType::NonFiction, std::string("909.07")));
    items_.push_back(std::make_shared<Book>("Behavioral Economics", "S. Ahmed", 2017, BookType::NonFiction, std::string("330.01")));
    items_.push_back(std::make_shared<Book>("Astronomy 101", "C. Martins", 2020, BookType::NonFiction, std::string("520.1")));

    // 3 Magazines (issue + pub date)
    items_.push_back(std::make_shared<Magazine>("Tech Monthly", "TechHouse", 2024, 142, QDate(2024, 9, 1)));
    items_.push_back(std::make_shared<Magazine>("Health Weekly", "WellnessPub", 2025, 27, QDate(2025, 10, 15)));
    items_.push_back(std::make_shared<Magazine>("Art & Design", "Canvas Press", 2025, 5, QDate(2025, 7, 20)));

    // 3 Movies (genre + rating)
    items_.push_back(std::make_shared<Movie>("Through the Mist", "I. Novak", 2014, "Drama", "PG-13"));
    items_.push_back(std::make_shared<Movie>("Deep Orbit", "G. Adebayo", 2022, "Sci-Fi", "PG-13"));
    items_.push_back(std::make_shared<Movie>("Hidden Trail", "S. Yamamoto", 2008, "Thriller", "R"));

    // 4 Video Games (genre + rating)
    items_.push_back(std::make_shared<VideoGame>("Starfall Odyssey", "NebulaWorks", 2023, "Action RPG", "T"));
    items_.push_back(std::make_shared<VideoGame>("Garden Realms", "BloomSoft", 2020, "Simulation", "E"));
    items_.push_back(std::make_shared<VideoGame>("Cipher Run", "VoxelForge", 2019, "Platformer", "E10+"));
    items_.push_back(std::make_shared<VideoGame>("Kings of Aether", "Crown Labs", 2017, "Strategy", "T"));

    // All items start Available by default (in Item base)
}

} // namespace hinlibs
