#include "View/Dialogs/aboutDialog.h"

#include "constants.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog{ parent }, m_ui{}
{
    m_ui.setupUi(this);

    const auto timeStamp = QString{ __DATE__ } + ", " + QString{ __TIME__ };
    m_ui.timestamp->setText("Built on " + timeStamp + ".");

    m_ui.version->setText(
        "<html><head/><body><p><span style=\" font-size:10pt; font-weight:600;\">Version " +
        Constants::Version.toString() + "</span></p></body></html>");
}
