#include <iostream>
#include <string>
#include <set>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cstring>

//#define commandAppend " -vec-report=6 2> fin.txt"
#define commandAppend "2> fin.txt"
#define vecReportLevel "-vec-report=6"
#define optimizationLevel "-O3"

using namespace std;

int main(int argc,char* argv[]){
	
	//initialize variables
	unordered_map<string,set<string>> htabLines;
	string cmd,filename;
	ifstream infile;
	streampos sizeVecReport;
	char *memblock;
	string sourceFileName = "";

	cmd = "";
	vector<string> params(argv,argv+argc);
	
	if(argc == 1){
		cerr<<endl<<"Enter arguments"<<endl;
		return 0;
	}
	else{
		for(int i=1;i<params.size();i++){
                        string out;
                        transform(params[i].begin(),params[i].end(),back_inserter(out),::toupper);
			bool blC = (out.find(".C") != string::npos) ? true : false;
			bool blCPP = (out.find(".CPP") != string::npos) ? true : false;
                        if(out.find("-VEC-REPORT") != string::npos)
                                params[i] = vecReportLevel;
                        else if(out.find("-O") != string::npos)
                                params[i] = optimizationLevel;
			else if(blC || blCPP)
				sourceFileName = params[i];
                        cmd += params[i] + " ";
		}
		cmd += commandAppend;
		//cout<<endl<<"Command: "<<cmd;
	}


       //if(sourceFileName.find("/") != string::npos){
       //         sourceFileName = sourceFileName.substr(sourceFileName.find_last_of("/")+1,sourceFileName.size()-sourceFileName.find_last_of("/"));
       //}

	//cout<<endl<<"Source File: " <<sourceFileName;
				
	const char *c = cmd.c_str();
	system(c);

	//parse the vectorization report file into a vector of strings
	string line;
	infile.open("fin.txt",ios::ate);
	sizeVecReport=infile.tellg();
	memblock=new char[sizeVecReport];
	infile.seekg(0,ios::beg);
	infile.read(memblock,sizeVecReport);
	infile.close();
	string blob(memblock);
	
	vector<string> vecLines;
	istringstream completeString(blob);
	string tempCompleteLine;

	while(getline(completeString,tempCompleteLine,'\n')){
		vecLines.push_back(tempCompleteLine);
	}

	int ct=0;
	bool isMsg = false;
	
	//parse the vector of strings into a hashtable
	for(vector<string>::iterator it=vecLines.begin(); it!=vecLines.end();it++){
		ct++;
        	if((*it).find("error") != string::npos){
		        cerr<<endl<<*it;
                        cerr<<endl<<"!!!!!!!!!!ERROR!!!!!!!!!!!!!! at line:"<<ct<<endl<<"Please check the command"<<endl;
                        return 0;	
		}
			
             	else if((*it).find("warning") != string::npos)
                        continue;
                else{
			vector<string> vecTemp;
			istringstream partialString(*it);
			string tempPartialLine;
			while(getline(partialString,tempPartialLine,':')){
				vecTemp.push_back(tempPartialLine);	
			}
			
		        if(vecTemp.size() < 3){
				//cerr<<endl<<"Vectorization report line:"<<ct<<" has not been processed; "<< vecTemp.size() << " column(s) found"<<endl;
				//cerr<<"Check 'fin.txt' for more details"<<endl;
				if(!isMsg){
					cerr<<endl<<endl<<"The following line(s) from the vec report have not been processed:"<<endl;
					isMsg = true;
				}
				cerr<<endl<<*it;
			}
			
			else{
				string strFileName = vecTemp[0].substr(0,vecTemp[0].find_first_of("("));
       				//if(strFileName.find("/") != string::npos){
       			        //	strFileName = strFileName.substr(strFileName.find_last_of("/")+1,strFileName.size()-strFileName.find_last_of("/"));
       				//}
				string strLineNb = vecTemp[0].substr(vecTemp[0].find("(")+1,vecTemp[0].find(")")-vecTemp[0].find("(")-1);
				//int intLineNb = atoi(strLineNb.c_str());
				string strKey = strFileName + ";" + strLineNb;
				string strReason;
				for(int j=2;j<vecTemp.size();j++){
					strReason += vecTemp[j];
					if(j==vecTemp.size()-1)
						strReason+=";";
					else
						strReason+=" -";
				}
				if(htabLines.find(strKey) != htabLines.end()){
					//key exists
					//check if the reason exists. if no, add to the set
					set<string> reasons = htabLines.at(strKey);
					if(reasons.find(strReason) == reasons.end()){
						reasons.insert(strReason);	
					}
					htabLines.at(strKey) = reasons;				
				}
				else{
					//key does not exist
					//add key-value pair to the hashtable
					set<string> reasons;
					reasons.insert(strReason);
					htabLines.insert(make_pair<string,set<string>>(strKey,reasons));
				}
										
                	}
				
		}
	}
	cout<<endl;
	
	/*
	//sort the keys of the hashtable
	vector<int> keys;

	for(unordered_map<int,set<string>>::iterator itKeys=htabLines.begin();itKeys!=htabLines.end();itKeys++){
		int intFirstDigit = (it->keys).find_first_of(";")+1;
		string strLineNb = (itKeys->first).substr(intFirstDigit,strLineNb.size()-intFirstDigit);
		keys.push_back(atoi(strLineNb.c_str()));
	}
	sort(keys.begin(),keys.end());
	*/	

	//write to output file
        //for(vector<int>::iterator it=keys.begin();it!=keys.end();it++){
        for(unordered_map<string,set<string>>::iterator it=htabLines.begin();it!=htabLines.end();it++){
		int intIndexOfDelimiter = (it->first).find_first_of(";");
		string strFileName = (it->first).substr(0,intIndexOfDelimiter);
		string strLineNb = (it->first).substr(intIndexOfDelimiter+1,strLineNb.size()-intIndexOfDelimiter-1);
                cout<<"File "<<strFileName<<":Line "<<strLineNb<<":";
                set<string> reasons = htabLines.at(it->first);
		string strReason="";
                for(set<string>::iterator itReasons=reasons.begin();itReasons!=reasons.end();itReasons++){
                        strReason+=*itReasons;
                }
		string out;
		transform(strReason.begin(),strReason.end(),std::back_inserter(out),::toupper);
		bool blNV = (out.find("LOOP WAS NOT VECTORIZED") != string::npos) ? true : false; 
		bool blV = (out.find("LOOP WAS VECTORIZED") != string::npos) ? true : false;
		bool blPV = (out.find("PARTIAL LOOP WAS VECTORIZED") != string::npos) ? true : false;
		if((blNV && blV) || (blNV && blPV) || (blV && blPV))
			cout<<"CONFLICT:";
		else{
			if(blPV)
				cout<<"PARTIALLY VECTORIZED:";
			else if(blV)
				cout<<"VECTORIZED:";
			else
				cout<<"NOT VECTORIZED:";
		}
		cout<<strReason;
                cout<<endl;
        }
}
