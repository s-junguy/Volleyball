#ifndef spieltag_h
#define spieltag_h
#include<iostream>
#include<string>
using namespace std;

class Spieltag{
	public:
		int heimMannschaft;
		int t2;
		int t3;
		int tupelIndex;
		
		Spieltag(int h, int team2, int team3, int index);
		void set_heimMannschaft(int h);
		void set_t2( int team2);
		void printSpieltag();
        string spieltagInString();
		
};
#endif
