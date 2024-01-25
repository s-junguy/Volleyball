#include"dataManager.h"
#include<sstream>
using namespace std;
DataManager :: DataManager(){
    //Open database
    int exit = sqlite3_open("teams.db", &DB);
    if(exit != SQLITE_OK){
        std::cout<<"Cannot open database: "<<sqlite3_errmsg(DB)<<std::endl;
    }
}



//returns false if Error (e.g. table already exists)
bool DataManager :: createTable(){
    std::string sql = "CREATE TABLE TEAMS("
                      "NR INT PRIMARY KEY     NOT NULL, "
                      "NAME           TEXT    NOT NULL, "
                      "ORT        CHAR(50), "
                      "BREITENGRAD DOUBLE NOT NULL, "
                      "LAENGENGRAD DOUBLE NOT NULL ); ";
    char* messageError;
    int exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);
  
    if (exit != SQLITE_OK) {
        sqlite3_free(messageError);
        return false;
    }
    return true;
}

//input: data set (for inserting data set in ui)
//returns false if Error
bool DataManager :: insert(std::string nr, std::string name, std::string ort, std::string breite, std::string laenge){
    char* messaggeError;
    std::string query = "SELECT * FROM TEAMS;";

    std::string sql("INSERT INTO TEAMS VALUES(" + nr + ", '" + name + "', '" + ort + "', " + breite + "," + laenge + ");");

    int exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK) {
        sqlite3_free(messaggeError);
        return false;
    }
    return true;
}

//input: team number (for inserting in ui)
//returns false if data set cannot be removed
bool DataManager :: remove(std::string teamnr){
    char* messageError;
    std::string sql = "SELECT NR FROM TEAMS WHERE NR = " + teamnr + ";";
    //if key doesnt exist -> give message
    //doesnt work with tableView
//    sqlite3_stmt *stmt;
//    if(sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0) != SQLITE_OK){
//
//        return false;
//    }
//    if(sqlite3_step(stmt) == SQLITE_DONE){//entry not found
//
//        return false;
//    }
    sql = "DELETE FROM TEAMS WHERE NR = " + teamnr + ";";
    sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);
    return true;
}

//returns string with whole table
std::string DataManager :: showData(){
    std::ostringstream oss;//stores data until converted to output string

    //Retrieve the Data from table
    string sql = "SELECT * FROM TEAMS";
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;
        return "";
    }

    //Iterate through rows of table, appending each column value to string
    while(sqlite3_step(stmt)==SQLITE_ROW){
        int numcol = sqlite3_column_count(stmt);
        for(int i=0; i < numcol; i++){
            oss << sqlite3_column_name(stmt,i);
            oss << " = ";
            oss << sqlite3_column_text(stmt,i);
            oss<<"\n";

        }
        oss << "\n";
    }
    std::string result = oss.str();
    return result;
}

double DataManager :: getlon(int key){
    ostringstream oss;
    string sql = "SELECT LAENGENGRAD FROM TEAMS WHERE NR = " + to_string(key);
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;

        return -1;
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        oss << sqlite3_column_text(stmt,0);
    }
    return stod(oss.str());
}

double DataManager :: getlat(int key){
    ostringstream oss;
    string sql = "SELECT BREITENGRAD FROM TEAMS WHERE NR = " + to_string(key);
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;
        return -1;
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        oss << sqlite3_column_text(stmt,0);
    }
    return stod(oss.str());
}

string DataManager :: getname(int key){
    ostringstream oss;
    string sql = "SELECT NAME FROM TEAMS WHERE NR = " + to_string(key);
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;
        return "";
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        oss << sqlite3_column_text(stmt,0);
    }
    return oss.str();
}

string DataManager :: getort(int key){
    ostringstream oss;
    string sql = "SELECT ORT FROM TEAMS WHERE NR = " + to_string(key);
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;

        return "";
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        oss << sqlite3_column_text(stmt,0);
    }
    return oss.str();
}

//vector key is empty
//we insert all the keys from the database table in there
void DataManager :: keysIntoVector(vector<int>& key){

    //Retrieve the Data from table
    std::string sql = "SELECT NR FROM TEAMS";
    sqlite3_stmt *stmt;//prepared statement
    int exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, 0);
    if(exit != SQLITE_OK){
        std::cout<<"Error preparing statement"<<sqlite3_errmsg(DB)<<std::endl;
    }

    //Iterate through rows of table, appending each column value to string
    while(sqlite3_step(stmt)==SQLITE_ROW){
        int nr = atoi((const char*)sqlite3_column_text(stmt,0));
        key.push_back(nr);
    }



}

void DataManager :: closedb(){
    sqlite3_close(DB);
}
