#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gronsfeld.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QComboBox>

class MainWindow : public QWidget
{
    Q_OBJECT

    QLabel* keyLabel = nullptr;
    QLabel* inputLabel = nullptr;
    QLabel* outputLabel = nullptr;
    QLabel* filenameLabel = nullptr;
    QLabel* alphabetLabel = nullptr;
    QComboBox* alphabetBox = nullptr;
    QLabel* modeLabel = nullptr;
    QComboBox* modeBox = nullptr;
    QLineEdit* keyEdit = nullptr;
    QLineEdit* filenameInEdit = nullptr;
    QLineEdit* filenameOutEdit = nullptr;
    QPlainTextEdit* inputText = nullptr;
    QPlainTextEdit* outputText = nullptr;
    QPushButton* encryptButton = nullptr;
    QPushButton* decryptButton = nullptr;
    QPushButton* encryptFileButton = nullptr;
    QPushButton* decryptFileButton = nullptr;

    gronsfeld::Alphabet getAlphabet ();

    void crypt(void (*func) (const gronsfeld::Alphabet&, const unsigned char*, std::size_t, char*, std::size_t, gronsfeld::Mode::Mode));
    void fcrypt(void (*func) (const gronsfeld::Alphabet&, const unsigned char*, std::size_t, std::fstream&, std::fstream&, gronsfeld::Mode::Mode));

public:

    void clear ();
    void mainScreen ();
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void encrypt();
    void decrypt();

    void fencrypt();
    void fdecrypt();
};

#endif // MAINWINDOW_H
