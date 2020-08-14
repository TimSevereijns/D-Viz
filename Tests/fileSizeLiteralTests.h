#ifndef FILESIZELITERALS_HPP
#define FILESIZELITERALS_HPP

#include <QtTest>

#include "Utilities/multiTestHarness.h"

class FileSizeLiteralTests : public QObject
{
    Q_OBJECT

  private slots:

    /**
     * @brief This preamble is run only once for the entire class. All setup work should be done
     * here.
     */
    void initTestCase();

    /**
     * @brief Clean up code for the entire class; called once.
     */
    void cleanupTestCase();

    /**
     * @brief This preamble is run before each test.
     */
    void init();

    /**
     * @brief Verifes kilobytes are correctly computed.
     */
    void KilobytesToBytes() const;

    /**
     * @brief Verifes megabytes are correctly computed.
     */
    void MegabytesToBytes() const;

    /**
     * @brief Verifes gigabytes are correctly computed.
     */
    void GigabytesToBytes() const;

    /**
     * @brief Verifes terabytes are correctly computed.
     */
    void TerabytesToBytes() const;

    /**
     * @brief Verifes kibibytes are correctly computed.
     */
    void KibibytesToBytes() const;

    /**
     * @brief Verifes kibibytes are correctly computed.
     */
    void MebibytesToBytes() const;

    /**
     * @brief Verifes gibibytes are correctly computed.
     */
    void GibibytesToBytes() const;

    /**
     * @brief Verifes tebibytes are correctly computed.
     */
    void TebibytesToBytes() const;
};

#endif // FILESIZELITERALS_HPP
