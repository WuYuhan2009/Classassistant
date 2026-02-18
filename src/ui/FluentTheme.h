#pragma once

#include <QString>

class QWidget;
class QDialog;

namespace FluentTheme {

QString sidebarPanelStyle();
QString sidebarButtonStyle();
QString trayMenuStyle();
QString dialogPrimaryButtonStyle();
QString dialogCardStyle();
QString dialogChromeStyle();

void applyWinUIWindowShadow(QWidget* widget);
void decorateDialog(QDialog* dialog, const QString& title);
void enableTouchOptimizations(QWidget* root);

QString getStyledOpenFileName(QWidget* parent,
                              const QString& title,
                              const QString& initialPath = QString(),
                              const QString& filter = QString());

QString getStyledSaveFileName(QWidget* parent,
                              const QString& title,
                              const QString& initialPath = QString(),
                              const QString& filter = QString());

}  // namespace FluentTheme
