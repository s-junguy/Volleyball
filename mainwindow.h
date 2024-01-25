#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>
#include<QSqlTableModel>
#include<cctype>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QGraphicsScene>
#include<QGraphicsPixmapItem>
#include<QUrl>
#include<unordered_map>
#include<utility>
#include<cmath>
#include<algorithm>
#include<string>
#include<QFileDialog>
#include"dijkstra.h"
#include"spieltag.h"
#include"help.h"
#include"dataManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::vector<Spieltag> schedulerightnumbers;

private:
    Ui::MainWindow *ui;
    DataManager d;
    QSqlDatabase db;
    QSqlTableModel *model;//model for tableView
    std::vector<Spieltag> schedule;
    double longitude;
    double latitude;
    int tilex;
    int tiley;
    int zoom;
    int countFinishedReplies;
    Dijkstra* dij;
    bool gtxtloaded = false;//true if we loaded the gtxt file, now dijkstra can be used
    QNetworkAccessManager manager;
    QNetworkReply *networkReply[4][4];//two dimensional to get indices for position in pix map
    QNetworkRequest request;
    QGraphicsScene myScene;
    QByteArray bytes;
    QPixmap myMap;
    QGraphicsPixmapItem *itemPtr;
    std::unordered_map<QNetworkReply*, std::pair<int, int>> replyIntxMap;//key networkreply, value position
    std::vector<double> tilewidth = {360,180,90,45,22.5,11.25,5.625,2.813,1.406,0.703,0.352,0.176,0.088,0.044,0.022,0.011,0.005,0.003,0.001};
    std::vector<std::pair<double, double>> wegcoord1;//vector of pairs <lon, lat> that holds a path
    std::vector<std::pair<double, double>> wegcoord2;
    double coordToPixelx(double value, double begin, double dif);
    double coordToPixely(double value, double begin, double dif);


    void doRequest(int tilex, int tiley);
    int long2tilex(double lon, int z);
    int lat2tiley(double lat, int z);
    double tilex2long(int x, int z);
    double tiley2lat(int y, int z);
    void loadDirection(int x, int y);//loads map for tilex + x, tiley + y
    double dodijkstra();//just works with gtxt file so gtxtloaded must be true

    double lon1, lon2, lon3, lat1, lat2, lat3;//lon and lat from teams we want to draw in map
    double xbegin, xend, ybegin, yend;//lon and lat for the edge of the map
    double difx, dify;//difference between xbegin and xend, ybegin and yend
    double nextLonInMap(double lon);//returns next lon in map if lon is not within the map
    double nextLatInMap(double lat);//returns next lat in map if lat is not within the map
    void lineToScene(std::vector<std::pair<double,double>>wegcoord, QPen pen);//adds line to scene from a coord vector
    Help* help;
    void writeschedule();//writes schedule with team names on textEdit

signals:
    void updateTable();//will be sent when data set inserted/deleted to update table view
    void newzoom();//emitted when zoom value changed -> tiles have to be calculated again
    void loadmap();//emitted when we want to change the map
    void allfinished();//emitted when all replies finished -> draw path

private slots:
    void on_pushTeam_clicked();//inserts data set (one team) into table
    void on_pushBerechnung_clicked();//calculates schedule
    void on_teamEntfernen_clicked();//deletes data set (one team) from table
    void on_tableform_clicked();//gets model for tableView, also called when table changed (data set inserted/deleted)

    void showMap();//calculates new tiles and draws into graphicsScene
    void redrawTile(QNetworkReply*);//draws tiles on the right position into graphicsScene

    void changezoom(int zoomvalue);//set zoom on
    void on_north_clicked();
    void on_east_clicked();
    void on_south_clicked();
    void on_west_clicked();

    void updateCountreply(QNetworkReply*);

    void on_showpath_clicked();
    void loadpath();
    void on_loadgtxt_clicked();
    void on_loadteams_clicked();
    void on_helpbutton_clicked();
};
#endif // MAINWINDOW_H

