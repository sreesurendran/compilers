/* 

Perfexpert module for parsing intel vectorization reports (6 and 7) and embedding collated messages as comments to a copy of the original source code

Approach
a. Generate vec report 6 (with optimization level O3) for the given source file.
b. Generate vec report 7 (with optimization level O3) for the given source file.
c. Parse the reports from (a) and (b) and collate messages for every source line mentioned in both the reports. Note that for vec report 7, the messages will be translated to their 'English' forms.
d. For every source line mentioned in both the reports, embed the collated message from (c) as a comment into a copy of the original source file.
e. Print the consolidated report on stdout

Invocation
a. the current user should have read-write access to the current directory
b. remove <source_file_name>_out.<source_file_extension> (if any) from the current directory
c. remove fip1.txt (if any) from the current directory
d. remove fip2.txt (if any) from the current directory
c. run ./a.out <fully_qualified_source_file>

Input (Remarks)
name of the source file, for eg. main.c
The default optimization level is O3. Possible input source file formats are .c and .cpp.

Output (Remarks)
a. printed on stdout in the following format:
    File <filename>:Line <line_no>:<Category>:<Reasons>
	where 
		<Category> can be 'CONFLICT', 'VECTORIZED' or 'NOT VECTORIZED' 
		<Reasons> will be a "; delimited" string of all messages for line <line_no> from vec reports 6 and 7 (combined)
b. a new annotated source file, for eg. main_out.c created in the current directory

Temporary files created (in the current directory)
a. fip1.txt - vec report 6
b. fip2.txt - vec report 7

*/

/* Changes to be made
 * a. replace system() with exec()
 * b. put dictionary for vec report 7 messages in a separate text file
 * b. for the _temp and _out file created, give file i/o permissions to the current user (is this requried? -- check)
 * c. make sure there's no issue if the source file is located in a different directory
 * d. test edge cases
 */


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
#include <cstddef>
#include "stdlib.h"

#define vecReportLevel "-vec-report="
#define optimizationLevel "-O3"
#define flags " -c -O3 -g "
#define ccompiler "icc "
#define cpluspluscompiler "icpc "

using namespace std;

unordered_map<string,set<string>> htabLines;

unordered_map<string,string> htabVecMessages = {
                {"VEC#00000" , "vectorization report for function - "},
                {"VEC#00001NPNR" , "loop was vectorized (no peel/no remainder)"},
                {"VEC#00001NPWR" , "loop was vectorized (no peel/with remainder)"},
                {"VEC#00001WPNR" , "loop was vectorized (with peel/no remainder)"},
                {"VEC#00001WPWR" , "loop was vectorized (with peel/with remainder)"},
                {"VEC#00002" , "loop was not vectorized -  "},
                {"VEC#00003V" , "peel loop was vectorized"},
                {"VEC#00003S" , "peel loop was not vectorized - "},
                {"VEC#00004V" , "remainder loop was vectorized (unmasked)"},
                {"VEC#00004M" , "remainder loop was vectorized (masked)"},
                {"VEC#00004S" , "remainder loop was not vectorized - "},
                {"VEC#00005" , "entire loop may be executed in scalar remainder"},
                {"VEC#00050" , "--- begin vector loop hierarchy summary ---"},
                {"VEC#00051" , "vectorized loop at nesting level - "},
                {"VEC#00052" , "loop inside vectorized loop at nesting level - "},
                {"VEC#00053" , "--- end vector loop hierarchy summary ---"},
                {"VEC#00100" , "--- begin vector loop memory reference summary ---"},
                {"VEC#00101UASL" , "unmasked aligned unit stride loads - "},
                {"VEC#00101UASS" , "unmasked aligned unit stride stores - "},
                {"VEC#00101UUSL" , "unmasked unaligned unit stride loads - "},
                {"VEC#00101UUSS" , "unmasked unaligned unit stride stores - "},
                {"VEC#00101USL" , "unmasked strided loads - "},
                {"VEC#00101USS" , "unmasked strided stores - "},
                {"VEC#00101MASL" , "masked aligned unit stride loads - "},
                {"VEC#00101MASS" , "masked aligned unit stride stores - "},
                {"VEC#00101MUSL" , "masked unaligned unit stride loads - "},
                {"VEC#00101MUSS" , "masked unaligned unit stride stores - "},
                {"VEC#00102MG" , "masked indexed (or gather) loads - "},
                {"VEC#00102MS" , "masked indexed (or scatter) stores - "},
                {"VEC#00101MSL" , "masked strided loads - "},
                {"VEC#00101MSS" , "masked strided stores - "},
                {"VEC#00102UG" , "unmasked indexed (or gather) loads - "},
                {"VEC#00102UL" , "unmasked indexed (or scatter) stores - "},
                {"VEC#00103UBL" , "unmasked broadcast loads - "},
                {"VEC#00103MB" , "masked broadcast loads - "},
                {"VEC#00104UANTL" , "unmasked aligned streaming loads - "},
                {"VEC#00104UANTS" , "unmasked aligned streaming stores - "},
                {"VEC#00104UUNTL" , "unmasked unaligned streaming loads - "},
                {"VEC#00104UUNTS" , "unmasked unaligned streaming stores - "},
                {"VEC#00104MASTL" , "masked aligned streaming loads - "},
                {"VEC#00104MASTS" , "masked aligned streaming stores - "},
                {"VEC#00104MUSTL" , "masked unaligned streaming loads - "},
                {"VEC#00104MUSTS" , "masked unaligned streaming stores - "},
                {"VEC#00105" , "--- end vector loop memory reference summary ---"},
                {"VEC#00200" , "--- begin vector loop cost summary ---"},
                {"VEC#00201" , "scalar loop cost - "},
                {"VEC#00202" , "vector loop cost - "},
                {"VEC#00203" , "estimated potential speedup - "},
                {"VEC#00204" , "lightweight vector operations - "},
                {"VEC#00205" , "medium-overhead vector operations - "},
                {"VEC#00206" , "heavy-overhead vector operations - "},
                {"VEC#00207" , "vectorized math library calls - "},
                {"VEC#00208" , "non-vectorized math library calls - "},
                {"VEC#00209" , "vectorized user function calls - "},
                {"VEC#00210" , "non-vectorized user function calls - "},
                {"VEC#00212" , "divides - "},
                {"VEC#00213" , "type converts - "},
                {"VEC#00299" , "--- end vector loop cost summary ---"},
                {"VEC#00300" , "--- begin vector function matching report ---"},
                {"VEC#00301" , "function call site - "},
                {"VEC#00302" , "non-vectorized user function calls with mismatched vector functions - "},
                {"VEC#00303" , "--- end vector function matching report ---"},
                {"VEC#00400" , "--- begin vector idiom recognition report ---"},
                {"VEC#00401" , "minimum value and minimum value loop index - "},
                {"VEC#00402" , "maximum value and maximum value loop index - "},
                {"VEC#00403" , "vector compress - "},
                {"VEC#00404" , "vector expand - "},
                {"VEC#00405" , "histogram - "},
                {"VEC#00405" , "saturating downconvert - "},
                {"VEC#00405" , "saturating add/subtract - "},
                {"VEC#00406" , "byte permute - "},
                {"VEC#00499" , "--- end vector idiom recognition report ---"},
                {"VEC#01000" , "--- last debug string ---"},
                {"VEC#99999" , "end"}
};

string trimWhiteSpaces(string str){
	string whitespaces(" \t\f\v\n\r");
	std::size_t found = str.find_first_not_of(whitespaces);
	if(found != string::npos)
		str = str.erase(0,found);
	found = str.find_last_not_of(whitespaces);
        if(found != string::npos)
                str = str.erase(found+1);	
	return str;
}

vector<string> splitStringByDelimiter(string str,char delim){
	vector<string> vecTemp;
        istringstream partialString(str);
        string tempPartialLine;
        while(getline(partialString,tempPartialLine,delim)){
        	vecTemp.push_back(tempPartialLine);
        }
	return vecTemp;	
}

void parseVecReport(string fileName,bool isVec6){
	string line;
	bool isMsg = false;
	ifstream infile(fileName);
	if(infile.is_open()){
		while(getline(infile,line)){
			if(line.find("error") != string::npos){
				cout<<endl<<line;
				infile.close();
				exit(0);
			}		
			else if(line.find("warning") != string::npos)
				continue;
			else{
              			vector<string> vecTemp = splitStringByDelimiter(line,':');
				if(vecTemp.size() < 3){
                  			if(!isMsg){
                                        	cerr<<endl<<endl<<"The following line(s) from the vec report have not been processed:"<<endl;
                                        	isMsg = true;
                                	}
                                	cerr<<endl<<line;				
				}
				else{
					string strFileName = vecTemp[0].substr(0,vecTemp[0].find_first_of("("));
					string strLineNb = vecTemp[0].substr(vecTemp[0].find("(")+1,vecTemp[0].find(")")-vecTemp[0].find("(")-1);
					string strKey = strFileName + ";" + strLineNb;
                                	string strReason;
                                	for(int j=2;j<vecTemp.size();j++){
						if(isVec6){
							strReason += trimWhiteSpaces(vecTemp[j]);
						}
						else{
							string trimmedString = trimWhiteSpaces(vecTemp[j]);
							vector<string> vecInner = splitStringByDelimiter(trimmedString,' ');
							if(vecInner.size() > 0){
								if(htabVecMessages.find(vecInner[0]) != htabVecMessages.end()){
									strReason += htabVecMessages.at(vecInner[0]);
									if(vecInner.size() == 2)
										strReason += vecInner[1];	
								}
								else{
									strReason += trimmedString;
								}
							}
						}
                                        	if(j==vecTemp.size()-1)
                                                	strReason+=";";
                                        	else
                                                	strReason+=" - ";
                                	}
					if(htabLines.find(strKey) != htabLines.end()){
						set<string> reasons = htabLines.at(strKey);
						if(reasons.find(strReason) == reasons.end()){
							reasons.insert(strReason);
						}
						htabLines.at(strKey) = reasons;
					}				
					else{
						set<string> reasons;
						reasons.insert(strReason);
						htabLines.insert(make_pair<string,set<string>>(strKey,reasons));
					}							
				}
			}	
		}
		infile.close();
	}		
}

string addCategory(set<string> reasons){
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
        	strReason = "CONFLICT:" + strReason;
        else{
        	if(blPV)
                	strReason = "PARTIALLY VECTORIZED:" + strReason;
                else if(blV)
                	strReason = "VECTORIZED:" + strReason;
                else
                	strReason = "NOT VECTORIZED:" + strReason;
        }
        return strReason;	
}

int executeCommand(string cmd){
	const char *c = cmd.c_str();
	return system(c);
}

void display_on_console(set<string>& sourceFiles){
	for(unordered_map<string,set<string>>::iterator it=htabLines.begin();it!=htabLines.end();it++){
                set<string> reasons = htabLines.at(it->first);
                int intIndexOfDelimiter = (it->first).find_first_of(";");
                string strFileName = (it->first).substr(0,intIndexOfDelimiter);
                string strLineNb = (it->first).substr(intIndexOfDelimiter+1,(it->first).size()-intIndexOfDelimiter-1);
                if(sourceFiles.find(strFileName) == sourceFiles.end()){
                        sourceFiles.insert(strFileName);
                }
                string strReason = addCategory(reasons);
                cout<<"File "<<strFileName<<":Line "<<strLineNb<<":"<<strReason;
                cout<<endl;
        }
}

void embed_in_soource_code(set<string>& sourceFiles,int returnValue){
        for(set<string>::iterator it=sourceFiles.begin();it!=sourceFiles.end();it++){
                ifstream infile(*it);
                int intIndexOfDelimiter = (*it).find_first_of(".");
                string rawFileName = (*it).substr(0,intIndexOfDelimiter);
                string fileExtension = (*it).substr(intIndexOfDelimiter+1,(*it).size()-intIndexOfDelimiter-1);
                string tempOutputFileName = rawFileName + "_temp." + fileExtension;
                ofstream outfile(tempOutputFileName);
                int ct = 0;
                string line;
                if(infile.is_open()){
                        while(getline(infile,line)){
                                ct++;
                                string strCount = std::to_string(static_cast<long long>(ct));
                                string strKey = (*it) + ";" + strCount;
                                if(htabLines.find(strKey) != htabLines.end()){
                                        set<string> reasons = htabLines.at(strKey);
                                        string sourceLine = line + "/* " + addCategory(reasons) + " */";
                                        outfile<<sourceLine<<endl;
                                }
                                else{
                                        outfile<<line<<endl;
                                }

                        }
                        infile.close();
                        outfile.close();
                }
                string finalOutputFileName = rawFileName + "_out." + fileExtension;
                string cmdRemoveLineFeedCharacters = "sed -e 's/\r//g' " + tempOutputFileName + " > " + finalOutputFileName;
                returnValue = executeCommand(cmdRemoveLineFeedCharacters);
                returnValue = executeCommand("rm " + tempOutputFileName);
        }
}

int main(int argc,char* argv[]){

	//if the command line is not of the form ./a.out <fully_qualified_source_file>, return
       if(argc < 2){
                cout<<endl<<"Enter filename:"<<endl;
                return 0;
        }

        //initialize variables
	vector<string> params(argv,argc+argv);
	string cmd = "";
	int returnValue;	
       	string out;

	//check if the source file is a .c or .cpp file to invoke the corresponding compiler
        transform(params[1].begin(),params[1].end(),back_inserter(out),::toupper);
        bool blC = (out.find(".C") != string::npos) ? true : false;
        bool blCPP = (out.find(".CPP") != string::npos) ? true : false;
 	out = "";	
	if(blC){
		out = ccompiler;
	}
	else if(blCPP)
		out = cpluspluscompiler;		

	//generate vec report 6 and write into fip1.txt
	cmd = out + params[1] + flags + vecReportLevel + "6 2> fip1.txt";
	returnValue = executeCommand(cmd);

	//generate vec report 7 and write into fip2.txt
	cmd = out + params[1] + flags + vecReportLevel + "7 2> fip2.txt";
	returnValue = executeCommand(cmd);		

	//parse vec report 6
	parseVecReport("fip1.txt",true);
	//parse vec report 7
	parseVecReport("fip2.txt",false);	
	
	//if the vec reports 6 and 7 could not be parsed, return
	if(htabLines.size() == 0){
		cout<<endl<<"Unable to process vector reports"<<endl;
		cout<<endl<<"Please check /fip1.txt and /fip2.txt"<<endl<<endl;
		return 0;
	}

	//print to stdout
	set<string> sourceFiles;
	display_on_console(sourceFiles);

	//embed in source code
	embed_in_soource_code(sourceFiles,returnValue);

	cout<<endl;
	return(0);
 }
