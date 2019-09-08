#include "conectar.h"
#include "ui_conectar.h"

Conectar::Conectar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Conectar)
{
    ui->setupUi(this);
    ui->linePorta->setFocus();
}

Conectar::~Conectar()
{
    delete ui;
}

void Conectar::on_ok_clicked()
{
    portaConexao = ui->linePorta->text();

    emit terminar();
    this->close();
}
