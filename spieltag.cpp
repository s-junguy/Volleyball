#include"spieltag.h"
using namespace std;
Spieltag :: Spieltag(int h, int team2, int team3, int index){
			heimMannschaft = h;
			t2 = team2;
			t3 = team3;
			tupelIndex = index;
}

void Spieltag :: set_heimMannschaft(int h){
	heimMannschaft = h;
}

void Spieltag :: set_t2(int team2){
	t2 = team2;
}	

void Spieltag :: printSpieltag(){
	cout<<"index:"<<tupelIndex<<" ("<<heimMannschaft<<","<<t2<<","<<t3<<")"<<endl;
}

string Spieltag :: spieltagInString(){
    string spiel = " ("+to_string(heimMannschaft)+","+to_string(t2)+","+to_string(t3)+")";
    return spiel;
}
