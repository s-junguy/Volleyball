// Author: Timon Kapischke
// Last modified on 15.12.2020
// Projekt: Routenplaner
// Version: 2.0

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include "dijkstra.h"
#include "Timer.h"

using namespace std;

Dijkstra::Dijkstra(string s){
    Timer time1;
	time1.start();	
	einlesen(s);		// lese Graph ein
	time1.stop();
    cout << "Zeit zum Einlesen: " << time1.secs() << "s" << endl;
}

void Dijkstra::berechne_dijkstra(int start, int ziel){
	
    startKnoten = start;    // lege Startknoten fest
	zielKnoten = ziel;      // lege Zielknoten fest

    for (unsigned int i = 0; i < AnzahlKnoten; i++) {
        weg.push_back(make_pair(-1, -1));   // setze bei alle Knoten den Abstand und die Herkunft auf -1
        fertig.push_back(false);            // setze alle Knoten auf "unbearbeitet"
   	}

	weg[startKnoten].first = 0; 			// setze Abstand des Startknoten auf 0  
	pq.push(make_pair(startKnoten, 0)); 	// füge Startknoten mit Abstand 0 in eine Priority Quere ein

	Timer time;
	time.start();

	while (!pq.empty()) {
        pair<int, long int> minKnoten = pq.top();   // Knoten aus Priority Quere mit Knotennummer und minimalem Abstand zum Startknoten;
        pq.pop();                                   // entferne Knoten obersten Knoten (mit minimalen Abstand) aus der Priority Quere
        if (fertig[minKnoten.first] == false) {     // betrachten nur die Knoten, welcher der Algorithmus noch nicht abgearbeitet hat
			if(minKnoten.first==zielKnoten)
                break;                              // falls der Zielknoten gefunden ist, soll abgebrochen werden		
			vector<pair<int, long int> > nachbar = adijazent(minKnoten.first);  // Vektor mit allen adijazenten Knoten und dessen Kantengewichten
            for (unsigned int i = 0; i < nachbar.size(); i++) {
                long int neuerAbstand = minKnoten.second + nachbar[i].second;   // Addiere aktuelles Gewicht + Gewicht der Kante
				if (weg[nachbar[i].first].first == -1 || weg[nachbar[i].first].first > neuerAbstand) {   // Falls der alte Abstand groeßer als der neue Abstand ist
					weg[nachbar[i].first].first = neuerAbstand;	    		// setze alten Abstand des Nachbarknoten auf den neuen Abstand
                    weg[nachbar[i].first].second = minKnoten.first;         // setze seine Herkunft auf minKnoten
                    pq.push(make_pair(nachbar[i].first, neuerAbstand));     // füge aktualisierten Nachbarknoten in die Priority Quere ein
				}
			}
            fertig[minKnoten.first] = true;     // setze den Knoten auf bearbeitet
		}
	}
	weg.clear();
	fertig.clear();
	while (!pq.empty()) { // leert Priority Quere
		pq.pop();
	}

	time.stop();
    cout << "Zeit zum Berechnen des Weges: " <<time.secs() << "s" << endl;
}

vector<pair<double, double> > Dijkstra::findeWeg(){
    weglaenge = weg[zielKnoten].first;      // kuerzester Weg
    vector<pair<double, double> > wegCoord; // Wegkoordinaten
	if (weglaenge == -1) {
		cout << "Startknoten: " << startKnoten << " Zielknoten: " << zielKnoten << " Weglaenge: " << weglaenge << endl;
		cout << "Es konnte leider kein Weg gefunden werden.\n" << endl;
	} else {
        vector<int> wegKnoten;                      // enthält Knotennummern des kuerzesten Weges
		wegKnoten.push_back(zielKnoten);
        int knoten = zielKnoten;                    // aktueller Knoten
        while (knoten != startKnoten) {             // gehe bis zum Anfang zurueck
			wegKnoten.push_back(weg[knoten].second);// fuege Knoten zum kuerzesten Weg hinzu
			knoten = weg[knoten].second; 			// aktualisiere Knoten
		}

        for (unsigned int i = 0; i < wegKnoten.size(); i++) {
            wegCoord.push_back(coordlist[wegKnoten[i]]); // erstellt die Koordinaten des kuerzesten Weges
		}

		cout << "Startknoten: " << startKnoten << " Zielknoten: " << zielKnoten << " Weglaenge: " << weglaenge << endl;
/*		
		cout.precision(9);
		cout << "Kuerzester Weg von " << startKnoten << " zu Knoten " << zielKnoten << " ist:" << endl;
        for (unsigned int i = wegKnoten.size() - 1; i >= 0; i--) {
			cout << "(" << wegKnoten[i] << "," << wegCoord[i].first << "," << wegCoord[i].second << ") ";
		}
		cout << endl;
*/		
	}
	return(wegCoord);
}

int Dijkstra::returnWeglaenge(){
    return weglaenge;
}

// Methoden für den internen Graph
void Dijkstra::einlesen(string s){
	int quellKnoten, zielKnoten;    // Quell und Zielknoten der Kanten,
	long int gewicht;               // Gewicht einer Kante
	int alter_quellKnoten = -1;     // Quellknoten der Vorgängerkante
	double xCoord, yCoord;          // X- und Y-Koordinate eines Knotens
	
	ifstream file(s.c_str());
    if (!file.is_open()) {  // falls Datei nicht geoeffnet werden konnte
        cerr << "Fehler beim oeffnen der Datei" << s << endl;
        throw invalid_argument(s);
	}	

	file >> AnzahlKnoten >> AnzahlKanten;   // liest die Anzahl der Knoten und Kanten ein
	for (unsigned int i = 0; i < AnzahlKnoten; i++) {   // liest die Knotenkoordinaten ein
		file >> xCoord >> yCoord;		
		coordlist.push_back(make_pair(xCoord, yCoord)); // fuege Koordinaten hinzu
    }  
	
	while (file >> quellKnoten >> zielKnoten >> gewicht) {  // lese Kanten ein
        if (quellKnoten != alter_quellKnoten) {             // vergleicht, ob sich der Quellknoten geaendert hat
			while (quellKnoten != alter_quellKnoten + 1) {  // Fall falls der Knoten keine ausgehende Kante hat
                edgeOffsets.push_back(-1);                  // falls keine Kante vorhanden fuege -1 in edgeOffsets hinzu
                alter_quellKnoten++;								
			}
            edgeOffsets.push_back(edgelist.size());         // fuege edglisten.size zu edgeOffset hinzu
            alter_quellKnoten = quellKnoten;                // aktualisiere quellKnoten
		}	
		edgelist.push_back(make_pair(zielKnoten, gewicht)); // fuege neue Kanten hinzu
	}
	file.close();

	while ((unsigned)quellKnoten != coordlist.size() - 1) { // Fall falls die letzten Knoten des Graphen keine ausgehenden Kanten haben
        edgeOffsets.push_back(-1);                          // fuege -1 in edgeOffsets hinzu
		quellKnoten++;
	}
}

vector<pair<int,long int> > Dijkstra::adijazent(int nummer){   // gibt alle adijanzenten Kanten zu einem Knoten als Vektor zurueck
    unsigned int start = edgeOffsets[nummer];               // Startposition der adijazenten Kanten in edgelist
    unsigned int ende = 0;                                  // Endposition der adijazenten Kanten in edgelist
    vector<pair<int, long int> > adijazent;                 // Ausgabevektor der adijazenten Knoten und dessen Kantengewicht
	
    if (start != -1) {                                      // Falls der aktuelle Knoten eine ausgehende Kante besitzt
        if ((unsigned)nummer == edgeOffsets.size() - 1) {   // Fall für den letzten Knoten
			ende=edgelist.size()-1;		
		} else {
            if (edgeOffsets[nummer + 1] != -1) {            // Fall falls Kanten vorhanden sind
			    ende = edgeOffsets[nummer + 1] - 1;			
		    } else { 
                unsigned int x = 0;                         // Anzahl der Knoten ohne ausgehende Kanten
				while (edgeOffsets[nummer + x + 1] == -1) {
					x++;
				}
                if ((unsigned)nummer + x != edgeOffsets.size() - 1) { // Fall falls mittendrin Knoten keine ausgehenden Kanten haben
					ende = edgeOffsets[nummer + x + 1] - 1;
                } else {                                // Falls die letzten Knoten keine ausgehenden Kanten haben
				 	ende = edgelist.size() - 1;
				}
			}						
		}
        for (unsigned int i = start; i <= ende; i++) {  // fuege alle Kanten den Ausgabevektor hinzu
            adijazent.push_back(edgelist[i]);
		}
	}
/*
	cout << "Adijazente Kanten und Knoten zu Knoten " << nummer << " sind: " << endl;
    for (unsigned int i = 0; i < adijazent.size(); i++) {
		cout << "Zielknoten: " << adijazent[i].first << " Kantengewicht: " << adijazent[i].second << endl;
	}
	cout << endl;
*/
	return(adijazent);
}

double Dijkstra::abstand(pair<double, double> a, pair<double, double> b){ //brechnet den euklidischen Abstand
    double euk = sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
    return euk;
}

pair<double, double> Dijkstra::returnKnoten(int i){ // Gibt i-ten Knoten zurueck
    pair<double, double> Knoten;
    Knoten = make_pair(coordlist[i].first, coordlist[i].second);
    return Knoten;
}

int Dijkstra::returnNextKnoten(pair<double, double> Knoten){ // gibt die Knotennummer des naechstgelegenen Knotens aus
    pair<double, double> aktuell;   // Knoten der aktuell bearbeitet wird
    pair<double, double> minimum;   // Knoten mit minimalen Abstand
    int ergebnis = 0;               // Knotennummer des minimalen Knotens in coordlist

    for (unsigned int i = 0; i < coordlist.size(); i++) {
        aktuell = make_pair(coordlist[i].first, coordlist[i].second);
        if (abstand(Knoten, aktuell) < abstand(Knoten, minimum)) {
            minimum = aktuell;
            ergebnis = i;
        }
    }
    return ergebnis;
}
