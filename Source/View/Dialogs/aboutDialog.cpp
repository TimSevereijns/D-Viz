#include "View/Dialogs/aboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog{ parent }, m_ui{}
{
    m_ui.setupUi(this);

    const auto timeStamp = QString{ __DATE__ } + ", " + QString{ __TIME__ };
    m_ui.timestamp->setText("Built on " + timeStamp + ".");
}
