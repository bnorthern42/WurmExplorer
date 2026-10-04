#pragma once

#include <QWidget>
#include <memory>
#include <vector>
#include <string>

class QTabWidget;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QLabel;
class QPushButton;

namespace granger {
class GrangerStore;

class GrangerPanel : public QWidget {
    Q_OBJECT

public:
    explicit GrangerPanel(std::shared_ptr<GrangerStore> store, QWidget* parent = nullptr);
    ~GrangerPanel() override = default;

    void refreshAll();

private slots:
    // Animals
    void onAnimalSelected(int row);
    void onAnimalSearchChanged(const QString& text);
    void onNewAnimalClicked();
    void onSaveAnimalClicked();
    void onDeleteAnimalClicked();
    void onEditTraitsClicked();

    // Breeders
    void onBreederSelected(int row);
    void onNewBreederClicked();
    void onSaveBreederClicked();
    void onDeleteBreederClicked();

    // Breeding pair
    void onEvaluateBreedingClicked();

private:
    void setupUi();
    QWidget* createAnimalsTab();
    QWidget* createBreedersTab();
    QWidget* createBreedingTab();

    void populateAnimalList();
    void populateBreederList();
    void updateParentDropdowns();
    void updateCaredByDropdown();
    void updateBreedingDropdowns();

    std::shared_ptr<GrangerStore> m_store;
    int m_selectedAnimalId = 0;
    int m_selectedBreederId = 0;
    std::vector<std::string> m_currentAnimalTraits;

    // Animal widgets
    QLineEdit* m_animalSearchEdit = nullptr;
    QListWidget* m_animalList = nullptr;
    QLineEdit* m_animalNameEdit = nullptr;
    QLineEdit* m_animalTypeEdit = nullptr;
    QComboBox* m_animalMotherCombo = nullptr;
    QComboBox* m_animalFatherCombo = nullptr;
    QComboBox* m_animalCaredByCombo = nullptr;
    QLabel* m_traitsSummaryLabel = nullptr;
    QLineEdit* m_animalNotesEdit = nullptr;
    QPushButton* m_saveAnimalBtn = nullptr;
    QPushButton* m_deleteAnimalBtn = nullptr;

    // Breeder widgets
    QListWidget* m_breederList = nullptr;
    QLineEdit* m_breederNameEdit = nullptr;
    QSpinBox* m_breederAhSpin = nullptr;
    QSpinBox* m_breederExtraSpin = nullptr;
    QPushButton* m_saveBreederBtn = nullptr;
    QPushButton* m_deleteBreederBtn = nullptr;

    // Breeding checker widgets
    QComboBox* m_breedingSireCombo = nullptr;
    QComboBox* m_breedingDamCombo = nullptr;
    QLabel* m_breedingStatusLabel = nullptr;
    QLabel* m_breedingDetailsLabel = nullptr;
};

} // namespace granger
