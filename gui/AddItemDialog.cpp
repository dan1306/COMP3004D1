#include "AddItemDialog.h"
#include "ui_AddItemDialog.h"

#include <QMessageBox>

using namespace hinlibs;

AddItemDialog::AddItemDialog(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AddItemDialog) {
    ui->setupUi(this);

    // Populate type combo
    ui->comboType->addItem("Fiction Book");
    ui->comboType->addItem("Non-Fiction Book");
    ui->comboType->addItem("Magazine");
    ui->comboType->addItem("Movie");
    ui->comboType->addItem("Video Game");

    // Reasonable defaults
    ui->spinYear->setMinimum(0);
    ui->spinYear->setMaximum(2100);
    ui->spinYear->setValue(QDate::currentDate().year());

    ui->datePublication->setDate(QDate::currentDate());

    connect(ui->comboType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddItemDialog::onTypeChanged);

    updateFieldVisibility();
}

AddItemDialog::~AddItemDialog() = default;

void AddItemDialog::onTypeChanged(int) {
    updateFieldVisibility();
}

void AddItemDialog::updateFieldVisibility() {
    const int typeIndex = ui->comboType->currentIndex();

    // Show all common fields
    ui->labelTitle->setVisible(true);
    ui->lineTitle->setVisible(true);
    ui->labelCreator->setVisible(true);
    ui->lineCreator->setVisible(true);
    ui->labelYear->setVisible(true);
    ui->spinYear->setVisible(true);

    auto showDewey = false;
    auto showIsbn  = false;
    auto showIssue = false;
    auto showPubDate = false;
    auto showGenre = false;
    auto showRating = false;

    switch (typeIndex) {
    case 0: // Fiction Book
        showIsbn = true;
        break;
    case 1: // Non-Fiction Book
        showDewey = true;
        showIsbn  = true;
        break;
    case 2: // Magazine
        showIssue   = true;
        showPubDate = true;
        break;
    case 3: // Movie
        showGenre  = true;
        showRating = true;
        break;
    case 4: // Video Game
        showGenre  = true;
        showRating = true;
        break;
    default:
        break;
    }

    ui->labelDewey->setVisible(showDewey);
    ui->lineDewey->setVisible(showDewey);

    ui->labelIsbn->setVisible(showIsbn);
    ui->lineIsbn->setVisible(showIsbn);

    ui->labelIssueNumber->setVisible(showIssue);
    ui->spinIssueNumber->setVisible(showIssue);

    ui->labelPublicationDate->setVisible(showPubDate);
    ui->datePublication->setVisible(showPubDate);

    ui->labelGenre->setVisible(showGenre);
    ui->lineGenre->setVisible(showGenre);

    ui->labelRating->setVisible(showRating);
    ui->lineRating->setVisible(showRating);
}

void AddItemDialog::accept() {
    const QString title   = ui->lineTitle->text().trimmed();
    const QString creator = ui->lineCreator->text().trimmed();
    const int year        = ui->spinYear->value();
    const int typeIndex   = ui->comboType->currentIndex();

    if (title.isEmpty() || creator.isEmpty()) {
        QMessageBox::warning(this, "Invalid Data",
                             "Title and Creator/Author field must not be empty.");
        return;
    }

    if (year <= 0) {
        QMessageBox::warning(this, "Invalid Data",
                             "Publication year must be positive.");
        return;
    }

    ItemInDB data;
    data.title_  = title.toStdString();
    data.creator_ = creator.toStdString();
    data.publicationYear_ = year;
    data.status_ = ItemStatus::Available;  

    // Clear optionals
    data.dewey_.reset();
    data.isbn_.reset();
    data.issueNumber_.reset();
    data.publicationDate_.reset();
    data.genre_.reset();
    data.rating_.reset();

    switch (typeIndex) {
    case 0: // Fiction Book
        data.kind_ = "FictionBook";
        if (!ui->lineIsbn->text().trimmed().isEmpty()) {
            data.isbn_ = ui->lineIsbn->text().trimmed().toStdString();
        }
        break;
    case 1: // Non-Fiction Book
        data.kind_ = "NonFictionBook";
        if (!ui->lineDewey->text().trimmed().isEmpty()) {
            data.dewey_ = ui->lineDewey->text().trimmed().toStdString();
        }
        if (!ui->lineIsbn->text().trimmed().isEmpty()) {
            data.isbn_ = ui->lineIsbn->text().trimmed().toStdString();
        }
        break;
    case 2: // Magazine
        data.kind_ = "Magazine";
        data.issueNumber_ = ui->spinIssueNumber->value();
        data.publicationDate_ = ui->datePublication->date();
        break;
    case 3: // Movie
        data.kind_ = "Movie";
        if (!ui->lineGenre->text().trimmed().isEmpty()) {
            data.genre_ = ui->lineGenre->text().trimmed().toStdString();
        }
        if (!ui->lineRating->text().trimmed().isEmpty()) {
            data.rating_ = ui->lineRating->text().trimmed().toStdString();
        }
        break;
    case 4: // Video Game
        data.kind_ = "VideoGame";
        if (!ui->lineGenre->text().trimmed().isEmpty()) {
            data.genre_ = ui->lineGenre->text().trimmed().toStdString();
        }
        if (!ui->lineRating->text().trimmed().isEmpty()) {
            data.rating_ = ui->lineRating->text().trimmed().toStdString();
        }
        break;
    default:
        QMessageBox::warning(this, "Invalid Data",
                             "Unknown item type selected.");
        return;
    }

    // Save to member and close dialog
    itemData_ = data;

    QDialog::accept();
}
