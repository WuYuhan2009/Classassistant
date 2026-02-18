#include "FluentTheme.h"

#include <QAbstractScrollArea>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QScroller>
#include <QWidget>

namespace FluentTheme {


namespace {
void styleFileDialog(QFileDialog& dialog) {
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setWindowFlag(Qt::FramelessWindowHint, true);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet(FluentTheme::dialogChromeStyle() +
                         QString("QFileDialog QPushButton{min-height:36px;padding:8px 12px;border:1px solid #cfdcec;background:#ffffff;border-radius:12px;}"
                                 "QFileDialog QPushButton:hover{background:#edf5ff;border-color:#9fbde1;}"
                                 "QFileDialog QListView,QFileDialog QTreeView{border:1px solid #d3dfef;border-radius:12px;background:#ffffff;}"
                                 "QFileDialog QLineEdit{min-height:34px;}"));
    FluentTheme::applyWinUIWindowShadow(&dialog);
    FluentTheme::enableTouchOptimizations(&dialog);
}
}

QString sidebarPanelStyle() {
    return "QWidget {"
           "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(248,251,255,248),stop:1 rgba(242,247,255,246));"
           "border:1px solid rgba(192,210,232,190);"
           "border-top-left-radius:22px;border-bottom-left-radius:22px;"
           "}";
}

QString sidebarButtonStyle() {
    return "QPushButton{"
           "background:rgba(255,255,255,0.92);"
           "border:1px solid #d3deec;"
           "border-radius:16px;"
           "font-size:20px;"
           "padding:6px;"
           "}"
           "QPushButton:hover{background:#f1f7ff;border-color:#abc4e6;}"
           "QPushButton:pressed{background:#e3eefc;border-color:#90afd6;}"
           "QPushButton:focus{border:2px solid #7ca7df;}";
}

QString trayMenuStyle() {
    return "QMenu{"
           "background:#f8fbff;"
           "border:1px solid #cfdbeb;"
           "border-radius:14px;"
           "padding:8px;"
           "}"
           "QMenu::item{font-size:14px;color:#1f3248;padding:10px 14px;border-radius:10px;margin:3px 2px;min-height:34px;}"
           "QMenu::item:selected{background:#e8f2ff;color:#16457d;}"
           "QMenu::separator{height:1px;background:#d6e2f1;margin:8px 4px;}";
}

QString dialogPrimaryButtonStyle() {
    return "QPushButton{background:#ffffff;border:1px solid #d8e0eb;border-radius:14px;font-weight:600;font-size:14px;padding:8px 12px;color:#1f2d3d;min-height:40px;}"
           "QPushButton:hover{background:#f4f8fd;}"
           "QPushButton:pressed{background:#e8f0fb;}";
}

QString dialogCardStyle() {
    return "background:#ffffff;border:1px solid #dfe5ee;border-radius:14px;";
}

QString dialogChromeStyle() {
    return "QDialog{"
           "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #f8fbff,stop:1 #f2f7ff);"
           "border:1px solid #d8e0eb;border-radius:16px;"
           "}"
           "QLabel{color:#223042;}"
           "QLineEdit,QTextEdit,QListWidget,QTreeWidget,QComboBox,QSpinBox,QTableWidget,QPlainTextEdit{"
           "background:#ffffff;border:1px solid #d3dfef;border-radius:16px;padding:8px;}"
           "QTreeWidget::item{height:30px;border-radius:10px;}"
           "QTreeWidget::item:selected{background:#e9f2ff;color:#1f4f8f;}"
           "QCheckBox{spacing:8px;}"
           "QSlider::groove:horizontal{height:6px;background:#dbe4ef;border-radius:3px;}"
           "QSlider::handle:horizontal{width:16px;margin:-5px 0;background:#ffffff;border:1px solid #9cb2ce;border-radius:8px;}"
           "QGroupBox{font-weight:700;border:1px solid #dfe5ee;border-radius:14px;margin-top:10px;padding-top:12px;background:#ffffff;}"
           "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 6px;}"
           "QScrollBar:vertical{background:transparent;width:10px;margin:2px;border-radius:5px;}"
           "QScrollBar::handle:vertical{background:#c8d8ec;min-height:20px;border-radius:5px;}"
           "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
           "QPushButton{border-radius:14px;}"
           "QSpinBox::up-button,QSpinBox::down-button{width:22px;border-left:1px solid #d1deef;background:#f5f9ff;border-radius:8px;margin:2px;}"
           "QSpinBox::up-button:hover,QSpinBox::down-button:hover{background:#e8f2ff;}"
           "QSpinBox::up-arrow{image:none;width:0;height:0;border-left:5px solid transparent;border-right:5px solid transparent;border-bottom:7px solid #4f6f94;}"
           "QSpinBox::down-arrow{image:none;width:0;height:0;border-left:5px solid transparent;border-right:5px solid transparent;border-top:7px solid #4f6f94;}"
           "QListWidget::item,QTreeWidget::item{border-radius:10px;padding:6px;}"
           "QFrame#DialogTitleBar{background:#ffffff;border:1px solid #d8e0eb;border-radius:14px;}"
           "QLabel#DialogTitleText{font-size:15px;font-weight:800;color:#1f3b5d;}"
           "QPushButton#DialogCloseBtn{font-size:15px;min-width:30px;max-width:30px;min-height:30px;max-height:30px;padding:0;border-radius:10px;}";
}

void applyWinUIWindowShadow(QWidget* widget) {
    if (!widget) {
        return;
    }
    auto* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(52, 84, 120, 80));
    widget->setGraphicsEffect(shadow);
}


void enableTouchOptimizations(QWidget* root) {
    if (!root) {
        return;
    }

    root->setAttribute(Qt::WA_AcceptTouchEvents, true);

    const auto scrollables = root->findChildren<QAbstractScrollArea*>();
    for (QAbstractScrollArea* area : scrollables) {
        if (!area || !area->viewport()) {
            continue;
        }
        area->setAttribute(Qt::WA_AcceptTouchEvents, true);
        area->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
        QScroller::grabGesture(area->viewport(), QScroller::LeftMouseButtonGesture);
    }
}

void decorateDialog(QDialog* dialog, const QString& title) {
    if (!dialog) {
        return;
    }
    dialog->setWindowTitle(title);
    dialog->setWindowFlags((dialog->windowFlags() | Qt::Tool | Qt::FramelessWindowHint) & ~Qt::WindowContextHelpButtonHint);
    dialog->setAttribute(Qt::WA_AcceptTouchEvents);
    dialog->setAttribute(Qt::WA_StyledBackground, true);
    dialog->setStyleSheet(dialogChromeStyle());
    applyWinUIWindowShadow(dialog);
    enableTouchOptimizations(dialog);
}

QString getStyledOpenFileName(QWidget* parent,
                              const QString& title,
                              const QString& initialPath,
                              const QString& filter) {
    QFileDialog dialog(parent, title, initialPath, filter);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    styleFileDialog(dialog);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return QString();
    }
    return dialog.selectedFiles().first();
}

QString getStyledSaveFileName(QWidget* parent,
                              const QString& title,
                              const QString& initialPath,
                              const QString& filter) {
    QFileDialog dialog(parent, title, initialPath, filter);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    styleFileDialog(dialog);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return QString();
    }
    return dialog.selectedFiles().first();
}

}  // namespace FluentTheme
