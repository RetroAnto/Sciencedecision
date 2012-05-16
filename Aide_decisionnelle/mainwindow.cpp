#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->departementSpinBox->setVisible(false);
    connect(ui->validerButton, SIGNAL(clicked()), this, SLOT(lancerRequete()));
    connect(ui->departementCheckBox, SIGNAL(clicked(bool)), this, SLOT(affichageSpinBox(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::remplirBDDAvecFichierCSVForums()
{

    QFile file("forums.csv");

    file.open(QFile::ReadOnly | QFile::Text);            //Open the file with readonly mode and text mode

    QTextStream stream(&file);                       //the stream "stream" permits to access to the file "file"

    QSqlDatabase connexionBDD = QSqlDatabase::addDatabase("QSQLITE");                                                                   //Connection to the database
    connexionBDD.setDatabaseName("BDD");
    connexionBDD.open();

    if (connexionBDD.isOpen())
    {
        QSqlQuery query(connexionBDD);
        QList<QString> listeOfLine;
        while(!stream.atEnd())
        {
            listeOfLine.append(stream.readLine());
        }

        int identifiantBaseDeDonnees = 0;
        for(int i=1; i<listeOfLine.length();i++)
        {
            QStringList caseList = listeOfLine.at(i).split(";");
            if(caseList.at(4)!="")                                  //This test permits to not register a forum where no one came
            {
                query.prepare("INSERT INTO Forum(id, Titre, Etablissement, Departement, Date, NombreParticipants) VALUES(:id, :Titre, :Etablissement, :Departement, :Date, :NombreParticipants)");
                QString Titre = caseList.at(0);
                Titre.remove("\"", Qt::CaseInsensitive);
                QString Etablissement = caseList.at(1);
                Etablissement.remove("\"", Qt::CaseInsensitive);
                QString Date = caseList.at(3);
                Date.remove("\"", Qt::CaseInsensitive);
                query.bindValue(":id", identifiantBaseDeDonnees);
                query.bindValue(":Titre", Titre);
                query.bindValue(":Etablissement", Etablissement);
                query.bindValue(":Departement", caseList.at(2));
                query.bindValue(":Date", Date);
                QString champParticipants = caseList.at(4);
                int nbParticipants = champParticipants.count("#");
                if(nbParticipants > 1) // Traitements à effectuer sur le nombre d'occurences de # dûes aux conventions d'écritures dans le fichier .csv
                {
                    nbParticipants--;
                    nbParticipants = (nbParticipants/2)+1;
                }
                query.bindValue(":NombreParticipants", nbParticipants);
                query.exec();
                identifiantBaseDeDonnees++;

            }
        }

    }
    connexionBDD.close();

}

void MainWindow::remplirBDDAvecFichierCSVEleves()
{

    QFile file("eleves.csv");

    file.open(QFile::ReadOnly | QFile::Text);            //Open the file with readonly mode and text mode

    QTextStream stream(&file);                       //the stream "stream" permits to access to the file "file"

    QSqlDatabase connexionBDD = QSqlDatabase::addDatabase("QSQLITE");                                                                   //Connection to the database
    connexionBDD.setDatabaseName("BDD");
    connexionBDD.open();

    if (connexionBDD.isOpen())
    {
        QSqlQuery query(connexionBDD);
        QList<QString> listeOfLine;
        while(!stream.atEnd())
        {
            listeOfLine.append(stream.readLine());
        }
        int identifiantBaseDeDonnees = 0;
        for(int i=1; i<listeOfLine.length();i++)
        {
            QStringList caseList = listeOfLine.at(i).split(";");
            if(caseList.at(0) != "\"ANNEE\"")
            {
                query.prepare("INSERT INTO Eleve(id, Annee, Etape, Nombre, DeptBac) VALUES(:id, :Annee, :Etape, :Nombre, :DeptBac)");
                QString Etape = caseList.at(1);
                Etape.remove("\"", Qt::CaseInsensitive);
                QString Nombre = caseList.at(3);
                Nombre.remove("\"", Qt::CaseInsensitive);
                QString DeptBac = caseList.at(4);
                DeptBac.remove("\"", Qt::CaseInsensitive);
                query.bindValue(":id", identifiantBaseDeDonnees);
                query.bindValue(":Annee", caseList.at(0));
                query.bindValue(":Etape", Etape);
                query.bindValue(":Nombre", Nombre);
                query.bindValue(":DeptBac", DeptBac);
                query.exec();
                identifiantBaseDeDonnees++;
            }
        }

    }
    connexionBDD.close();
}

void MainWindow::creer_BDD()
{
    QSqlDatabase connexionBDD = QSqlDatabase::addDatabase("QSQLITE");
    connexionBDD.setDatabaseName("BDD");
    connexionBDD.open();
    if(connexionBDD.isOpen())
    {
        QSqlQuery query(connexionBDD);
        query.exec("CREATE TABLE Forum(id INTEGER, Titre VARCHAR (50), Etablissement VARCHAR(50), Departement INTEGER, Date VARCHAR (50), NombreParticipants INTEGER)");
        query.exec("CREATE TABLE Eleve(id INTEGER, Annee VARCHAR(50), Etape VARCHAR(50), Nombre INTEGER, DeptBac INTEGER)");
        connexionBDD.close();
    }
    MainWindow::remplirBDDAvecFichierCSVForums();
    MainWindow::remplirBDDAvecFichierCSVEleves();

}

void MainWindow::lancerRequete()
{
    QSqlDatabase connexionBDD = QSqlDatabase::addDatabase("QSQLITE");
    connexionBDD.setDatabaseName("BDD");
    connexionBDD.open();
    if(connexionBDD.isOpen())
    {
        QSqlQuery query;
        double resultat = 0;
        QString annee = ui->anneeComboBox->currentText();
        QString promotion = ui->promoComboBox->currentText();
        int departement = -1;
        if(ui->departementSpinBox->isVisible())
        {
            departement = ui->departementSpinBox->value();
        }

        if(!(annee.contains("A"))) // On est toujours sur la valeur par défaut
        {
            if(!(promotion.contains("Promo"))) // Idem
            {
                if(departement != -1)
                {
                    query.prepare("SELECT nombre FROM eleve WHERE annee = :Annee AND etape = :Promo AND DeptBac = :Dept");
                    query.bindValue(":Annee", annee);
                    query.bindValue(":Promo", promotion);
                    query.bindValue(":Dept", departement);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
                else
                {
                    query.prepare("SELECT nombre FROM eleve WHERE annee = :Annee AND etape = :Promo");
                    query.bindValue(":Annee", annee);
                    query.bindValue(":Promo", promotion);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
            }
            else
            {
                if(departement != -1)
                {
                    query.prepare("SELECT nombre FROM eleve WHERE annee = :Annee AND DeptBac = :Dept");
                    query.bindValue(":Annee", annee);
                    query.bindValue(":Dept", departement);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
                else
                {
                    query.prepare("SELECT nombre FROM eleve WHERE annee = :Annee");
                    query.bindValue(":Annee", annee);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
            }
        }
        else
        {
            if(!(promotion.contains("Promo"))) // Idem
            {
                if(departement != -1)
                {
                    query.prepare("SELECT nombre FROM eleve WHERE etape = :Promo AND DeptBac = :Dept");
                    query.bindValue(":Promo", promotion);
                    query.bindValue(":Dept", departement);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
                else
                {
                    query.prepare("SELECT nombre FROM eleve WHERE etape = :Promo");
                    query.bindValue(":Promo", promotion);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
            }
            else
            {
                if(departement != 1)
                {
                    query.prepare("SELECT nombre FROM eleve WHERE DeptBac = :Dept");
                    query.bindValue(":Dept", departement);
                    if(query.exec())
                    {
                        while(query.next())
                        {
                            resultat += query.value(0).toDouble();
                        }
                    }
                }
            }
        }

        //QString test("Résultat : " << resultat << "étudiants");
        //ui->resultatLabel->setText();
        qDebug() << resultat;
    }
}

void MainWindow::affichageSpinBox(bool visible){
    ui->departementSpinBox->setVisible(visible);
}
