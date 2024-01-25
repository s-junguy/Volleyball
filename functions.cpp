#include"functions.h"
//sets vectors with all possible triples, spielstatistik to zero, sum of numbers til n
//sum: entry= sum of index (like faculty)
//passing vector as parameter by reference so c++ doesnt make a copy of it
//vector range gives range of indices of first number from tupel
void gesTupel(int n, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int>& sum, vector<int>& range){
    int sumNumber=0;
    sum.push_back(0);
    int index=0;
    int rangeIndex = 0;
    range.push_back(-1);
    for(int i=1; i<=n; i++){
        sumNumber += i;
        for(int j=i+1; j<=n; j++){
            for(int k=j+1; k<=n;k++){
                Spieltag s(i,j,k,index);
                tupel.push_back(s);
                rangeIndex++;
                index++;
            }
            Spielstatistik st(i,j,0);
            stat.push_back(st);
        }
        sum.push_back(sumNumber);
        if(i<=n-2)
            range.push_back(rangeIndex-1);
    }
}
//team a, team b, a<b
//returns index of Spielstatistik (team a,team b)
int indexSpielstatistik(int a, int b, vector<int> sum, int n){
    return n*(a-1)-sum[a]+b-1;
}

//under condition that a<b<c
//returns true if the 3 teams a,b,c didnt play more than one time with one another
//to check with Spielstatistik if tupel (a,b,c) can be used
bool checkSpielstatistik(int a, int b, int c, vector<Spielstatistik> stat, vector<int> sum, int n){
    if(stat[indexSpielstatistik(a,b,sum,n)].gespielt <2 && stat[indexSpielstatistik(b,c,sum,n)].gespielt <2 && stat[indexSpielstatistik(a,c,sum,n)].gespielt <2)
        return true;
    else
        return false;
}

//set number of played matches in Spielstatistik for triple (a,b,c) +1
void setSpielstatistikUp(int a, int b, int c, vector<Spielstatistik>& stat, vector<int> sum, int n){
    stat[indexSpielstatistik(a,b,sum,n)].setGespieltHigher();
    stat[indexSpielstatistik(b,c,sum,n)].setGespieltHigher();
    stat[indexSpielstatistik(a,c,sum,n)].setGespieltHigher();
}

//set number of played matches in Spielstatistik for triple (a,b,c) -1
void setSpielstatistikDown(int a, int b, int c, vector<Spielstatistik>& stat, vector<int> sum, int n){
    stat[indexSpielstatistik(a,b,sum,n)].setGespieltDown();
    stat[indexSpielstatistik(b,c,sum,n)].setGespieltDown();
    stat[indexSpielstatistik(a,c,sum,n)].setGespieltDown();
}

//returns true if stat[j].gespielt==2 for triples with int first as heimMannschaft
bool checkSpielstatistikRange(int first, vector<Spielstatistik> stat, vector<int> sum, int n){
        //begin: index from first in stat
        //check Spielstatistik for first
        for(int j=n*(first-1)-sum[first]+first; j<=n*(first-1)-sum[first]+n-1; j++){
            if(stat[j].gespielt != 2)
                return false;
        }
        return true;
}

//returns true if stat[j].gespielt==2 for all triples
bool spielstatistikVoll(vector<Spielstatistik> stat, vector<int> sum){
    for(int i=0; i<stat.size(); i++){
        if(stat[i].gespielt!=2)
            return false;
    }
    return true;
}



//returns first result at the beginning
//just checks wether triple can be added to schedule with checkSpielstatistik
//stores first try for result even if stat not full
bool ersteTeilloesung(int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n){
    //backtrack so teilloesung already exists
    if(schedule.size() !=0 && schedule.back().heimMannschaft == first) return false;
    for(int i=range[first-1]+1; i<=range[first]; i++){
        int a = tupel[i].heimMannschaft;
        int b = tupel[i].t2;
        int c = tupel[i].t3;
        if(checkSpielstatistik(a,b,c,stat,sum,n)){//if teams a,b,c didnt play >2 times with one another
            schedule.push_back(tupel[i]);
            setSpielstatistikUp(a,b,c,stat,sum,n);
            //check if we have a result for first
            if(checkSpielstatistikRange(first, stat, sum, n))
                return true;
        }
        if(first != 1 && c == n){
            if(stat[indexSpielstatistik(a,b,sum,n)].gespielt != 2){
                return false;
            }
        }
    }
    return false;
}

void backtrackNecessary(Spieltag s, int first, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, int n){
    for(int i=s.heimMannschaft+1; i<=s.t2; i++){
        if(stat[indexSpielstatistik(s.heimMannschaft,i,sum,n)].gespielt != 2){
            setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
            schedule.pop_back();
            break;
        }
    }
}

bool backtrack(bool& teilloesungvorhanden, Spieltag s, int lasttupel, int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n){
    //we got to the first triple of ersteTeilloesung -> no teilloesung
    if(s.tupelIndex == tupelfirst(first, schedule)){
        teilloesungvorhanden = false;
    }
    //last triple in range -> no need for backtracking, just delete from schedule
    if(s.tupelIndex == range[first]){
        setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
        schedule.pop_back();
        return false;
    }
    //last triple from range first that could be included in schedule with checkSpielstatistik -> no teilloesung
    if(s.tupelIndex == lasttupel && schedule[schedule.size()-2].heimMannschaft != first){
        teilloesungvorhanden = false;
        setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
        schedule.pop_back();
        return true;//for second while loop in solve to break
    }

    setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
    schedule.pop_back();//delete triple s from schedule because we backtrack from there

    int i = s.tupelIndex+1;
    while(i <= range[first]){
        if(checkSpielstatistik(tupel[i].heimMannschaft, tupel[i].t2, tupel[i].t3, stat, sum, n)){//check if spielstatistik till tupel is full
            schedule.push_back(tupel[i]);
            setSpielstatistikUp(tupel[i].heimMannschaft, tupel[i].t2, tupel[i].t3, stat, sum, n);
        }
        if(checkSpielstatistikRange(first, stat, sum, n)){
            return true;
        }
        i++;
    }
    return false;
}




//returns tupelIndex from last triple in range first that can be included in schedule regarding checkSpielstatistik
//returns 0 if no triple can be included
int lastTupel(int first, vector<Spieltag> tupel, vector<Spieltag> schedule, vector<Spielstatistik> stat, vector<int> sum, vector<int> range, int n){
    int lastTupel=0;
    for(int i=schedule.size()-1; i>=0; i--){
        if(schedule[i].heimMannschaft >= first){
            setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
            schedule.pop_back();
        }
        if(schedule[i].heimMannschaft < first) break;
    }
    for(int j=range[first]; j>=range[first-1]+1; j--){
        if(checkSpielstatistik(tupel[j].heimMannschaft, tupel[j].t2, tupel[j].t3, stat, sum, n)){
            lastTupel = tupel[j].tupelIndex;
            break;
        }
    }
    return lastTupel;
}

//returns tupelIndex for first triple in schedule with heimMannschaft==first
int tupelfirst(int first, vector<Spieltag> schedule){
    for(int i=0; i<schedule.size(); i++){
        if(schedule[i].heimMannschaft == first){
            return schedule[i].tupelIndex;
        }
    }
    return -1;
}


void solve(int first, vector<Spieltag>& tupel, vector<Spielstatistik>& stat, vector<int> sum, vector<Spieltag>& schedule, vector<int> range, int n){
    Spieltag s = tupel[0];
    bool teilloesungvorhanden = true;//marker to break backtrack loop
    int lasttupel = 0;//tupelIndex from last triple in range first that can be included in schedule regarding checkSpielstatistik
    while(first <= n-3){
        if(spielstatistikVoll(stat, sum)){//if we found valid schedule break
            break;
        }

        lasttupel = lastTupel(first, tupel, schedule, stat, sum, range, n);
        if(!ersteTeilloesung(first, tupel, stat, sum, schedule, range, n)){
            if(first != 1 && schedule.back().heimMannschaft == first-1){//no fitting triple added->no backtrack within teilloesung first
                first--;
            }else{
                teilloesungvorhanden = true;
                if(schedule.back().tupelIndex == range[first]){//last triple in schedule is last in range first->delete from schedule->no backtrack
                    setSpielstatistikDown(schedule.back().heimMannschaft,schedule.back().t2,schedule.back().t3,stat,sum,n);
                    schedule.pop_back();
                }
                s = schedule.back();
                while(true){//backtrack til first triple in schedule ersteTeilloesung or til we found teilloesung
                    if(teilloesungvorhanden == true && !backtrack(teilloesungvorhanden, s, lasttupel, first, tupel, stat, sum, schedule, range, n)){
                        if(s.t2 > first+1){
                            backtrackNecessary(s, first, stat, sum, schedule, n);
                        }
                        s = schedule.back();
                    }else{
                        break;
                    }
                }
                if(teilloesungvorhanden == false){//means that schedule.back() is first triple in ersteTeilloesung(first)
                    first--;
                }else{//first loop backtrack got a result
                    first++;
                }

            }
        }else{
            first++;//firstTeilloesung is a teilloesung->no backtrack needed
        }
    }
    if(checkSpielstatistik(tupel.back().heimMannschaft, tupel.back().t2, tupel.back().t3, stat, sum, n)){
        schedule.push_back(tupel.back());
        setSpielstatistikUp(tupel.back().heimMannschaft, tupel.back().t2, tupel.back().t3, stat, sum, n);
    }
}

