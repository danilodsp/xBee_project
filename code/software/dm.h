#ifndef DM_H
#define DM_H

#include <QMainWindow>
#include <windows.h>
//#include <QtSql/QSqlDatabase>
//#include <QtSql/QMYSQLDriver>
#include <QMessageBox>
#include <QString>
#include <QTreeWidgetItem>
#include <QThread>
#include "conectar.h"
#include <comunicacao.h>

namespace Ui {
    class DM;
}

class DM : public QMainWindow
{
    Q_OBJECT

public:
    explicit DM(QWidget *parent = 0);
    ~DM();
    void init();
    bool criarConexao();
    bool abrirPorta();
    bool lerArquivo();
    void leituraCompleta(LPVOID, DWORD);
    void conclusao(DWORD dwRead, LPVOID lpBuf, BOOL fWaitingOnRead, OVERLAPPED osReader);
    bool escreverArquivo(char *lpBuf, DWORD dwToWrite);

public:
    QString textoPorta;
    //HANDLE hComm;
    QTreeWidgetItem *itemTree;
    QTreeWidgetItem *item2;

public:
    Conectar *conexao;
    comunicacao *comm;

public slots:
    void abrirConexao();
    void atualizarConexao();
    void printTexto();
    void msgAlerta();

private:
    Ui::DM *ui;

private slots:
    void on_pushButton_ler_clicked();
    void on_pushButton_clicked();
};

#endif // DM_H
