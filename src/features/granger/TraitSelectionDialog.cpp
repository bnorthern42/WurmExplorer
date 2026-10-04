#include "TraitSelectionDialog.hpp"
#include "GrangerTraits.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <algorithm>

namespace granger {

TraitSelectionDialog::TraitSelectionDialog(const std::vector<std::string>& initialTraits, QWidget* parent)
    : QDialog(parent), m_initialTraits(initialTraits) {
    setWindowTitle("Select Animal Traits");
    resize(640, 580);
    setupUi();
}

void TraitSelectionDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* container = new QWidget(scrollArea);
    auto* gridLayout = new QGridLayout(container);
    gridLayout->setSpacing(12);

    const std::vector<TraitCategory> categories = {
        TraitCategory::COMBAT,
        TraitCategory::SPEED,
        TraitCategory::DRAFT,
        TraitCategory::OUTPUT,
        TraitCategory::MISC,
        TraitCategory::NEGATIVE
    };

    int row = 0;
    int col = 0;
    for (const auto cat : categories) {
        auto* group = new QGroupBox(QString::fromStdString(GrangerTraits::getCategoryName(cat)), container);
        auto* groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(8, 8, 8, 8);
        groupLayout->setSpacing(4);

        auto traits = GrangerTraits::getTraitsByCategory(cat);
        for (const auto& t : traits) {
            bool rare = GrangerTraits::isRare(t.internal_name);
            QString labelText = QString::fromStdString(t.display_name);
            if (rare) {
                labelText = "★ " + labelText;
            }

            auto* cb = new QCheckBox(labelText, group);
            cb->setToolTip(QString::fromStdString(t.description));

            if (rare) {
                cb->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; }").arg(treasure::ui::theme::ACCENT_MINT));
            } else if (cat == TraitCategory::NEGATIVE) {
                cb->setStyleSheet(QString("QCheckBox { color: %1; }").arg(treasure::ui::theme::STATUS_DANGER));
            }

            bool isChecked = std::find(m_initialTraits.begin(), m_initialTraits.end(), t.internal_name) != m_initialTraits.end();
            cb->setChecked(isChecked);

            m_traitCheckboxes[t.internal_name] = cb;
            groupLayout->addWidget(cb);
        }

        gridLayout->addWidget(group, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea, 1);

    // Bottom action row
    auto* actionRow = new QHBoxLayout();
    auto* clearBtn = new QPushButton("Clear All", this);
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        for (auto& [name, cb] : m_traitCheckboxes) {
            cb->setChecked(false);
        }
    });

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    actionRow->addWidget(clearBtn);
    actionRow->addStretch(1);
    actionRow->addWidget(buttonBox);

    mainLayout->addLayout(actionRow);
}

std::vector<std::string> TraitSelectionDialog::getSelectedTraits() const {
    std::vector<std::string> result;
    for (const auto& [name, cb] : m_traitCheckboxes) {
        if (cb->isChecked()) {
            result.push_back(name);
        }
    }
    return result;
}

} // namespace granger
