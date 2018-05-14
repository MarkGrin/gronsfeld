#include "mainwindow.h"
#include "gronsfeld.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QDebug>

#include <memory>
namespace {
void deleteAndNull() {
}
template<typename T, typename ...Args>
void deleteAndNull(T&& obj, Args ...objects) {
    delete obj;
    obj = nullptr;
    deleteAndNull(objects...);
}

} // namespace
void MainWindow::clear()
{
    delete layout();

    deleteAndNull(&keyLabel, &outputLabel,&encryptButton,
                  &decryptButton, &inputText, &outputText,
                  &inputLabel, &outputLabel, &filenameLabel,
                  &filenameInEdit, &filenameOutEdit, &encryptFileButton,
                  &decryptFileButton, &alphabetLabel, &alphabetBox);

}

void MainWindow::mainScreen()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* keyLayout = new QHBoxLayout;
    QHBoxLayout* alphabetLayout = new QHBoxLayout;
    QHBoxLayout* inputLayout = new QHBoxLayout;
    QHBoxLayout* outputLayout = new QHBoxLayout;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QHBoxLayout* fileLayout   = new QHBoxLayout;
    QHBoxLayout* fileButtonLayout = new QHBoxLayout;
    QHBoxLayout* modeLayout = new QHBoxLayout;

    keyLayout->addWidget(keyLabel = new QLabel("Key:"));
    keyLayout->addWidget(keyEdit = new QLineEdit(""));

    alphabetLayout->addWidget(alphabetLabel = new QLabel ("Alphabet:"));
    alphabetLayout->addWidget(alphabetBox = new QComboBox);
    alphabetBox->addItem("latin");
    alphabetBox->addItem("extended latin");
    alphabetBox->addItem("complete");

    inputLayout->addWidget(inputLabel = new QLabel("Input:"));
    inputLayout->addWidget(inputText = new QPlainTextEdit(""));

    outputLayout->addWidget(outputLabel = new QLabel("Output:"));
    outputLayout->addWidget(outputText = new QPlainTextEdit(""));
    outputText->setReadOnly(true);

    buttonLayout->addWidget(encryptButton = new QPushButton("Encrypt"));
    buttonLayout->addWidget(decryptButton = new QPushButton("Decrypt"));
    QObject::connect (encryptButton, SIGNAL(clicked()), this, SLOT(encrypt()));
    QObject::connect (decryptButton, SIGNAL(clicked()), this, SLOT(decrypt()));

    fileLayout->addWidget(filenameLabel = new QLabel("Filename in/out:"));
    fileLayout->addWidget(filenameInEdit = new QLineEdit(""));
    fileLayout->addWidget(filenameOutEdit = new QLineEdit(""));

    fileButtonLayout->addWidget(encryptFileButton = new QPushButton("Encrypt File"));
    fileButtonLayout->addWidget(decryptFileButton = new QPushButton("Decrypt File"));
    QObject::connect (encryptFileButton, SIGNAL(clicked()), this, SLOT(fencrypt()));
    QObject::connect (decryptFileButton, SIGNAL(clicked()), this, SLOT(fdecrypt()));

    modeLayout->addWidget(modeLabel = new QLabel("Mode"));
    modeLayout->addWidget(modeBox = new QComboBox);
    modeBox->addItem("error");
    modeBox->addItem("move");
    modeBox->addItem("delete");

    mainLayout->addLayout(keyLayout);
    mainLayout->addLayout(alphabetLayout);
    mainLayout->addLayout(modeLayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(outputLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(fileLayout);
    mainLayout->addLayout(fileButtonLayout);

    setLayout(mainLayout);
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    mainScreen();
}

MainWindow::~MainWindow()
{
    clear();
}

gronsfeld::Alphabet MainWindow::getAlphabet()
{
    if (!alphabetBox)
        return gronsfeld::latinAlphabet();
    if (alphabetBox->currentText() == "latin")
        return gronsfeld::latinAlphabet();
    if (alphabetBox->currentText() == "extended latin")
        return gronsfeld::latinExtendedAlphabet();
    if (alphabetBox->currentText() == "complete")
        return gronsfeld::allAlphabet();

    return gronsfeld::allAlphabet();
}

void MainWindow::crypt(void (*func)(const gronsfeld::Alphabet &, const unsigned char *, std::size_t, char *, std::size_t, gronsfeld::Mode::Mode))
{
    try {
        std::string keyText = keyEdit->text().toStdString();
        for (auto& digit : keyText)
            if (!std::isdigit(digit))
                throw std::logic_error ("invalid key");

        std::shared_ptr<unsigned char> key (new unsigned char[keyText.size()], std::default_delete<unsigned char[]>());
        for (std::size_t i = 0; i < keyText.size(); i++)
            key.get()[i] = keyText[i] - '0';

        std::string input = inputText->toPlainText().toStdString();

        gronsfeld::Mode::Mode mode = gronsfeld::Mode::ERROR;
        if (modeBox->currentText() == "move")
            mode = gronsfeld::Mode::MOVE;
        else if (modeBox->currentText() == "delete")
            mode = gronsfeld::Mode::DELETE;

        std::shared_ptr<char> array(new char[input.size() + 1], std::default_delete<char[]>());
        memcpy(array.get(), input.data(), input.size() + 1);
        func(getAlphabet(), key.get(), keyText.size(), array.get(), input.size(), mode);
        for (std::size_t i = 0; i < input.size(); i++)
            if (!std::isprint(input[i]))
                throw std::logic_error("(" + std::to_string((unsigned char)input[i]) +") imprintable character in output");
        input = array.get();

        outputText->setPlainText(QString::fromStdString(input));

    }
    catch (std::logic_error& e) {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
    }
    catch (...) {
        QMessageBox msgBox;
        msgBox.setText("Unknown error");
        msgBox.exec();
    }
}
void MainWindow::fcrypt(void (*func) (const gronsfeld::Alphabet&, const unsigned char*, std::size_t, std::fstream&, std::fstream&, gronsfeld::Mode::Mode))
{
    try {
        std::string keyText = keyEdit->text().toStdString();
        for (auto& digit : keyText)
            if (!std::isdigit(digit))
                throw std::logic_error ("invalid key");

        std::shared_ptr<unsigned char> key (new unsigned char[keyText.size()], std::default_delete<unsigned char[]>());
        for (std::size_t i = 0; i < keyText.size(); i++)
            key.get()[i] = keyText[i] - '0';

        std::string filenameIn = filenameInEdit->text().toStdString();
        std::string filenameOut = filenameOutEdit->text().toStdString();

        qDebug () << QString::fromStdString(filenameIn) << QString::fromStdString(filenameOut) << "\n";
        std::fstream input  (filenameIn, std::ios::binary | std::ios::in);
        std::fstream output (filenameOut, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!input)
            throw std::logic_error ("input file opening error");
        if (!output)
            throw std::logic_error ("output file opening error");

        gronsfeld::Mode::Mode mode = gronsfeld::Mode::ERROR;
        if (modeBox->currentText() == "move")
            mode = gronsfeld::Mode::MOVE;
        else if (modeBox->currentText() == "delete")
            mode = gronsfeld::Mode::DELETE;
        func(getAlphabet(), key.get(), keyText.size(), input, output, mode);
    }
    catch (std::logic_error& e) {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
    }
    catch (...) {
        QMessageBox msgBox;
        msgBox.setText("Unknown error");
        msgBox.exec();
    }
    QMessageBox msgBox;
    msgBox.setText("Done");
    msgBox.exec();
}

void MainWindow::encrypt()
{
    crypt (gronsfeld::encrypt);
}

void MainWindow::decrypt()
{
    crypt (gronsfeld::decrypt);
}

void MainWindow::fencrypt()
{
    fcrypt (gronsfeld::encrypt);
}

void MainWindow::fdecrypt()
{
    fcrypt (gronsfeld::decrypt);
}
