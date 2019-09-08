#ifndef CONECTAR_H
#define CONECTAR_H

#include <QDialog>

namespace Ui {
    class Conectar;
}

class Conectar : public QDialog
{
    Q_OBJECT

public:
    explicit Conectar(QWidget *parent = 0);
    ~Conectar();

public:
    QString portaConexao;

signals:
    void terminar();

private slots:
    void on_ok_clicked();

private:
    Ui::Conectar *ui;
};

#endif // CONECTAR_H
