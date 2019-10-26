#include "Windows/aboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog{ parent }, m_ui{}
{
    m_ui.setupUi(this);

    const auto timeStamp = QString{ __DATE__ } + ", " + QString{ __TIME__ }; // NOLINT
    m_ui.timeStampLabel->setText(timeStamp);
}
