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

}  // namespace FluentTheme
