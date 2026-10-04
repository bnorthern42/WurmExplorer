#pragma once

#include <QDialog>
#include <vector>
#include <string>
#include <map>

class QCheckBox;

namespace granger {

class TraitSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit TraitSelectionDialog(const std::vector<std::string>& initialTraits, QWidget* parent = nullptr);
    ~TraitSelectionDialog() override = default;

    std::vector<std::string> getSelectedTraits() const;

private:
    void setupUi();

    std::vector<std::string> m_initialTraits;
    std::map<std::string, QCheckBox*> m_traitCheckboxes;
};

} // namespace granger
