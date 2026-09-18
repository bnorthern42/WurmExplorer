#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <memory>
#include "../../models/AnnotationStore.hpp"
#include "../../core/Config.hpp"

namespace treasure {
namespace ui {

class AnnotationPanel : public QWidget {
    Q_OBJECT
public:
    explicit AnnotationPanel(std::shared_ptr<models::AnnotationStore> store, QWidget* parent = nullptr);
    
    void setContext(std::shared_ptr<core::ServerConfig> cfg);
    void refreshList();
    
    const std::vector<models::Annotation>& getVisibleAnnotations() const { return listboxAnnots; }

signals:
    void dataChanged();
    void requestFocus(float x, float y);

private slots:
    void onToolChange();
    void onSearchFilterChanged();
    void onListSelectionChanged();
    void onKingdomChanged();
    
    void saveCurrent();
    void undoPoint();
    void resetEditor();
    void deleteCurrent();

private:
    std::shared_ptr<models::AnnotationStore> annotStore;
    std::shared_ptr<core::ServerConfig> currentCfg;
    
    std::vector<models::Annotation> listboxAnnots;
    std::string currentAnnotId;
    std::vector<models::Point> pendingPoints;
    std::string draftType;

    // UI Elements
    QPushButton* panBtn;
    QPushButton* deedBtn;
    QPushButton* roadBtn;
    QPushButton* bridgeBtn;
    QPushButton* tunnelBtn;
    QPushButton* guardBtn;
    
    QRadioButton* lineDrawBtn;
    QRadioButton* brushDrawBtn;
    
    QButtonGroup* toolGroup;
    QButtonGroup* drawGroup;

    QLineEdit* searchEdit;
    QComboBox* filterCb;
    QListWidget* annotList;
    QLabel* countLabel;

    QLabel* selectionLabel;
    QLabel* posLabel;
    QLineEdit* nameEdit;
    
    QWidget* kingdomRow;
    QComboBox* kingdomCb;
    
    QWidget* influenceRow;
    QSpinBox* influenceSpin;
    
    QCheckBox* statusCb;
    QTextEdit* notesEdit;
    
    QPushButton* saveBtn;
    QPushButton* undoBtn;
    QPushButton* newBtn;
    QPushButton* deleteBtn;
    
    void createToolGroup();
    void createBrowserGroup();
    void createEditorGroup();
    
    QPushButton* createToolButton(const QString& text, const QString& typeData, bool checked = false);
    
    void updateEditorUI();
};

} // namespace ui
} // namespace treasure
