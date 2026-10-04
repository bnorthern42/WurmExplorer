#include "GrangerPanel.hpp"
#include "GrangerStore.hpp"
#include "GrangerTraits.hpp"
#include "BreedingEvaluator.hpp"
#include "TraitSelectionDialog.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>

namespace granger {

using namespace treasure::ui;

GrangerPanel::GrangerPanel(std::shared_ptr<GrangerStore> store, QWidget* parent)
    : QWidget(parent), m_store(store) {
    setupUi();
    refreshAll();
}

void GrangerPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createAnimalsTab(), "🐎 Animals");
    tabs->addTab(createBreedersTab(), "🧑 Breeders");
    tabs->addTab(createBreedingTab(), "❤️ Breeding Pair");

    mainLayout->addWidget(tabs);
}

QWidget* GrangerPanel::createAnimalsTab() {
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    m_animalSearchEdit = new QLineEdit(tab);
    m_animalSearchEdit->setPlaceholderText("🔍 Search animals by name or trait...");
    connect(m_animalSearchEdit, &QLineEdit::textChanged, this, &GrangerPanel::onAnimalSearchChanged);
    layout->addWidget(m_animalSearchEdit);

    m_animalList = new QListWidget(tab);
    m_animalList->setMinimumHeight(150);
    connect(m_animalList, &QListWidget::currentRowChanged, this, &GrangerPanel::onAnimalSelected);
    layout->addWidget(m_animalList, 1);

    auto* scrollArea = new QScrollArea(tab);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* formContainer = new QWidget(scrollArea);
    auto* formLayout = new QFormLayout(formContainer);
    formLayout->setContentsMargins(4, 4, 4, 4);
    formLayout->setSpacing(6);

    m_animalNameEdit = new QLineEdit(formContainer);
    formLayout->addRow("Name:", m_animalNameEdit);

    m_animalTypeEdit = new QLineEdit(formContainer);
    m_animalTypeEdit->setPlaceholderText("e.g. Horse, Champion Horse");
    formLayout->addRow("Species / Type:", m_animalTypeEdit);

    m_animalMotherCombo = new QComboBox(formContainer);
    formLayout->addRow("Mother (Dam):", m_animalMotherCombo);

    m_animalFatherCombo = new QComboBox(formContainer);
    formLayout->addRow("Father (Sire):", m_animalFatherCombo);

    m_animalCaredByCombo = new QComboBox(formContainer);
    formLayout->addRow("Cared By:", m_animalCaredByCombo);

    auto* traitsRow = new QHBoxLayout();
    auto* editTraitsBtn = new QPushButton("Edit Traits...", formContainer);
    connect(editTraitsBtn, &QPushButton::clicked, this, &GrangerPanel::onEditTraitsClicked);
    traitsRow->addWidget(editTraitsBtn);

    m_traitsSummaryLabel = new QLabel(formContainer);
    m_traitsSummaryLabel->setWordWrap(true);
    m_traitsSummaryLabel->setText(QString("<span style='color: %1; font-style: italic;'>None</span>").arg(theme::TEXT_SECONDARY));

    formLayout->addRow("Traits:", traitsRow);
    formLayout->addRow("", m_traitsSummaryLabel);

    m_animalNotesEdit = new QLineEdit(formContainer);
    formLayout->addRow("Notes:", m_animalNotesEdit);

    auto* btnRow = new QHBoxLayout();
    auto* newBtn = new QPushButton("➕ New", formContainer);
    connect(newBtn, &QPushButton::clicked, this, &GrangerPanel::onNewAnimalClicked);

    m_saveAnimalBtn = new QPushButton("💾 Save", formContainer);
    m_saveAnimalBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    connect(m_saveAnimalBtn, &QPushButton::clicked, this, &GrangerPanel::onSaveAnimalClicked);

    m_deleteAnimalBtn = new QPushButton("🗑 Delete", formContainer);
    m_deleteAnimalBtn->setStyleSheet(QString("color: %1; font-weight: bold;").arg(theme::STATUS_DANGER));
    m_deleteAnimalBtn->setEnabled(false);
    connect(m_deleteAnimalBtn, &QPushButton::clicked, this, &GrangerPanel::onDeleteAnimalClicked);

    btnRow->addWidget(newBtn);
    btnRow->addWidget(m_saveAnimalBtn);
    btnRow->addWidget(m_deleteAnimalBtn);
    formLayout->addRow(btnRow);

    scrollArea->setWidget(formContainer);
    layout->addWidget(scrollArea, 1);

    return tab;
}

QWidget* GrangerPanel::createBreedersTab() {
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    m_breederList = new QListWidget(tab);
    connect(m_breederList, &QListWidget::currentRowChanged, this, &GrangerPanel::onBreederSelected);
    layout->addWidget(m_breederList, 1);

    auto* formContainer = new QWidget(tab);
    auto* formLayout = new QFormLayout(formContainer);
    formLayout->setContentsMargins(4, 4, 4, 4);
    formLayout->setSpacing(6);

    m_breederNameEdit = new QLineEdit(formContainer);
    formLayout->addRow("Player Name:", m_breederNameEdit);

    m_breederAhSpin = new QSpinBox(formContainer);
    m_breederAhSpin->setRange(0, 100);
    formLayout->addRow("AH Skill (0-100):", m_breederAhSpin);

    m_breederExtraSpin = new QSpinBox(formContainer);
    m_breederExtraSpin->setRange(0, 50);
    formLayout->addRow("Extra Care Slots:", m_breederExtraSpin);

    auto* btnRow = new QHBoxLayout();
    auto* newBtn = new QPushButton("➕ New", formContainer);
    connect(newBtn, &QPushButton::clicked, this, &GrangerPanel::onNewBreederClicked);

    m_saveBreederBtn = new QPushButton("💾 Save", formContainer);
    m_saveBreederBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    connect(m_saveBreederBtn, &QPushButton::clicked, this, &GrangerPanel::onSaveBreederClicked);

    m_deleteBreederBtn = new QPushButton("🗑 Delete", formContainer);
    m_deleteBreederBtn->setStyleSheet(QString("color: %1; font-weight: bold;").arg(theme::STATUS_DANGER));
    m_deleteBreederBtn->setEnabled(false);
    connect(m_deleteBreederBtn, &QPushButton::clicked, this, &GrangerPanel::onDeleteBreederClicked);

    btnRow->addWidget(newBtn);
    btnRow->addWidget(m_saveBreederBtn);
    btnRow->addWidget(m_deleteBreederBtn);
    formLayout->addRow(btnRow);

    layout->addWidget(formContainer);
    return tab;
}

QWidget* GrangerPanel::createBreedingTab() {
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    m_breedingSireCombo = new QComboBox(tab);
    m_breedingDamCombo = new QComboBox(tab);
    formLayout->addRow("Sire (Father):", m_breedingSireCombo);
    formLayout->addRow("Dam (Mother):", m_breedingDamCombo);

    auto* evalBtn = new QPushButton("⚡ Evaluate Breeding Pair", tab);
    evalBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    connect(evalBtn, &QPushButton::clicked, this, &GrangerPanel::onEvaluateBreedingClicked);

    layout->addLayout(formLayout);
    layout->addWidget(evalBtn);

    m_breedingStatusLabel = new QLabel(tab);
    m_breedingStatusLabel->setAlignment(Qt::AlignCenter);
    m_breedingStatusLabel->setStyleSheet("padding: 8px; font-size: 13px; font-weight: bold; border-radius: 6px;");
    m_breedingStatusLabel->setText("Select two animals to evaluate compatibility");
    layout->addWidget(m_breedingStatusLabel);

    auto* scrollArea = new QScrollArea(tab);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_breedingDetailsLabel = new QLabel(scrollArea);
    m_breedingDetailsLabel->setWordWrap(true);
    m_breedingDetailsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea->setWidget(m_breedingDetailsLabel);

    layout->addWidget(scrollArea, 1);
    return tab;
}

void GrangerPanel::refreshAll() {
    populateAnimalList();
    populateBreederList();
    updateParentDropdowns();
    updateCaredByDropdown();
    updateBreedingDropdowns();
}

void GrangerPanel::populateAnimalList() {
    m_animalList->clear();
    QString filter = m_animalSearchEdit->text().trimmed().toLower();

    for (const auto& a : m_store->getAnimals()) {
        QString name = QString::fromStdString(a.name);
        QString type = QString::fromStdString(a.type);
        QString traitsStr;
        for (const auto& tr : a.traits) {
            traitsStr += QString::fromStdString(tr) + " ";
        }

        if (!filter.isEmpty()) {
            if (!name.toLower().contains(filter) && !type.toLower().contains(filter) && !traitsStr.toLower().contains(filter)) {
                continue;
            }
        }

        QString itemText = QString("%1 (%2) [ID: %3]\nTraits: %4")
            .arg(name)
            .arg(type)
            .arg(a.id)
            .arg(a.traits.empty() ? "None" : QString::number(a.traits.size()) + " traits");

        auto* item = new QListWidgetItem(itemText, m_animalList);
        item->setData(Qt::UserRole, a.id);
    }
}

void GrangerPanel::populateBreederList() {
    m_breederList->clear();
    for (const auto& p : m_store->getPlayers()) {
        int cared = m_store->getCaredAnimalCount(p.id);
        int rem = m_store->getRemainingSlots(p.id);
        QString itemText = QString("%1 (AH: %2, Slots: %3/%4, Left: %5)")
            .arg(QString::fromStdString(p.name))
            .arg(p.ah_skill)
            .arg(cared)
            .arg(p.maxSlots())
            .arg(rem);

        auto* item = new QListWidgetItem(itemText, m_breederList);
        item->setData(Qt::UserRole, p.id);
    }
}

void GrangerPanel::updateParentDropdowns() {
    m_animalMotherCombo->clear();
    m_animalFatherCombo->clear();
    m_animalMotherCombo->addItem("None (0)", 0);
    m_animalFatherCombo->addItem("None (0)", 0);

    for (const auto& a : m_store->getAnimals()) {
        QString text = QString("%1: %2 (%3)").arg(a.id).arg(QString::fromStdString(a.name)).arg(QString::fromStdString(a.type));
        m_animalMotherCombo->addItem(text, a.id);
        m_animalFatherCombo->addItem(text, a.id);
    }
}

void GrangerPanel::updateCaredByDropdown() {
    m_animalCaredByCombo->clear();
    m_animalCaredByCombo->addItem("None (0)", 0);
    for (const auto& p : m_store->getPlayers()) {
        int rem = m_store->getRemainingSlots(p.id);
        QString text = QString("%1: %2 (Slots left: %3)").arg(p.id).arg(QString::fromStdString(p.name)).arg(rem);
        m_animalCaredByCombo->addItem(text, p.id);
    }
}

void GrangerPanel::updateBreedingDropdowns() {
    m_breedingSireCombo->clear();
    m_breedingDamCombo->clear();
    for (const auto& a : m_store->getAnimals()) {
        QString text = QString("%1: %2 (%3)").arg(a.id).arg(QString::fromStdString(a.name)).arg(QString::fromStdString(a.type));
        m_breedingSireCombo->addItem(text, a.id);
        m_breedingDamCombo->addItem(text, a.id);
    }
}

void GrangerPanel::onAnimalSelected(int row) {
    if (row < 0 || row >= m_animalList->count()) return;
    int id = m_animalList->item(row)->data(Qt::UserRole).toInt();
    const auto* a = m_store->getAnimal(id);
    if (!a) return;

    m_selectedAnimalId = a->id;
    m_animalNameEdit->setText(QString::fromStdString(a->name));
    m_animalTypeEdit->setText(QString::fromStdString(a->type));
    m_animalNotesEdit->setText(QString::fromStdString(a->notes));

    int motherIdx = m_animalMotherCombo->findData(a->mother_id);
    if (motherIdx >= 0) m_animalMotherCombo->setCurrentIndex(motherIdx);

    int fatherIdx = m_animalFatherCombo->findData(a->father_id);
    if (fatherIdx >= 0) m_animalFatherCombo->setCurrentIndex(fatherIdx);

    int caredIdx = m_animalCaredByCombo->findData(a->cared_by);
    if (caredIdx >= 0) m_animalCaredByCombo->setCurrentIndex(caredIdx);

    m_currentAnimalTraits = a->traits;
    m_traitsSummaryLabel->setText(QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(m_currentAnimalTraits)));
    m_deleteAnimalBtn->setEnabled(true);
}

void GrangerPanel::onAnimalSearchChanged(const QString&) {
    populateAnimalList();
}

void GrangerPanel::onNewAnimalClicked() {
    m_selectedAnimalId = 0;
    m_animalNameEdit->clear();
    m_animalTypeEdit->setText("horse");
    m_animalNotesEdit->clear();
    m_animalMotherCombo->setCurrentIndex(0);
    m_animalFatherCombo->setCurrentIndex(0);
    m_animalCaredByCombo->setCurrentIndex(0);
    m_currentAnimalTraits.clear();
    m_traitsSummaryLabel->setText(QString("<span style='color: %1; font-style: italic;'>None</span>").arg(theme::TEXT_SECONDARY));
    m_deleteAnimalBtn->setEnabled(false);
}

void GrangerPanel::onSaveAnimalClicked() {
    QString name = m_animalNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Animal name cannot be empty.");
        return;
    }

    Animal a;
    a.id = m_selectedAnimalId;
    a.name = name.toStdString();
    a.type = m_animalTypeEdit->text().trimmed().toStdString();
    if (a.type.empty()) a.type = "horse";
    a.mother_id = m_animalMotherCombo->currentData().toInt();
    a.father_id = m_animalFatherCombo->currentData().toInt();
    a.cared_by = m_animalCaredByCombo->currentData().toInt();
    a.traits = m_currentAnimalTraits;
    a.notes = m_animalNotesEdit->text().trimmed().toStdString();

    if (m_selectedAnimalId > 0) {
        m_store->updateAnimal(a);
    } else {
        m_store->addAnimal(a);
    }
    m_store->save();
    refreshAll();
}

void GrangerPanel::onDeleteAnimalClicked() {
    if (m_selectedAnimalId <= 0) return;
    if (QMessageBox::question(this, "Confirm", "Delete selected animal?") == QMessageBox::Yes) {
        m_store->deleteAnimal(m_selectedAnimalId);
        m_store->save();
        onNewAnimalClicked();
        refreshAll();
    }
}

void GrangerPanel::onEditTraitsClicked() {
    TraitSelectionDialog dlg(m_currentAnimalTraits, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_currentAnimalTraits = dlg.getSelectedTraits();
        m_traitsSummaryLabel->setText(QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(m_currentAnimalTraits)));
    }
}

void GrangerPanel::onBreederSelected(int row) {
    if (row < 0 || row >= m_breederList->count()) return;
    int id = m_breederList->item(row)->data(Qt::UserRole).toInt();
    const auto* p = m_store->getPlayer(id);
    if (!p) return;

    m_selectedBreederId = p->id;
    m_breederNameEdit->setText(QString::fromStdString(p->name));
    m_breederAhSpin->setValue(p->ah_skill);
    m_breederExtraSpin->setValue(p->extra_slots);
    m_deleteBreederBtn->setEnabled(true);
}

void GrangerPanel::onNewBreederClicked() {
    m_selectedBreederId = 0;
    m_breederNameEdit->clear();
    m_breederAhSpin->setValue(0);
    m_breederExtraSpin->setValue(0);
    m_deleteBreederBtn->setEnabled(false);
}

void GrangerPanel::onSaveBreederClicked() {
    QString name = m_breederNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Breeder name cannot be empty.");
        return;
    }

    Player p;
    p.id = m_selectedBreederId;
    p.name = name.toStdString();
    p.ah_skill = m_breederAhSpin->value();
    p.extra_slots = m_breederExtraSpin->value();

    if (m_selectedBreederId > 0) {
        m_store->updatePlayer(p);
    } else {
        m_store->addPlayer(p);
    }
    m_store->save();
    refreshAll();
}

void GrangerPanel::onDeleteBreederClicked() {
    if (m_selectedBreederId <= 0) return;
    if (QMessageBox::question(this, "Confirm", "Delete selected breeder? Associated animals will have cared_by reset.") == QMessageBox::Yes) {
        m_store->deletePlayer(m_selectedBreederId);
        m_store->save();
        onNewBreederClicked();
        refreshAll();
    }
}

void GrangerPanel::onEvaluateBreedingClicked() {
    int sireId = m_breedingSireCombo->currentData().toInt();
    int damId = m_breedingDamCombo->currentData().toInt();

    const auto* sire = m_store->getAnimal(sireId);
    const auto* dam = m_store->getAnimal(damId);

    if (!sire || !dam) {
        m_breedingStatusLabel->setText("Please select both a Sire and Dam.");
        m_breedingStatusLabel->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 8px;").arg(theme::SURFACE_CARD, theme::TEXT_SECONDARY, theme::BORDER_MUTED));
        m_breedingDetailsLabel->clear();
        return;
    }

    auto res = BreedingEvaluator::evaluate(*sire, *dam);
    if (res.isCompatible()) {
        m_breedingStatusLabel->setText("✔ COMPATIBLE PAIR");
        m_breedingStatusLabel->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 6px; padding: 8px;").arg(theme::STATUS_SUCCESS, theme::TEXT_ON_ACCENT));
    } else {
        m_breedingStatusLabel->setText(QString("✖ INCOMPATIBLE: %1").arg(QString::fromStdString(res.message)));
        m_breedingStatusLabel->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 6px; padding: 8px;").arg(theme::STATUS_DANGER, theme::TEXT_PRIMARY));
    }

    QString details = "<b>Shared Traits:</b><br>" + QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(res.shared_traits)) + "<br><br>";
    details += "<b>Sire Unique Traits:</b><br>" + QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(res.parent1_only_traits)) + "<br><br>";
    details += "<b>Dam Unique Traits:</b><br>" + QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(res.parent2_only_traits)) + "<br><br>";
    if (!res.rare_traits_in_pool.empty()) {
        details += "<b>★ Rare Traits in Pool:</b><br>" + QString::fromStdString(GrangerTraits::getFormattedTraitsHtml(res.rare_traits_in_pool)) + "<br><br>";
    }
    m_breedingDetailsLabel->setText(details);
}

} // namespace granger
