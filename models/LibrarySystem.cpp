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

    QSqlQuery query1;
    query1.prepare("SELECT userid_ FROM loans WHERE itemid_ = :itemid_ AND userid_ = :patronId_");

    query1.bindValue(":itemid_", itemId);
    query1.bindValue(":patronId_", patronId);
    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return false;
    }

    if(!query1.next()){
        // item has not been borrowed or patron isnt the one who borrowed item
        return false;
    }

    int userIDwhoBorrowed = query1.value("userid_").toInt();

    if(userIDwhoBorrowed != patronId){
        // not  the patron  of interest
        return false;
    }

    QSqlQuery query2;
    query2.prepare("DELETE FROM loans WHERE itemid_ = :itemid_ AND userid_ = :patronId_");
    query2.bindValue(":itemid_", itemId);
    query2.bindValue(":patronId_", patronId);
    if (!query2.exec()) {
        qDebug() << "ERROR:" << query2.lastError().text();
        return false;
    }


    QSqlQuery query3;
    query3.prepare("UPDATE items SET status_='Available' WHERE itemid_ = :itemid_");
    query3.bindValue(":itemid_", itemId);
    if (!query3.exec()) {
        qDebug() << "ERROR:" << query3.lastError().text();
        return false;
    }


    loansByItemId_.erase(it);

    // D1 behavior: simply mark available (no auto-assign to next hold)
    item->setStatus(ItemStatus::Available);
    return true;
}

bool LibrarySystem::placeHold(int patronId, int itemId) {

        QSqlQuery query1;
        query1.prepare("SELECT userid_, role_ FROM patrons WHERE userid_ = :patronId");
        query1.bindValue(":patronId", patronId);
        if (!query1.exec() || !query1.next()) {
            return false;
        }

        std::string role_ = query1.value("role_").toString().toStdString();

        if(role_ != "Patron"){
            return false;
        }

        QSqlQuery query2;

        query2.prepare("SELECT status_ FROM items WHERE itemid_ = :itemId");
        query2.bindValue(":itemId", itemId);
        if (!query2.exec() || !query2.next()) {
            return false;
        }

        std::string status_ = query2.value("status_").toString().toStdString();

        if ( status_ != "CheckedOut") {
            return false;
        }

        QSqlQuery query3;

        query3.prepare("SELECT userid_, itemid_ FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
        query3.bindValue(":itemId", itemId);
        query3.bindValue(":patronId", patronId);
        if (query3.exec() && query3.next()) {
            return false;
        }


        QSqlQuery query4;

        query4.prepare("SELECT userid_, itemid_ FROM holds WHERE itemid_ = :itemId AND userid_ = :patronId");
        query4.bindValue(":itemId", itemId); // Re-bind for clarity
        query4.bindValue(":patronId", patronId);
        if (query4.exec() && query4.next()) {
            return false;
        }

        QSqlQuery query5;
        query5.prepare("INSERT INTO holds (itemid_, userid_) VALUES (:itemId, :patronId)");
        query5.bindValue(":itemId", itemId);
        query5.bindValue(":patronId", patronId);

        if (!query5.exec()) {
             qDebug() << "Error:" << query5.lastError().text();
             return false;
        }

        return true;

}

bool LibrarySystem::cancelHold(int patronId, int itemId) {
    QSqlQuery query1;
    query1.prepare("SELECT itemid_, userid_ FROM holds WHERE userid_ = :patronId  AND itemid_ = :itemId");
    query1.bindValue(":patronId", patronId);
    query1.bindValue(":itemId", itemId);

    if(!query1.exec() || !query1.next()){
        return false;
    }

    QSqlQuery query2;
    query2.prepare("DELETE FROM holds WHERE userid_ = :patronId AND itemid_ = :itemId");
    query2.bindValue(":patronId", patronId);
    query2.bindValue(":itemId", itemId);

    if(!query2.exec()){
        return false;
    }

    return true;


}

std::vector<LibrarySystem::AccountLoan>
LibrarySystem::getAccountLoans(int patronId, const QDate& today) const {
    std::vector<AccountLoan> out;
    QSqlQuery query1;
    query1.prepare("SELECT l.dueDate_, i.itemid_, i.title_ FROM loans l JOIN items i ON i.itemid_ = l.itemid_ WHERE l.userid_ = :patronId");
    query1.bindValue(":patronId", patronId);
    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return out;
    }

      while (query1.next()) {
          AccountLoan al;
          al.itemId = query1.value("itemid_").toInt();
          al.title = query1.value("title_").toString().toStdString();
          QDate dueDate = query1.value("dueDate_").toDate();
          al.dueDate = dueDate;
          al.daysRemaining = today.daysTo(dueDate);

          out.push_back(std::move(al));
      }

      return out;
}

std::vector<LibrarySystem::AccountHold>
LibrarySystem::getAccountHolds(int patronId) const {
    std::vector<AccountHold> out;


    QSqlQuery query1;
    query1.prepare("SELECT itemid_ FROM loans WHERE userid_= :patronId");
    query1.bindValue(":patronId", patronId);

    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return out;
    }

    while (query1.next()) {

        int itemid_ = query1.value("itemid_").toInt();

        QSqlQuery query2;
        query2.prepare("SELECT userid_ FROM loans WHERE itemid_= :itemid_ ORDER BY loanid_ ASC");
        query2.bindValue(":itemid_", itemid_);

        if (!query2.exec()) {
            qDebug() << "ERROR:" << query1.lastError().text();
            continue;
        }

        int queuePos = 1;
        while(query2.next()){
            if(query2.value("userid_").toInt() == patronId){
                QSqlQuery query3;
                query3.prepare("SELECT itemid_, title_ FROM items WHERE itemid_ = :itemid_");
                query3.bindValue(":itemid_", itemid_);
                if (!query3.exec()) {
                    qDebug() << "ERROR:" << query3.lastError().text();
                    continue;
                }

                if(query3.next()){
                    AccountHold foundItemQueuePos;
                    foundItemQueuePos.itemId = query3.value("itemid_").toUInt();
                    foundItemQueuePos.title = query3.value("title_").toString().toStdString();
                    foundItemQueuePos.queuePosition = queuePos;
                    out.push_back(std::move(foundItemQueuePos));
                }



                break;
            }
            queuePos++;
        }

    }

    return out;
}
// --- helpers ---

int LibrarySystem::countLoansForPatron(int patronId) const {
//    int count = 0;
//    for (const auto& kv : loansByItemId_) {
//        if (kv.second.patronId == patronId) ++count;
//    }
//    return count;
    QSqlQuery query1;
    query1.prepare("SELECT COUNT(*) AS num_of_loans FROM loans WHERE userid_= :patronId");
    query1.bindValue(":patronId", patronId);

    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return 0;
    }

    if(!query1.next()){
        return 0;
    }

    return query1.value("num_of_loans").toInt();

}

bool LibrarySystem::isLoanedBy(int itemId, int patronId) const {

//    auto it = loansByItemId_.find(itemId);

//    if (it == loansByItemId_.end()) {
//        return false;
//    }

//    bool result = (it->second.patronId == patronId);

//    return result;
    QSqlQuery query1;
    query1.prepare("SELECT userid_ FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
    query1.bindValue(":itemId", itemId);
    query1.bindValue(":patronId", patronId);
    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return false;
    }

    if(!query1.next()){
        return false;
    }

    int userIDwhoBorrowed = query1.value("userid_").toInt();

    if(userIDwhoBorrowed != patronId){
        return false;
    }

    return true;

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
