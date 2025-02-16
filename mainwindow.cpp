// mainwindow.cpp

#pragma comment(lib, "C:/llama/c++")
#include "vosk_api.h"

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDir>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QJsonDocument>
#include <QJsonObject>
#include <vosk_api.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
    isRecording(false), 
    audioDevice(nullptr),
    model(nullptr),
    recognizer(nullptr)
{
    initializeUI();
    
    // Scan for available models
    QDir modelsDir("C:/llama/models/");
    QStringList models = modelsDir.entryList(QStringList() << "vosk-model*", QDir::Dirs);
    modelSelector->addItems(models);
}

void MainWindow::initializeUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // Model selector
    modelSelector = new QComboBox(this);
    layout->addWidget(modelSelector);
    
    // Record button
    recordButton = new QPushButton("Start Recording", this);
    layout->addWidget(recordButton);
    
    // Text display
    recognizedText = new QTextEdit(this);
    recognizedText->setReadOnly(true);
    layout->addWidget(recognizedText);
    
    // Connect signals
    connect(modelSelector, &QComboBox::currentTextChanged, this, &MainWindow::modelSelected);
    connect(recordButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);
    
    setMinimumSize(400, 300);
}

void MainWindow::modelSelected(const QString &path)
{
    QString fullPath = "C:/llama/models/" + path;
    initializeSpeechRecognition(fullPath);
}

void MainWindow::initializeSpeechRecognition(const QString &modelPath)
{
    if (!QDir(modelPath).exists()) {
        recognizedText->append("Error: Model not found at " + modelPath);
        return;
    }
    
    // Очищаем предыдущую модель и распознаватель, если они существуют
    if (recognizer) {
        vosk_recognizer_free(recognizer);
        recognizer = nullptr;
    }
    if (model) {
        vosk_model_free(model);
        model = nullptr;
    }
    
    // Создаем новую модель и распознаватель
    model = vosk_model_new(modelPath.toStdString().c_str());
    if (model == nullptr) {
        recognizedText->append("Error: Could not load model");
        return;
    }
    
    recognizer = vosk_recognizer_new(model, 16000.0);
    if (recognizer == nullptr) {
        recognizedText->append("Error: Could not create recognizer");
        vosk_model_free(model);
        model = nullptr;
        return;
    }
}

void MainWindow::toggleRecording()
{
    if (!isRecording) {
        startRecording();
    } else {
        stopRecording();
    }
}

void MainWindow::startRecording()
{
    if (!model || !recognizer) {
        recognizedText->append("Error: Please select a model first");
        return;
    }
    
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    
    // Get default audio input device
    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isNull()) {
        format.setSampleRate(format.sampleRate());
        audioSource = std::make_unique<QAudioSource>(inputDevice, format);
        audioDevice = audioSource->start();
        connect(audioDevice, &QIODevice::readyRead, this, &MainWindow::processAudioData);
        
        isRecording = true;
        recordButton->setText("Stop Recording");
    } else {
        recognizedText->append("Error: No audio input device found");
    }
}

void MainWindow::stopRecording()
{
    if (audioSource) {
        audioSource->stop();
        audioDevice = nullptr;
    }
    isRecording = false;
    recordButton->setText("Start Recording");
}

void MainWindow::processAudioData()
{
    if (!audioDevice || !recognizer) return;
    
    QByteArray data = audioDevice->readAll();
    
    if (vosk_recognizer_accept_waveform(recognizer, data.data(), data.size())) {
        const char* result = vosk_recognizer_result(recognizer);
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(result));
        QJsonObject obj = doc.object();
        
        if (obj.contains("text")) {
            QString text = obj["text"].toString();
            if (!text.isEmpty()) {
                recognizedText->append(text);
            }
        }
    } else {
        const char* partial = vosk_recognizer_partial_result(recognizer);
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(partial));
        QJsonObject obj = doc.object();
        
        if (obj.contains("partial")) {
            QString text = obj["partial"].toString();
            if (!text.isEmpty()) {
                // Update last line with partial result
                QTextCursor cursor = recognizedText->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
                cursor.insertText("... " + text);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    stopRecording();
    
    if (recognizer) {
        vosk_recognizer_free(recognizer);
    }
    if (model) {
        vosk_model_free(model);
    }
}