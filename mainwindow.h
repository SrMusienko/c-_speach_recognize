// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QAudioSource>
#include <memory>
#include <vosk_api.h>

// Форвард-декларации заменяем на полные определения
struct VoskModel;
struct VoskRecognizer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleRecording();
    void modelSelected(const QString &path);
    void processAudioData();

private:
    void initializeUI();
    void initializeSpeechRecognition(const QString &modelPath);
    void stopRecording();
    void startRecording();

    QComboBox *modelSelector;
    QPushButton *recordButton;
    QTextEdit *recognizedText;
    
    VoskModel* model;
    VoskRecognizer* recognizer;
    std::unique_ptr<QAudioSource> audioSource;
    QIODevice* audioDevice;
    bool isRecording;
};

#endif // MAINWINDOW_H