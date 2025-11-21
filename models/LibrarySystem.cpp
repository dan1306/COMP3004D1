#include "LibrarySystem.h"
#include <algorithm>
#include <QDebug>

namespace hinlibs {

LibrarySystem::LibrarySystem() {
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName("db/hinlibs.sqlite3");


    if (!db_.open()) {
        qDebug() << "ERROR: " << db_.lastError();
        return;
    } else {
        qDebug() << "Working";
    }

    getUsersFromDB();
    getItemsFromDB();
}

// --- DB operation ---

void LibrarySystem::getUsersFromDB(){
    QSqlQuery query;
    query.prepare("SELECT * FROM users");

    if (!query.exec()) {
        qDebug() << "ERROR:" << query.lastError().text();
    } else {
        while(query.next()){
            int userid_ = query.value("userid_").toInt();
            std::string name_ = query.value("name_").toString().toStdString();
            std::string role_ = query.value("role_").toString().toStdString();

            std::shared_ptr<User> user;

            if(role_ == "Patron"){
                user = std::make_shared<Patron>(name_, userid_);
            } else if (role_ == "Librarian"){
                user = std::make_shared<User>(name_, Role::Librarian, userid_);
            } else if(role_ == "Administrator"){
               user = std::make_shared<User>(name_, Role::Administrator, userid_);
            }

            userIdByName_[user->name()] = userid_;
            usersById_[userid_] = std::move(user);

        }
    }
}

void LibrarySystem::getItemsFromDB() {

    QSqlQuery query;
    query.prepare("SELECT * FROM items");

    if (!query.exec()) {
        qDebug() << "ERROR:" << query.lastError().text();
    } else {
        while (query.next()) {

            int itemid_ = query.value("itemid_").toInt();
            std::string kind_ = query.value("kind_").toString().toStdString();
            std::string title_ = query.value("title_").toString().toStdString();
            std::string creator_ = query.value("creator_").toString().toStdString();
            int publicationYear_ = query.value("publicationYear_").toInt();

            std::optional<std::string> dewey_ = std::nullopt;
            QVariant deweyVal = query.value("dewey_");
            if (!deweyVal.isNull()) {
                dewey_ = deweyVal.toString().toStdString();
            }

            std::optional<std::string> isbn_ = std::nullopt;
            QVariant isbnVal = query.value("isbn_");
            if (!isbnVal.isNull()) {
                isbn_ = isbnVal.toString().toStdString();
            }

            int issueNumber_ = -1;
            QVariant issueNumberVal = query.value("issueNumber_");
            if (!issueNumberVal.isNull()) {
                issueNumber_ = issueNumberVal.toInt();
            }

            QDate publicationDate_;
            QVariant publicationDateVal = query.value("publicationDate_");
            if (!publicationDateVal.isNull()) {
                QString pubString_ = publicationDateVal.toString();
                publicationDate_ = QDate::fromString(pubString_, "yyyy-MM-dd");
            }

            std::string genre_ = "";
            QVariant genreVal = query.value("genre_");
            if (!genreVal.isNull()) {
                genre_ = genreVal.toString().toStdString();
            }

            std::string rating_ = "";
            QVariant ratingVal = query.value("rating_");
            if (!ratingVal.isNull()) {
                rating_ = ratingVal.toString().toStdString();
            }

            std::string status_ = query.value("status_").toString().toStdString();
            ItemStatus itemStatus;
            if (status_ == "CheckedOut") {
                itemStatus = ItemStatus::CheckedOut;
            } else {
                itemStatus = ItemStatus::Available;
            }

            if (kind_ == "FictionBook") {

                items_.push_back(std::make_shared<Book>(itemid_, title_, creator_, publicationYear_,BookType::Fiction, dewey_, isbn_, itemStatus));

            } else if (kind_ == "NonFictionBook") {

                items_.push_back(std::make_shared<Book>(itemid_, title_, creator_, publicationYear_, BookType::NonFiction, dewey_, isbn_, itemStatus));

            } else if (kind_ == "Magazine") {

                items_.push_back(std::make_shared<Magazine>(itemid_, title_, creator_, publicationYear_, issueNumber_, publicationDate_, itemStatus));

            } else if (kind_ == "Movie") {

                items_.push_back(std::make_shared<Movie>(itemid_, title_, creator_, publicationYear_, genre_, rating_, itemStatus));

            } else if (kind_ == "VideoGame") {

                items_.push_back(std::make_shared<VideoGame>(itemid_, title_, creator_, publicationYear_, genre_, rating_, itemStatus));
            }
        }
    }
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

            // Removing hold from DB
            QSqlQuery query3;
            query3.prepare("DELETE FROM holds WHERE itemid_ = :itemid_ AND userid_ = :userid_");
            query3.bindValue(":itemid_", itemId);
            query3.bindValue(":userid_", patronId);

            if (!query3.exec()) {
                qDebug() << "ERROR:" << query3.lastError().text();
                return false;

            }
        }
    }

    if (countLoansForPatron(patronId) >= MAX_ACTIVE_LOANS) return false;

    QSqlQuery query1;
    query1.prepare("UPDATE items SET status_='CheckedOut' WHERE itemid_ = :itemid_");
    query1.bindValue(":itemid_", itemId);

    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return false;
    }

    QSqlQuery query2;
    query2.prepare("INSERT INTO loans (userid_, itemid_, checkoutDate_, dueDate_) "
                   "VALUES (:userid_, :itemid_, :checkoutDate_, :dueDate_)");

    QString checkoutDate_ = QDate::currentDate().toString("yyyy-MM-dd");
    QString dueDate_ =  QDate::currentDate().addDays(LOAN_PERIOD_DAYS).toString("yyyy-MM-dd");

    query2.bindValue(":userid_", patronId);
    query2.bindValue(":itemid_", itemId);
    query2.bindValue(":checkoutDate_", checkoutDate_);
    query2.bindValue(":dueDate_", dueDate_);

    if (!query2.exec()) {
        qDebug() << "ERROR:" << query2.lastError().text();
        return false;
    }

    item->setStatus(ItemStatus::CheckedOut);       
    Loan loan{ itemId, patronId, QDate::currentDate(), QDate::currentDate().addDays(LOAN_PERIOD_DAYS) };
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

    if (it == loansByItemId_.end()) {
        return false;
    }

    bool result = (it->second.patronId == patronId);

    return result;
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


} // namespace hinlibs
