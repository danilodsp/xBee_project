#include "comunicacao.h"

void comunicacao::run()
{
    if(!porta.startsWith("COM")){
        emit alertaPorta();
    }

    //HANDLE hComm;
    hComm = CreateFileA( porta.toAscii().constData(),
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         0,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         0);
    if (hComm == INVALID_HANDLE_VALUE)
       emit alertaPorta();

    /*DWORD size;
    size = GetFileSize(hComm,0);
    lpBuf = malloc(size);*/

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    SetCommState(hComm, &dcb);
}

bool comunicacao::lerArquivo()
{
    DWORD dwRead;
    DWORD dwRes;
    DWORD dwStoredFlags;
    //LPVOID lpBuf;
    BOOL fWaitingOnRead = FALSE;
    //BOOL fThreadDone = FALSE;
    OVERLAPPED osReader = {0};
    OVERLAPPED osStatus = {0};
    int erro;
/*
    DWORD size;
    size = GetFileSize(hComm,0);
    lpBuf = malloc(size);*/

    // Create the overlapped event. Must be closed before exiting
    // to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osReader.hEvent == NULL){
       // Error creating overlapped event; abort.
        texto = "Erro ao criar overlapped event.";
        emit textoConsole();
        return false;
    }

    osStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osStatus.hEvent == NULL){
       // Error creating overlapped event; abort.
        texto = "Erro ao criar overlapped event.";
        emit textoConsole();
        return false;
    }

    dwStoredFlags = EV_BREAK | EV_CTS   | EV_DSR | EV_ERR | EV_RING |\
                    EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY ;
    if (!SetCommMask(hComm, dwStoredFlags)){
        erro = GetLastError();
        texto = tr("ERROR: %1").arg(QString::number(erro,10));
        emit textoConsole();
       // error setting communications mask
    }

    if (!fWaitingOnRead) {
       // Issue read operation.
       if (!ReadFile(hComm, lpBuf, AMOUNT_TO_READ, &dwRead, &osReader)) {
          if (GetLastError() != ERROR_IO_PENDING){     // read not delayed?
             // Error in communications; report it.
              erro = GetLastError();
              texto = tr("ERROR: %1").arg(QString::number(erro,10));
              emit textoConsole();
          }
          else{
             fWaitingOnRead = TRUE;
             texto = "Leitura pendente.";
             emit textoConsole();
          }
       }
       else {
          // read completed immediately
          texto = "Leitura em andamento.";
          emit textoConsole();
          leituraCompleta(lpBuf, dwRead);
        }
    }

    if (fWaitingOnRead) {
       texto = "fWaitingOnRead";
       emit textoConsole();
       dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
       switch(dwRes)
       {
          // Read completed.
          case WAIT_OBJECT_0:
              if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)){ // Error in communications; report it.
                  texto = "Erro em comunicaçao.";
                  emit textoConsole();
              }
              else{
                  leituraCompleta(lpBuf, dwRead); // Read completed successfully.
              }
              //  Reset flag so that another opertion can be issued.
              fWaitingOnRead = FALSE;
              break;

          case WAIT_TIMEOUT:
              // Operation isn't complete yet. fWaitingOnRead flag isn't
              // changed since I'll loop back around, and I don't want
              // to issue another read until the first one finishes.
              //
              // This is a good time to do some background work.
              break;

          default:
              // Error in the WaitForSingleObject; abort.
              // This indicates a problem with the OVERLAPPED structure's
              // event handle.
              break;
       }/*
       int num;
       num = dwRead;
       texto = tr("Byte: %1\n").arg(QString::number(num,10));
       emit textoConsole();*/
    }

}

void comunicacao::leituraCompleta(char * lpBuf, DWORD dwRead)
{
    //QString teste;
    int num;

    //num = dwRead;
    for(int i=0;i < (int) dwRead; i++){
        num = (int) lpBuf[i];
        texto = tr("Byte: %1").arg(QString::number(num,16));
        emit textoConsole();
    }
    texto = tr("******************");
    emit textoConsole();
}

bool comunicacao::escreverArquivo(char * lpBuf, DWORD dwToWrite)
{
    OVERLAPPED osWrite = {0};
    DWORD dwWritten;
    BOOL fRes;
    int erro;

    /*PCVOID lpBuf1;
    DWORD size;
    size = GetFileSize(hComm,0);
    lpBuf1 = malloc(size);*/

    // Create this writes OVERLAPPED structure hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL){
       texto = "Erro ao criar evento."; // Error creating overlapped event handle.
       emit textoConsole();
       return FALSE;
   }

    // Issue write.
    if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
       if (GetLastError() != ERROR_IO_PENDING) {
          erro = GetLastError();
          texto = tr("ERROR: %1").arg(QString::number(erro,10)); // WriteFile failed, but it isn't delayed. Report error and abort.
          emit textoConsole();
          fRes = FALSE;
       }
       else {
          // Write is pending.
          texto = "Escrita está pendente.";
          emit textoConsole();
          if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)){
             fRes = FALSE;
          }
          else{
             texto = "Escrita com sucesso.";
             emit textoConsole();
             // Write operation completed successfully.
             fRes = TRUE;
          }
       }
    }
    else{
       texto = "Escrita completa.";
       emit textoConsole();
       // WriteFile completed immediately.
       fRes = TRUE;
   }

    texto = tr("msg: %1").arg(QString::number((int)lpBuf[0],10));
    emit textoConsole();

    CloseHandle(osWrite.hEvent);
    return fRes;
}
