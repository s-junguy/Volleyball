#include"spielstatistik.h"

Spielstatistik :: Spielstatistik(int t1, int t2, int anzahl){
	team1 = t1;
	team2 = t2;
	gespielt = anzahl;
}	

void Spielstatistik :: setGespieltHigher(){
	gespielt += 1;
}

void Spielstatistik :: setGespieltDown(){
	gespielt -= 1;
}

void Spielstatistik :: printSpielstatistik(){
	cout<<"("<<team1<<","<<team2<<")->"<<gespielt<<endl;
}
