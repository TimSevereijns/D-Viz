#include "Windows/aboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog{ parent }, m_ui{}
{
    m_ui.setupUi(this);

    const auto timeStamp = std::string{ __DATE__ } + ", " + std::string{ __TIME__ }; // NOLINT
    m_ui.timeStampLabel->setText(QString::fromStdString(timeStamp));
}
