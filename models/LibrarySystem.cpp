#include "LibrarySystem.h"
#include <algorithm>
#include <QDebug>
#include <functional>
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

    if (countLoansForPatron(patronId) >= MAX_ACTIVE_LOANS) return false;

    QSqlQuery query1;

    query1.prepare("SELECT * FROM users WHERE userid_ = :patronId");
    query1.bindValue(":patronId", patronId);
    if (!query1.exec() || !query1.next()) return false;
    if(query1.value("role_").toString().toStdString() != "Patron") return false;



    QSqlQuery query2;
    query2.prepare("SELECT * FROM items WHERE itemid_ = :itemId");
    query2.bindValue(":itemId", itemId);
    if (!query2.exec() || !query2.next()) return false;
    if(query2.value("status_").toString().toStdString() != "Available") return false;

    QSqlQuery query3;
    query3.prepare("SELECT * FROM holds WHERE itemid_ = :itemId ORDER BY holdid_ ASC");
    query3.bindValue(":itemId", itemId);
    if(!query3.exec()) return false;
    if (query3.next()) {
        if (query3.value("userid_").toInt() != patronId) {
            return false;
        }
        QSqlQuery query4;
        query4.prepare("DELETE FROM holds WHERE itemid_ = :itemId AND userid_ = :patronId");
        query4.bindValue(":itemId", itemId);
        query4.bindValue(":patronId", patronId);
        if (!query4.exec()) return false;

    }

    QSqlQuery query5;
    query5.prepare("SELECT * FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
    query5.bindValue(":itemId", itemId);
    query5.bindValue(":patronId", patronId);

    if (!query5.exec()) return false;

    if (query5.next()) {
        return false; // Cannot borrow the same item twice
    }


    QSqlQuery query6;

    query6.prepare("UPDATE items SET status_ = 'CheckedOut' WHERE itemid_ = :itemId");
    query6.bindValue(":itemId", itemId);
    if (!query6.exec()) return false;

    QSqlQuery query7;

    query7.prepare("INSERT INTO loans (userid_, itemid_, checkoutDate_, dueDate_) "
                   "VALUES (:patronId, :itemId, :checkoutDate_, :dueDate_)");
    QString checkoutDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString dueDate =  QDate::currentDate().addDays(LOAN_PERIOD_DAYS).toString("yyyy-MM-dd");
    query7.bindValue(":checkoutDate_", checkoutDate);
    query7.bindValue(":dueDate_", dueDate);
    query7.bindValue(":patronId", patronId);
    query7.bindValue(":itemId", itemId);
    if (!query7.exec()) return false;


    for (auto& i : items_) {
        if (i->id() == itemId) {
            i->setStatus(ItemStatus::CheckedOut);
            break;
        }
    }

    Loan loan{ itemId, patronId, QDate::currentDate(), QDate::currentDate().addDays(LOAN_PERIOD_DAYS) };
    loansByItemId_[itemId] = loan;

    return true;

}

bool LibrarySystem::returnItem(int patronId, int itemId) {


    QSqlQuery query1;

    query1.prepare("SELECT * FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
    query1.bindValue(":itemId", itemId);
    query1.bindValue(":patronId", patronId);
    if (!query1.exec() || !query1.next()) {
        qDebug() << "Error:" << query1.lastError().text();
        return false;
    }

    QSqlQuery query2;
    query2.prepare("DELETE FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
    query2.bindValue(":itemId", itemId);
    query2.bindValue(":patronId", patronId);
    if (!query2.exec()) {
        qDebug() << "Error:" << query2.lastError().text();
        return false;
    }

    QSqlQuery query3;
    query3.prepare("UPDATE items SET status_ = :status_ WHERE itemid_ = :itemId");
    query3.bindValue(":status_", "Available");
    query3.bindValue(":itemId", itemId);
    if (!query3.exec()) {
        qDebug() << "Error:" << query3.lastError().text();
        return false;
    }

    auto it = loansByItemId_.find(itemId);
    if (it != loansByItemId_.end()) {
        loansByItemId_.erase(it);
    }

    for (auto& itemPtr : items_) {
        if (itemPtr->id() == itemId) {
            itemPtr->setStatus(ItemStatus::Available);
            break;
        }
    }

    return true;


}

//bool LibrarySystem::placeHold(int patronId, int itemId) {

//        QSqlQuery query1;
//        query1.prepare("SELECT userid_, role_ FROM users WHERE userid_ = :patronId");
//        query1.bindValue(":patronId", patronId);
//        if (!query1.exec() || !query1.next()) {
//            return false;
//        }

//        std::string role_ = query1.value("role_").toString().toStdString();

//        if(role_ != "Patron"){
//            return false;
//        }

//        QSqlQuery query2;

//        query2.prepare("SELECT status_ FROM items WHERE itemid_ = :itemId");
//        query2.bindValue(":itemId", itemId);
//        if (!query2.exec() || !query2.next()) {
//            return false;
//        }

//        std::string status_ = query2.value("status_").toString().toStdString();

//        if ( status_ != "CheckedOut") return false;

//        QSqlQuery amIfirstInTheHoldQueue;

//        amIfirstInTheHoldQueue.prepare("SELECT * FROM holds where itemid_ = :itemId  ORDER BY holdid_ ASC");
//        amIfirstInTheHoldQueue.bindValue(":itemId", itemId);

//        if(!amIfirstInTheHoldQueue.exec()) return false;
//        if (status_ == "Available") {
//            if (amIfirstInTheHoldQueue.next()) {
//                if (amIfirstInTheHoldQueue.value("userid_").toInt() == patronId) {
//                    }
//                }
//        }



//        QSqlQuery query3;

//        query3.prepare("SELECT userid_, itemid_ FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
//        query3.bindValue(":itemId", itemId);
//        query3.bindValue(":patronId", patronId);
//        if (query3.exec() && query3.next()) {
//            return false;
//        }


//        QSqlQuery query4;

//        query4.prepare("SELECT userid_, itemid_ FROM holds WHERE itemid_ = :itemId AND userid_ = :patronId");
//        query4.bindValue(":itemId", itemId);
//        query4.bindValue(":patronId", patronId);
//        if (query4.exec() && query4.next()) {
//            return false;
//        }

//        QSqlQuery query5;
//        query5.prepare("INSERT INTO holds (itemid_, userid_) VALUES (:itemId, :patronId)");
//        query5.bindValue(":itemId", itemId);
//        query5.bindValue(":patronId", patronId);

//        if (!query5.exec()) {
//             qDebug() << "Error:" << query5.lastError().text();
//             return false;
//        }

//        return true;

//}

// this one is a bit complicated the one above I  tried to implement this logic but now it is just there as reference
bool LibrarySystem::placeHold(int patronId, int itemId) {

        QSqlQuery query1;
        query1.prepare("SELECT userid_, role_ FROM users WHERE userid_ = :patronId");
        query1.bindValue(":patronId", patronId);
        if (!query1.exec() || !query1.next()) {
            return false; // User not found
        }

        std::string role_ = query1.value("role_").toString().toStdString();

        if(role_ != "Patron"){
            return false; // Must be a Patron
        }

        QSqlQuery query2;

        query2.prepare("SELECT status_ FROM items WHERE itemid_ = :itemId");
        query2.bindValue(":itemId", itemId);
        if (!query2.exec() || !query2.next()) {
            return false; // Item not found
        }

        std::string status_ = query2.value("status_").toString().toStdString();

        // Check if the item is Available with no holds
        if ( status_ == "Available") {
            QSqlQuery checkHolds;
            checkHolds.prepare("SELECT holdid_ FROM holds WHERE itemid_ = :itemId");
            checkHolds.bindValue(":itemId", itemId);
            if (!checkHolds.exec()) return false;

            // If item is Available AND no holds exist, the item is free to borrow. Cannot place a hold.
            if (!checkHolds.next()) {
                return false;
            }
        }

        // If status is CheckedOut OR status is Available with holds, continue to place the new hold.

        QSqlQuery query3;
        // Check if patron already has the item on loan
        query3.prepare("SELECT userid_, itemid_ FROM loans WHERE itemid_ = :itemId AND userid_ = :patronId");
        query3.bindValue(":itemId", itemId);
        query3.bindValue(":patronId", patronId);
        if (query3.exec() && query3.next()) {
            return false;
        }


        QSqlQuery query4;
        // Check if patron already has an active hold on this item
        query4.prepare("SELECT userid_, itemid_ FROM holds WHERE itemid_ = :itemId AND userid_ = :patronId");
        query4.bindValue(":itemId", itemId);
        query4.bindValue(":patronId", patronId);
        if (query4.exec() && query4.next()) {
            return false;
        }

        QSqlQuery query5;
        // Insert the new hold
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
    query1.prepare("SELECT itemid_ FROM holds WHERE userid_= :patronId");
    query1.bindValue(":patronId", patronId);

    if (!query1.exec()) {
        qDebug() << "ERROR:" << query1.lastError().text();
        return out;
    }

    while (query1.next()) {

        int itemid_ = query1.value("itemid_").toInt();

        QSqlQuery query2;
        query2.prepare("SELECT userid_ FROM holds WHERE itemid_= :itemid_ ORDER BY holdid_ ASC");
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

// Librrarian Operation

bool LibrarySystem::removeItemFromCatalogue(int librarianId, int itemId){
    auto it = usersById_.find(librarianId);
    if (it == usersById_.end()) return false;
    if (it->second->role() != Role::Librarian) return false;

    QSqlQuery query1;
    query1.prepare(
        "SELECT * FROM  items WHERE itemid_ = :itemid_"
    );

    query1.bindValue(":itemid_", itemId);
    if (!query1.exec() || !query1.next()) return false;

    std::string status_ = query1.value("status_").toString().toStdString();

    if(status_ != "Available") return false;

    QSqlQuery query2;
    query2.prepare("DELETE FROM holds WHERE itemid_ = :itemid_");
    query2.bindValue(":itemid_", itemId);

    if (!query2.exec()) return false;

    QSqlQuery query3;
    query3.prepare("DELETE FROM items WHERE itemid_ = :itemid_");
    query3.bindValue(":itemid_", itemId);

    if (!query3.exec()) return false;


    removeItemByID(itemId);
    return true;


}

void LibrarySystem::removeItemByID(int itemid_){

    auto itemStart = std::remove_if(
        items_.begin(),
        items_.end(),
        [itemid_]( std::shared_ptr<Item>& p) {
            return (p != nullptr && p->id() == itemid_);
        }
    );

    items_.erase(itemStart, items_.end());
}

bool LibrarySystem::addItemToCatalogue(int librarianID, const ItemInDB& item){

    auto it = usersById_.find(librarianID);
    if (it == usersById_.end()) return false;
    if (it->second->role() != Role::Librarian) return false;

    QSqlQuery query1;

    query1.prepare(
        "INSERT INTO items (kind_, title_, creator_, publicationYear_, dewey_, isbn_, "
        "issueNumber_, publicationDate_, genre_, rating_, status_) "
        "VALUES (:kind_, :title_, :creator_, :publicationYear_, :dewey_, :isbn_, "
        ":issueNumber_, :publicationDate_, :genre_, :rating_, :status_)"
    );

    query1.bindValue(":kind_", QString::fromStdString(item.kind_));
    query1.bindValue(":title_", QString::fromStdString(item.title_));
    query1.bindValue(":creator_", QString::fromStdString(item.creator_));
    query1.bindValue(":publicationYear_", item.publicationYear_);

    QVariant deweyVal_= item.dewey_.has_value() ? QVariant(QString::fromStdString(item.dewey_.value())) : QVariant(QVariant::String);

    query1.bindValue(":dewey_", deweyVal_);

    QVariant isbnVal_= item.isbn_.has_value() ? QVariant(QString::fromStdString(item.isbn_.value())) : QVariant(QVariant::String);

    query1.bindValue(":isbn_", isbnVal_);

    QVariant issueNumberVal_ = item.issueNumber_.has_value()  ? QVariant(item.issueNumber_.value()) : QVariant(QVariant::Int);

    query1.bindValue(":issueNumber_", issueNumberVal_);

    QVariant publicationDateVal_ = item.publicationDate_.has_value() ? QVariant(item.publicationDate_.value().toString("yyyy-MM-dd")) : QVariant(QVariant::String);

    query1.bindValue(":publicationDate_", publicationDateVal_);

    QVariant genreVal_ = item.genre_.has_value() ? QVariant(QString::fromStdString(item.genre_.value())) : QVariant(QVariant::String);

    query1.bindValue(":genre_", genreVal_);

    QVariant ratingVal_ = item.rating_.has_value()? QVariant(QString::fromStdString(item.rating_.value())) : QVariant(QVariant::String);

    query1.bindValue(":rating_", ratingVal_);

    QString statusVal_ = "Available";

    query1.bindValue(":status_", statusVal_);

    if (!query1.exec()) {
        qDebug() << "ERROR: " << query1.lastError();
        return false;
    }

    int lastInsertedID = query1.lastInsertId().toInt();
    if (lastInsertedID < 1) {
        qDebug() << "ERROR: Somthing unexpected happened while inserting.";
        return false;
    }


    QSqlQuery query2;
    query2.prepare("SELECT * FROM items WHERE itemid_ = :itemid_");
    query2.bindValue(":itemid_", lastInsertedID);

    if (!query2.exec() || !query2.next()) return false;

    int itemid_ = query2.value("itemid_").toInt();
    std::string kind_ = query2.value("kind_").toString().toStdString();
    std::string title_ = query2.value("title_").toString().toStdString();
    std::string creator_ = query2.value("creator_").toString().toStdString();
    int publicationYear_ = query2.value("publicationYear_").toInt();

    std::optional<std::string> dewey_ = std::nullopt;
    QVariant deweyVal = query2.value("dewey_");
    if (!deweyVal.isNull()) {
        dewey_ = deweyVal.toString().toStdString();
    }

    std::optional<std::string> isbn_ = std::nullopt;
    QVariant isbnVal = query2.value("isbn_");
    if (!isbnVal.isNull()) {
        isbn_ = isbnVal.toString().toStdString();
    }

    int issueNumber_ = -1;
    QVariant issueNumberVal = query2.value("issueNumber_");
    if (!issueNumberVal.isNull()) {
        issueNumber_ = issueNumberVal.toInt();
    }

    QDate publicationDate_;
    QVariant publicationDateVal = query2.value("publicationDate_");
    if (!publicationDateVal.isNull()) {
        QString pubString_ = publicationDateVal.toString();
        publicationDate_ = QDate::fromString(pubString_, "yyyy-MM-dd");
    }

    std::string genre_ = "";
    QVariant genreVal = query2.value("genre_");
    if (!genreVal.isNull()) {
        genre_ = genreVal.toString().toStdString();
    }

    std::string rating_ = "";
    QVariant ratingVal = query2.value("rating_");
    if (!ratingVal.isNull()) {
        rating_ = ratingVal.toString().toStdString();
    }

    std::string status_ = query2.value("status_").toString().toStdString();
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



