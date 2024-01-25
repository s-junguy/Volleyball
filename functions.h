#ifndef functions_h
#define functions_h
#include<iostream>
#include<vector>
#include"spielstatistik.h"
#include"spieltag.h"
//sets vectors with all possible tuples, spielstatistik to zero, sum of numbers til n
//sum: entry= sum of index (like faculty)
//passing vector as parameter by reference so c++ doesnt make a copy of it
//vector range gives range of indices of first number from tupel
void gesTupel(int n, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int>& sum, vector<int>& range);

//team a, team b
//returns index of Spielstatistik (team a,team b)
int indexSpielstatistik(int a, int b, vector<int> sum, int n);

//under condition that a<b<c
//returns true if the 3 teams a,b,c didnt play more than one time with one another
//to check with Spielstatistik if tupel (a,b,c) can be used
bool checkSpielstatistik(int a, int b, int c, vector<Spielstatistik> stat, vector<int> sum, int n);

//set number of played matches in Spielstatistik for tuple (a,b,c) +1
void setSpielstatistikUp(int a, int b, int c, vector<Spielstatistik>& stat, vector<int> sum, int n);

//set number of played matches in Spielstatistik for tuple (a,b,c) -1
void setSpielstatistikDown(int a, int b, int c, vector<Spielstatistik>& stat, vector<int> sum, int n);

//returns true if stat[j].gespielt==2 for tuples with int first as heimMannschaft
bool checkSpielstatistikRange(int first, vector<Spielstatistik> stat, vector<int> sum, int n);

//returns true if stat[j].gespielt==2 for all tuples
bool spielstatistikVoll(vector<Spielstatistik> stat, vector<int> sum);

//returns first result at the beginning
//just checks wether tuple can be added to schedule with checkSpielstatistik
//stores first try for result even if stat not full
bool ersteTeilloesung(int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n);

void backtrackNecessary(Spieltag s, int first, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, int n);

bool backtrack(bool& teilloesungvorhanden, Spieltag s, int lasttupel, int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n);

//returns tupelIndex from last tuple in range first that can be included in schedule regarding checkSpielstatistik
//returns 0 if no tuple can be included
int lastTupel(int first, vector<Spieltag> tupel, vector<Spieltag> schedule, vector<Spielstatistik> stat, vector<int> sum, vector<int> range, int n);

//returns tupelIndex for first tuple in schedule with heimMannschaft==first
int tupelfirst(int first, vector<Spieltag> schedule);

void solve(int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n);

#endif
