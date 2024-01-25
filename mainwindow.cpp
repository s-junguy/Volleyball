#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QString>
#include"spieltag.h"
#include"spielstatistik.h"
#include"functions.h"
#include<unistd.h>
#include<QSqlDatabase>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    d.createTable();
    //database connection for the tableView
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("teams.db");
    if(!db.open()){
        ui->tabelle->setText("DB konnte nicht geöffnet werden.");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }
    connect(this, SIGNAL(updateTable()), this, SLOT(on_tableform_clicked())); //for updating table view when data set inserted/deleted
    connect(ui->zoombox, SIGNAL(valueChanged(int)), this, SLOT(changezoom(int))); //value in zoombox changed -> set zoom on new value
    connect(this, SIGNAL(newzoom()), this, SLOT(showMap())); //load map again with new zoom
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(redrawTile(QNetworkReply*))); //reply finished -> draw tile

    //int countFinishedReply updated when one reply finished
    //emits signal allfinished if all replies are finished
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(updateCountreply(QNetworkReply*)));
    connect(this, SIGNAL(loadmap()), this, SLOT(showMap())); //loadmap emitted in on_showpath_clicked where dijkstra is being used
    connect(this, SIGNAL(allfinished()), this, SLOT(loadpath())); //all replies from loading map finished, now loadpath

    ui->teamsForSchedule->setPlaceholderText("z.B. 1,2,3");
}

MainWindow::~MainWindow()
{
    delete ui;
    d.closedb();
    db.close();//database connection with qsql for tableView
}

void MainWindow::on_pushTeam_clicked(){
    QString nr = ui->eingabeNr->text();
    QString ort = ui->eingabeOrt->text();
    QString name = ui->eingabeName->text();
    QString laengengrad = ui->eingabeLaengengrad->text();
    QString breitengrad = ui->eingabeBreitengrad->text();

    if(nr != "" && ort != "" && name != "" && breitengrad != "" && laengengrad != ""){
        if(!d.insert(nr.toStdString(), name.toStdString(), ort.toStdString(), breitengrad.toStdString(),
                     laengengrad.toStdString())){
            ui->tabelle->setText("Error in: Datensatz einfügen\nTeam Nr. existiert schon.");
            ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
        }else{
            ui->tabelle->setText("Datensatz eingefügt");
            ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
        }
    }else{
        ui->tabelle->setText("Error in: Datensatz einfügen.\nAlle Felder müssen Einträge haben.");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }

    emit updateTable();

    ui->eingabeOrt->clear();
    ui->eingabeNr->clear();
    ui->eingabeName->clear();
    ui->eingabeBreitengrad->clear();
    ui->eingabeLaengengrad->clear();

}

//writes schedule with team names and place on the second tab
void MainWindow::writeschedule(){
    string output = "";
    //schedulerightnumbers is a vector holding all playing days
    for(int i=0; i<schedulerightnumbers.size(); i++){
        output.append("Spieltag ");
        output.append(to_string(i+1));
        output.append(":\n");
        output.append(to_string(schedulerightnumbers[i].heimMannschaft));
        output.append(". ");
        output.append(d.getname(schedulerightnumbers[i].heimMannschaft));
        output.append("\n");
        output.append(to_string(schedulerightnumbers[i].t2));
        output.append(". ");
        output.append(d.getname(schedulerightnumbers[i].t2));
        output.append("\n");
        output.append(to_string(schedulerightnumbers[i].t3));
        output.append(". ");
        output.append(d.getname(schedulerightnumbers[i].t3));
        output.append("\nSpielort: ");
        output.append(d.getort(schedulerightnumbers[i].heimMannschaft));
        output.append("\n\n");
    }
    ui->schedulewritten->setText(QString::fromStdString(output));
}

void MainWindow::on_pushBerechnung_clicked(){
    vector<int> numbersForSchedule;//consists of the team numbers we type into the text edit to get a schedule
    schedule = {};//original schedule with numbers 1 to n, must be empty for new calculation
    schedulerightnumbers = {};//schedule replaced with right team numbers
    ui->tabelle->clear();
    ui->schedulewritten->clear();
    vector<int> keysintable;//vector with keys from the table
    d.keysIntoVector(keysintable);//get keys into the vector

    //get team numbers from teams for schedule
    string keys = (ui->teamsForSchedule->text()).toStdString();
    string k = "";//k appends the characters til they are one team number -> then save k

    //following code gets the team numbers from string keys (textEdit) into vector numbersForSchedule
    //we check wether the number exists as a key in the table and wether the input format is correct
    for(unsigned i=0; i<keys.size(); i++){
        if(keys[i] == ','){
            if(isdigit(keys[i-1])){
                if(count(keysintable.begin(), keysintable.end(), stoi(k))){//check if key exists in table
                    numbersForSchedule.push_back(stoi(k));
                    k = "";
                }else{
                    QString output = "Team Nr. " + QString::number(stoi(k)) + " existiert nicht in der Tabelle. Der Spielplan kann nicht erstellt werden.";
                    ui->tabelle->setText(output);
                    ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
                    return;
                }
            }else{
                ui->tabelle->setText("Eingabeformat der Teamnummber nicht korrekt. "
                                     "Bitte nur Zahlen mit Komma getrennt eingeben. Bsp: '1,2,3'");
                ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
            }
        }else if(isdigit(keys[i])){
            k+=keys[i];
            if(i == keys.size()-1){
                if(count(keysintable.begin(), keysintable.end(), stoi(k))){
                    numbersForSchedule.push_back(stoi(k));
                }else{
                    QString output = "Team Nr. " + QString::number(stoi(k)) + " existiert nicht in der Tabelle. Der Spielplan kann nicht erstellt werden.";
                    ui->tabelle->setText(output);
                    ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
                    return;
                }
            }
        }else{
            ui->tabelle->setText("Eingabeformat der Teamnummber nicht korrekt. "
                                 "Bitte nur Zahlen mit Komma getrennt eingeben. Bsp: '1,2,3'");
            ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
            return;
        }
    }

    //now get the schedule and do the output with the right team numbers
    int n = numbersForSchedule.size();//number of teams
    if(n < 3){
        ui->tabelle->setText("Mind. 3 Teams nötig");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }else if(n == 3){
        QString spielplan = "Spieltag 1: (" + ui->teamsForSchedule->text() + ")\n" + "Spieltag 2: (" + ui->teamsForSchedule->text() + ")";
        ui->tabelle->setText(spielplan);
        Spieltag s(numbersForSchedule[0],numbersForSchedule[1], numbersForSchedule[2],0);
        schedulerightnumbers.push_back(s);
        schedulerightnumbers.push_back(s);
        writeschedule();
    }else{
        if(n%3 == 0 || (n-1)%3 == 0){
            vector<Spieltag> tupel;
            vector<Spielstatistik> stat;
            vector<int> sum;
            vector<int> rangefirst;
            gesTupel(n,tupel,stat,sum,rangefirst);//create tuples and statistics
            solve(1, tupel,stat,sum, schedule, rangefirst,n);//get schedule
            QString plan;//output string for schedule
            for(int i=0;i<schedule.size();i++){
                //replace values from tuple with team numbers to show schedule
                plan.append("Spieltag ");
                plan.append(QString::number(i+1));
                plan.append(": (");
                plan.append(QString::number(numbersForSchedule[schedule[i].heimMannschaft-1]));
                plan.append(",");
                plan.append(QString::number(numbersForSchedule[schedule[i].t2-1]));
                plan.append(",");
                plan.append(QString::number(numbersForSchedule[schedule[i].t3-1]));
                plan.append(")\n");

                Spieltag scheduleright(numbersForSchedule[schedule[i].heimMannschaft-1],numbersForSchedule[schedule[i].t2-1],
                        numbersForSchedule[schedule[i].t3-1], schedule[i].tupelIndex);
                schedulerightnumbers.push_back(scheduleright);
                writeschedule();
            }
            ui->tabelle->setText(plan);
        }else{
            ui->tabelle->setText("Kein zulässiger Spielplan verfügbar, da n und n-1 nicht durch 3 teilbar sind.");
            ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
        }
    }
}

void MainWindow::on_teamEntfernen_clicked(){
    QString q = ui->teamNrEntfern->text();
    if(!d.remove(q.toStdString())){
        ui->tabelle->setText("Error in: Datensatz entfernen. Team existiert nicht.");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }else{
        ui->tabelle->setText("Datensatz entfernt");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }
    emit updateTable();
    ui->teamNrEntfern->clear();
}

void MainWindow::on_tableform_clicked(){
    //to access the database
    //create database connection and open database
//    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//    db.setDatabaseName("teams.db");
//    if(!db.open()){
//        ui->tabelle->setText("DB konnte nicht geöffnet werden.");
//        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
//    }
//    QSqlDatabase db = QSqlDatabase::database("teams.db");
//    db.setDatabaseName("teams.db");
    //create the model
    model = new QSqlTableModel(this,db);
    model->setTable("TEAMS");

    //get data from table into model
    model->select();

    //get model into tableView widget
    ui->tableView->setModel(model);
}

void MainWindow::redrawTile(QNetworkReply* reply)
{
    //replyIntxMap is unordered map with <reply, pair<int,int>>
    //replyIntxMap holds QNetworkReply with the matching position for the tile
    int x = replyIntxMap[reply].first;
    int y = replyIntxMap[reply].second;
    bytes=networkReply[x][y]->readAll();
    myMap.loadFromData(bytes);

//QGraphicsPixmapItem provides a pixmap item that can be added to GraphicsScene
    itemPtr=myScene.addPixmap(myMap);
    itemPtr->setPos(x*256,y*256);
    networkReply[x][y]->deleteLater();
}

int MainWindow::long2tilex(double lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int MainWindow::lat2tiley(double lat, int z)
{
    double latrad = lat * M_PI/180.0;
    return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z)));
}

double MainWindow::tilex2long(int x, int z)
{
    return x / (double)(1 << z) * 360.0 - 180;
}

double MainWindow::tiley2lat(int y, int z)
{
    double n = M_PI - 2.0 * M_PI * y / (double)(1 << z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void MainWindow::doRequest(int tilex, int tiley){
    for(int x=-2; x<=1; x++){
        //get tilex into url
        QString preurl = "https://humboldt.openstreetmap.de/" + QString::number(zoom) + "/";
        preurl += QString::number(tilex+x) + "/";

        for(int y=-2; y<=1; y++){
            //get tiley into url
            QString url = preurl;
            url += QString::number(tiley+y) + ".png";
//            qDebug() << url;//show url in terminal
            //get request and reply
            request.setUrl(QUrl(url));
            networkReply[x+2][y+2] = manager.get(request);

            //get reply and int into unordered map
            replyIntxMap[networkReply[x+2][y+2]]=make_pair(x+2,y+2);
        }
    }
}

//x,y are integers holding the direction in which we move the map
//x,y are (0,0) if we just load the map with lon and lat in the middle
void MainWindow::loadDirection(int x, int y){
    //all buttons that load a new map have to be disabled while loading current map
    ui->zoombox->setDisabled(true);
    ui->north->setDisabled(true);
    ui->east->setDisabled(true);
    ui->south->setDisabled(true);
    ui->west->setDisabled(true);
    myScene.clear();
    countFinishedReplies = 0;

    //get tile numbers from lon/lat
    if(x == 0 && y == 0){//we dont move the map -> lon and lat in the middle
        tilex = long2tilex(longitude, zoom);
        tiley = lat2tiley(latitude, zoom);
    }else{//we move the map -> change starting tile number
        tilex += x;
        tiley += y;
    }
    //move map -> lon and lat have to be loaded again to be in the middle
    if(x != 0 || y != 0){
        longitude = tilex2long(tilex, zoom);
        latitude = tiley2lat(tiley, zoom);
    }

    doRequest(tilex, tiley);//do request -> finished -> draw

    ui->graphicsView->setScene(&myScene);
    ui->graphicsView->show();

}

void MainWindow::showMap(){
    loadDirection(0,0);//load map, allfinished->loadpath
}

void MainWindow::updateCountreply(QNetworkReply* r){
    countFinishedReplies++;//if one reply finished
    if(countFinishedReplies == 16){
        //all buttons were disabled while loading current map
        //now all replies are finished -> enable buttons
        ui->zoombox->setEnabled(true);
        ui->north->setEnabled(true);
        ui->east->setEnabled(true);
        ui->south->setEnabled(true);
        ui->west->setEnabled(true);
        emit allfinished();//whole map loaded, signal to loadpath
    }
}

void MainWindow::changezoom(int zoomvalue){
        zoom = zoomvalue;
        emit newzoom();//signal for loading new map (tile numbers have to be calculated again, so new requests)
}

void MainWindow::on_north_clicked(){
    loadDirection(0,-1);
}

void MainWindow::on_east_clicked(){
    loadDirection(1,0);
}

void MainWindow::on_south_clicked(){
    loadDirection(0,1);
}

void MainWindow::on_west_clicked(){
    loadDirection(-1,0);
}

void MainWindow::on_loadgtxt_clicked(){
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Image"), "~",
                                                    tr("All Files (*)"));
    dij = new Dijkstra(filename.toStdString());
    gtxtloaded = true;
}

//returns the greatest distance between two points of the path
double MainWindow::dodijkstra(){
    DataManager d;
    int spieltag = ui->spieltagnr->value();
    if(spieltag > schedulerightnumbers.size()) return -1;//Spieltag doesnt exist

    //get lon/lat of the teams we built a schedule with
    lat1 = d.getlat(schedulerightnumbers[spieltag-1].heimMannschaft);
    lat2 = d.getlat(schedulerightnumbers[spieltag-1].t2);
    lat3 = d.getlat(schedulerightnumbers[spieltag-1].t3);
    lon1 = d.getlon(schedulerightnumbers[spieltag-1].heimMannschaft);
    lon2 = d.getlon(schedulerightnumbers[spieltag-1].t2);
    lon3 = d.getlon(schedulerightnumbers[spieltag-1].t3);

    pair<double,double> startknoten1;
    startknoten1 = make_pair(lon3, lat3);

    pair<double,double> startknoten2;
    startknoten2 = make_pair(lon2, lat2);

    pair<double,double> zielknoten;
    zielknoten = make_pair(lon1, lat1);

    dij->berechne_dijkstra(dij->returnNextKnoten(startknoten1),dij->returnNextKnoten(zielknoten));// berechnet den kuerzesten Weg

    wegcoord1=dij->findeWeg();//vector of pairs with <lon, lat> that holds a path from startknoten1 to zielknoten1

    dij->berechne_dijkstra(dij->returnNextKnoten(startknoten2),dij->returnNextKnoten(zielknoten));// berechnet den kuerzesten Weg

    wegcoord2=dij->findeWeg();

    //get middle point of 2 paths and difference between highest and lowest lon/lat
    double minlon = wegcoord1[0].first;
    double maxlon = wegcoord1[0].first;
    double minlat = wegcoord1[0].second;
    double maxlat = wegcoord1[0].second;
    for(int i=1; i<wegcoord1.size(); i++){
        if(wegcoord1[i].first < minlon){
            minlon = wegcoord1[i].first;
        }
        if(wegcoord1[i].first > maxlon){
            maxlon = wegcoord1[i].first;
        }
        if(wegcoord1[i].second < minlat){
            minlat = wegcoord1[i].second;
        }
        if(wegcoord1[i].second > maxlat){
            maxlat = wegcoord1[i].second;
        }
    }
    for(int i=1; i<wegcoord2.size(); i++){
        if(wegcoord2[i].first < minlon){
            minlon = wegcoord2[i].first;
        }
        if(wegcoord2[i].first > maxlon){
            maxlon = wegcoord2[i].first;
        }
        if(wegcoord2[i].second < minlat){
            minlat = wegcoord2[i].second;
        }
        if(wegcoord2[i].second > maxlat){
            maxlat = wegcoord2[i].second;
        }
    }

    longitude = maxlon - (maxlon - minlon)/2;
    latitude = maxlat - (maxlat - minlat)/2;

    double diflon = maxlon - minlon;
    double diflat = maxlat - minlat;

    double dif;//greatest difference between two lon and lat from both paths
    if(diflon > diflat){
        dif = diflon;
    }else{
        dif = diflat;
    }
    return dif;

}

void MainWindow::on_showpath_clicked(){
    if(schedulerightnumbers.size() == 0){
        ui->tabelle->setText("Spielplan wurde noch nicht berechnet. Bitte gib die Teamnummern ein für die du einen Spielplan berechnen möchtest und"
                             " klicke auf 'Spielplan für Teams'.");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
        return;
    }
    if(gtxtloaded == false){
        ui->tabelle->setText("gtxt Datei wurde noch nicht geladen. Bitte klicken Sie auf 'Lade .gtxt Datei' und wählen Sie eine .gtxt Datei aus.");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
        return;
    }
    //dijkstra algorithm is used
    //calculate greatest difference between two lat/lon on the paths
    //longitude and latitude are calculated to be in the middle between both paths so the path will be shown in the middle of the map
    double dif = dodijkstra();
    //calculate zoom
    if(dif>0){
        int i=0;
        //tilewidth is a vector that holds the width of tiles in degrees
        //we want to get the right zoom so both paths are completely on the map
        while(i < tilewidth.size() && dif < tilewidth[i]){
            i++;
        }
        zoom = i;
        //disconnect so map wont be loaded 2 times because button clicked and zoom changed
        disconnect(ui->zoombox, SIGNAL(valueChanged(int)), this, SLOT(changezoom(int)));
        ui->zoombox->setValue(i);//set value in zoombox on calculated zoom value
        connect(ui->zoombox, SIGNAL(valueChanged(int)), this, SLOT(changezoom(int)));

        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);//change tab
        emit loadmap();//signal to showMap
    }else{
        ui->tabelle->setText("Spieltag existiert nicht");
        ui->tabWidgetSpielplan->setCurrentIndex(0);//change tab
    }


}
//value is the world coordinate (here lon) we want to change into pixel coordinate
//begin is the smallest lat on the map
//dif is the difference between the smallest and biggest lon on the map
double MainWindow::coordToPixelx(double value, double begin, double dif){
    return ((value-begin)/dif)*1024;
}

double MainWindow::coordToPixely(double value, double begin, double dif){
    return 1024-(((value-begin)/dif)*1024);
}

double MainWindow::nextLonInMap(double lon){
    if(lon >= xbegin && lon <= xend) return lon;//lon within map
    else if(abs(lon-xend) < abs(lon-xbegin)) return xend;//lon>xend
    else return xbegin;//lon <xbegin
}

double MainWindow::nextLatInMap(double lat){
    if(lat >= ybegin && lat <= yend) return lat;
    else if(abs(lat-yend) < abs(lat-ybegin)) return yend;
    else return ybegin;
}

void MainWindow::lineToScene(std::vector<std::pair<double, double>> wegcoord, QPen pen){
    for(int i=0; i<wegcoord.size()-1; i++){
        //check wether path is still on map
        //first = lon = x, second = lat = y
        //if both points i and i+1 within map
        if(wegcoord[i].first < xend && wegcoord[i].first > xbegin && wegcoord[i].second < yend && wegcoord[i].second > ybegin &&
            wegcoord[i+1].first < xend && wegcoord[i+1].first > xbegin && wegcoord[i+1].second < yend && wegcoord[i+1].second > ybegin)
            myScene.addLine(coordToPixelx(wegcoord[i].first,xbegin, difx), coordToPixely(wegcoord[i].second,ybegin, dify),
                        coordToPixelx(wegcoord[i+1].first, xbegin, difx), coordToPixely(wegcoord[i+1].second, ybegin, dify), pen);
        //if both points not within map
        else if((wegcoord[i].first > xend || wegcoord[i].first < xbegin || wegcoord[i].second > yend || wegcoord[i].second < ybegin) &&
                (wegcoord[i+1].first > xend || wegcoord[i+1].first < xbegin || wegcoord[i+1].second > yend || wegcoord[i+1].second < ybegin))
            continue;
        //if one point within map
        else
            myScene.addLine(coordToPixelx(nextLonInMap(wegcoord[i].first),xbegin, difx), coordToPixely(nextLatInMap(wegcoord[i].second),ybegin, dify),
                        coordToPixelx(nextLonInMap(wegcoord[i+1].first), xbegin, difx), coordToPixely(nextLatInMap(wegcoord[i+1].second), ybegin, dify), pen);
    }
}

void MainWindow::loadpath(){

    //calculate lon/lat of corners of the whole map
    xbegin = tilex2long(tilex-2,zoom);//smallest lon on map
    ybegin = tiley2lat(tiley+2,zoom);//smallest lat on map
    xend = tilex2long(tilex+2,zoom);//biggest lon on map
    yend = tiley2lat(tiley-2,zoom);//biggest lat on map

    //length of width and height of the whole map in lon/lat
    difx = xend - xbegin;
    dify = yend - ybegin;

    //draw ellipses of 3 places
    QGraphicsEllipseItem* e = new QGraphicsEllipseItem(coordToPixelx(lon1, xbegin, difx), coordToPixely(lat1, ybegin, dify),10,10);
    if(lon1 > xbegin && lon1 < xend && lat1 > ybegin && lat1 < yend){//check if place is still within the map
        e->setBrush(QBrush(Qt::red));
        myScene.addItem(e);
    }
    if(lon2 > xbegin && lon2 < xend && lat2 > ybegin && lat2 < yend){
        e = new QGraphicsEllipseItem(coordToPixelx(lon2, xbegin, difx), coordToPixely(lat2, ybegin, dify),10,10);
        e->setBrush(QBrush(Qt::blue));
        myScene.addItem(e);
    }
    if(lon3 > xbegin && lon3 < xend && lat3 > ybegin && lat3 < yend){
        e = new QGraphicsEllipseItem(coordToPixelx(lon3, xbegin, difx), coordToPixely(lat3, ybegin, dify),10,10);
        e->setBrush(QBrush(Qt::blue));
        myScene.addItem(e);
    }

    //draw paths between ellipses
    QPen pen1 (Qt::green);
    pen1.setWidth(4);
    lineToScene(wegcoord1,pen1);
    QPen pen2 (Qt::darkGreen);
    pen2.setWidth(4);
    lineToScene(wegcoord2,pen2);
}
//load teams from file into database table
void MainWindow::on_loadteams_clicked(){
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Image"), "~",
                                                    tr("All Files (*)"));
    QFile inputFile(filename);
    inputFile.open(QIODevice::ReadOnly);

    QTextStream in(&inputFile);
    int teamend = 0;//one team as 3 lines in file
    //save all data from a team in strings to insert into table
    string keynumber;
    string name;
    string ort;
    string breite;
    string laenge;
    while(!in.atEnd()){//go through all lines of file
        QString line = in.readLine();
        QStringList linelist;
        if(teamend == 0){//first line holds the key and name
            linelist = line.split(".");
            keynumber = linelist[0].toStdString();
            name = linelist[1].remove(0,1).toStdString();
        }else if(teamend == 1){//second line holds the place
            ort = line.toStdString();
        }else{//third line holds lat and lon of the place
            linelist = line.split(" ");
            breite = linelist[0].toStdString();
            laenge = linelist[1].toStdString();
            DataManager d;
            d.insert(keynumber, name, ort, breite, laenge);//insert data set into table
        }
        teamend++;
        if(teamend == 3) teamend = 0;
    }
    inputFile.close();
    emit updateTable();//now table will be shown on tableView
}

//if helpbutton clicked then second window with a program guide will be opened
void MainWindow::on_helpbutton_clicked(){
    help = new Help(this);
    help->show();
}




