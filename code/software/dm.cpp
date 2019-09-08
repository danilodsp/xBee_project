#include "dm.h"
#include "ui_dm.h"

DM::DM(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DM)
{
    ui->setupUi(this);

    ui->console->ensureCursorVisible();

    init();
    connect(ui->actionConectar,SIGNAL(triggered()),this,SLOT(abrirConexao()));
}

DM::~DM()
{
    delete ui;
}

void DM::init(){
    // Inicializando tabela
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(17);
    QTableWidgetItem *titulo0 = new QTableWidgetItem;
    QTableWidgetItem *titulo1 = new QTableWidgetItem;
    QTableWidgetItem *titulo2 = new QTableWidgetItem;
    titulo0->setText("Data - Hora");
    ui->tableWidget->setHorizontalHeaderItem(0,titulo0);
    titulo1->setText("ID");
    ui->tableWidget->setHorizontalHeaderItem(1,titulo1);
    titulo2->setText("Dados");
    ui->tableWidget->setHorizontalHeaderItem(2,titulo2);
    ui->tableWidget->setColumnWidth(0,200);
    ui->tableWidget->setColumnWidth(1,60);
    ui->tableWidget->setColumnWidth(2,120);
    for(int h=0; h<ui->tableWidget->rowCount(); h++)
        ui->tableWidget->setRowHeight(h,20);

    // Inserindo Tree
    itemTree = new QTreeWidgetItem();
    QStringList labels;
    labels << tr("ID") << tr("Endereço");
    itemTree->setText(0,"teste");
    itemTree->setText(1,"ok");
    ui->treeWidget->setColumnWidth(0,20);
    ui->treeWidget->setColumnWidth(1,40);
    //ui->treeWidget->header()->setResizeMode(QHeaderView::Stretch);
    //ui->treeWidget->header();
    ui->treeWidget->setHeaderLabels(labels);
    item2 = new QTreeWidgetItem(itemTree);
    item2 = new QTreeWidgetItem(ui->treeWidget);
    item2->setData(0,Qt::UserRole,"Teste");
}

bool DM::criarConexao(){
}

void DM::abrirConexao(){
    conexao = new Conectar();

    conexao->show();
    connect(conexao,SIGNAL(terminar()),this,SLOT(atualizarConexao()));
}

void DM::atualizarConexao(){
    textoPorta = conexao->portaConexao;

    comm = new comunicacao();
    connect(comm,SIGNAL(textoConsole()),this,SLOT(printTexto()));
    connect(comm,SIGNAL(alertaPorta()),this,SLOT(msgAlerta()));

    ui->console->appendPlainText(tr("Abrir porta: %1").arg(textoPorta));

    comm->porta = textoPorta;
    //abrirPorta();

    comm->start();
}

void DM::printTexto()
{
    ui->console->appendPlainText(comm->texto);
}

void DM::msgAlerta()
{
    QString tst;

    tst = comm->porta;

    tst = tr("%1").arg(QString(tst.toLatin1()));
    QMessageBox::critical(0, QObject::tr("ERRO Porta inválida"), tst);
}

void DM::on_pushButton_clicked()
{
    char lpBuf[512];
    DWORD dwToWrite = 1;

    //lpBuf[0] = (char) ui->lineEdit->text().toAscii().toLong(0,16);
    lpBuf[0] = (char) ui->lineEdit->text().toInt(0,10);
    //dwToWrite = 0x00000005;

    if(comm->escreverArquivo(lpBuf, dwToWrite)){
        ui->console->appendPlainText(tr("Operaçao de escrita concluída."));
        ui->console->appendPlainText(tr("******************"));
    }
}

void DM::on_pushButton_ler_clicked()
{
    comm->lerArquivo();
}
