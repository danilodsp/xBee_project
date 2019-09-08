#ifndef COMUNICACAO_H
#define COMUNICACAO_H

#define READ_TIMEOUT      500      // milliseconds
#define AMOUNT_TO_READ    512

#include <QThread>
#include <windows.h>
#include <QString>

class comunicacao : public QThread
{
    Q_OBJECT

public:
    void run();
    bool lerArquivo();
    void leituraCompleta(char * lpBuf, DWORD dwRead);
    void conclusao(DWORD dwRead, LPVOID lpBuf, BOOL fWaitingOnRead, OVERLAPPED osReader);
    bool escreverArquivo(char * lpBuf, DWORD dwToWrite);
    //comunicacao();

public:
    QString porta;
    HANDLE hComm;
    char lpBuf[AMOUNT_TO_READ];
    QString texto;

signals:
    void textoConsole();
    void alertaPorta();

};

#endif // COMUNICACAO_H
