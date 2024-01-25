#ifndef spielstatistik_h
#define spielstatistik_h
#include<iostream>
using namespace std;

class Spielstatistik{
	public:
        int team1;
        int team2;
        int gespielt;
		
		Spielstatistik(int t1, int t2, int anzahl);
		
		void setGespieltHigher();
		void setGespieltDown();
		
		void printSpielstatistik();
};
#endif
		
		
