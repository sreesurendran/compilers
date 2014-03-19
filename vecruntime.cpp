/* to run this code: 

./executable <fully_qualified_sourcefile> <fully_qualified_executable> <arguments_for_binary> 

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
#define loadMacpo "source /work/01174/ashay/apps/macpo-setup.sh"
#define runMacpoPrefix "macpo.sh --macpo:check-alignment="
#define loadPerfExpert "module load papi hpctoolkit perfexpert"
#define unloadPerfExpert "module unload perfexpert"
#define perfExpertExp "perfexpert_run_exp ./"
#define perfExpertAnalyzer "perfexpert_analyzer -t 0.1 -i "
#define perfExpertPath1 ":/opt/apps/perfexpert/4.1.1/bin"
#define perfExpertPath2 ":/opt/apps/papi/5.3.0/bin"
#define perfExpertPath3 ":/opt/apps/hpctoolkit/5.3.2/bin"
#define macpoPath1 ":/work/01174/ashay/apps/jdk1.7/bin" 
#define macpoPath2 ":/work/01174/ashay/apps/jdk1.7/jre/bin"
#define macpoPath3 ":/work/01174/ashay/apps/graphviz/bin"
#define macpoPath4 ":/work/01174/ashay/apps/opttran/bin/"

using namespace std;

unordered_map<string,set<string>> htabLines;
float MpoNVCt = 0;
float MpoPVCt = 0;
float INVCt = 0;
float IPVCt = 0;

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

string getConsoleOutput(string command){
	
	string result = "";
        FILE *pip = popen(command.c_str(),"r");
        if(pip == NULL)
                return result;
        while(!feof(pip)){
                char buffer[1024];
                while(fgets(buffer,sizeof(buffer),pip) != NULL){
			//buffer[sizeof(buffer)-1] = '\0';
                        result += buffer;
                }
        }
        pclose(pip);
	pip = NULL;
	return result;	
}

bool isMacpoAnalyzedLoop(string command,vector<string>& noMacpoAnalysis, string fileName, string lineNb){
	string result = getConsoleOutput(command);

	if(result.find("[macpo] Code at") != string::npos){
		size_t posLine  = result.find("[macpo] Code at");
		string line = result.substr(posLine);
		size_t posReason = line.find("could not");
		string reason = line.substr(posReason);
		reason = "File " + fileName + ":Line " + lineNb + ":" + reason;
		noMacpoAnalysis.push_back(reason);
	}
        string out;
        transform(result.begin(),result.end(),back_inserter(out),::toupper);
        if(out.find("[MACPO] ANALYZED CODE AT") != string::npos)
		return true;
	else
		return false;
	
}

int executeCommand(string cmd){
	const char *c = cmd.c_str();
	return system(c);
}

bool getHotLoops(string executableDetails,set<string>& hotLoops){

        string result = getConsoleOutput(string(perfExpertExp) + executableDetails);
        string xmlFileName = "";
        if(!result.empty()){
                vector<string> result_by_line = splitStringByDelimiter(result,'\n');
                for(int i=0;i<result_by_line.size();i++){
        		string out;
        		transform(result_by_line[i].begin(),result_by_line[i].end(),back_inserter(out),::toupper);
                        if(out.find("PERFEXPERT_ANALYZER") != string::npos){
                                vector<string> xmlFileLine = splitStringByDelimiter(result_by_line[i],' ');
                                if(xmlFileLine.size() >= 4)
                                        xmlFileName = xmlFileLine[3];
                                break;
                        }
                }
                if(xmlFileName != ""){
                        result = "";
                        string temp = string(perfExpertAnalyzer) + xmlFileName;
                        result = getConsoleOutput(temp);
                        vector<string> perfExpertOutput = splitStringByDelimiter(result,'\n');
                        for(auto ii=perfExpertOutput.begin();ii!=perfExpertOutput.end();ii++){
        			string out;
        			transform((*ii).begin(),(*ii).end(),back_inserter(out),::toupper);
                                if(out.find("LOOP IN FUNCTION") != string::npos){
                                        vector<string> hotLoopParts = splitStringByDelimiter(*ii,' ');
                                        for(int i=0;i<hotLoopParts.size();i++){
                                                if(hotLoopParts[i].find(":") != string::npos)
                                                        hotLoops.insert(hotLoopParts[i]);
                                        }
                                }
                        }

                }
		else
			return false;
        }
	else
		return false;
	return true;	
}

int main(int argc,char* argv[]){

       if(argc < 3){
                cout<<endl<<"Enter filename:"<<endl;
                return 0;
        }

        //initialize variables
	vector<string> params(argv,argc+argv);
	string cmd = "";
	int returnValue;
	string execName = "";
	for(int i=2;i<params.size();i++){
		execName += params[i] + " ";
	}
	
       	string out;
        transform(params[1].begin(),params[1].end(),back_inserter(out),::toupper);
        bool blC = (out.find(".C") != string::npos) ? true : false;
        bool blCPP = (out.find(".CPP") != string::npos) ? true : false;
 	out = "";	
	if(blC){
		out = ccompiler;
	}
	else if(blCPP)
		out = cpluspluscompiler;		
	cmd = out + params[1] + flags + vecReportLevel + "6 2> fip1.txt";
	returnValue = executeCommand(cmd);

	//cout<<endl<<"Cmd: "<<cmd;

	cmd = out + params[1] + flags + vecReportLevel + "7 2> fip2.txt";
	returnValue = executeCommand(cmd);		

	//cout<<endl<<"Cmd: "<<cmd;
	
	string line;
	parseVecReport("fip1.txt",true);
	parseVecReport("fip2.txt",false);	
	
	set<string> sourceFiles;
	vector<string> noMacpoAnalysisNV;
	vector<string> noMacpoAnalysisPV;
	set<string> macpoAnalysisNV;//of the format <filename(just_the_name)>:<linenb>

	if(htabLines.size() == 0){
		cout<<endl<<"Unable to process vector reports"<<endl;
		cout<<endl<<"Please check /fip1.txt and /fip2.txt"<<endl<<endl;
		return 0;
	}

  	for(unordered_map<string,set<string>>::iterator it=htabLines.begin();it!=htabLines.end();it++){
		set<string> reasons = htabLines.at(it->first);
                int intIndexOfDelimiter = (it->first).find_first_of(";");
                string strFileName = (it->first).substr(0,intIndexOfDelimiter);
                //string strLineNb = (it->first).substr(intIndexOfDelimiter+1,strLineNb.size()-intIndexOfDelimiter-1);
                string strLineNb = (it->first).substr(intIndexOfDelimiter+1,(it->first).size()-intIndexOfDelimiter-1);
		if(sourceFiles.find(strFileName) == sourceFiles.end()){
                	sourceFiles.insert(strFileName);
                }		
		string strReason = addCategory(reasons);
                cout<<"File "<<strFileName<<":Line "<<strLineNb<<":"<<strReason;
		if(strReason.substr(0,14) == "NOT VECTORIZED"){
			INVCt++;
       			string macpoCmd = string(runMacpoPrefix) + strFileName  + "#" + strLineNb + string(flags) + params[1];
        		if(isMacpoAnalyzedLoop(macpoCmd,noMacpoAnalysisNV,strFileName,strLineNb)){
				if(strFileName.find("/") != string::npos){
					size_t indexOfLastSlash = strFileName.find_last_of("/");
					macpoAnalysisNV.insert(strFileName.substr(indexOfLastSlash + 1) + ":" + strLineNb);
				}
				else
					macpoAnalysisNV.insert(strFileName+":"+strLineNb);
				MpoNVCt++;
			}
		}
		else if(strReason.substr(0,8) == "CONFLICT"){
			string out;
			transform(strReason.begin(),strReason.end(),back_inserter(out),::toupper);
			if(out.find("PARTIAL LOOP WAS VECTORIZED") != string::npos){
				IPVCt++;
                        	string macpoCmd = string(runMacpoPrefix) + strFileName + "#" + strLineNb + string(flags) + params[1];
                        	if(isMacpoAnalyzedLoop(macpoCmd,noMacpoAnalysisPV,strFileName,strLineNb)){
                                	if(strFileName.find("/") != string::npos){
                                        	size_t indexOfLastSlash = strFileName.find_last_of("/");
                                        	macpoAnalysisNV.insert(strFileName.substr(indexOfLastSlash + 1) + ":" + strLineNb);
                                	}
					else
						macpoAnalysisNV.insert(strFileName + ":" + strLineNb);
                        		MpoPVCt++;
                        	}
			}
		}
		
                cout<<endl;
        }

	
	cout<<endl<<endl;
	cout<<endl<<"Macpo can instrument "<<MpoNVCt << " out of "<< INVCt << " i.e " << (MpoNVCt/INVCt)*100 << "% non vectorized loops";
	cout<<endl<<"Macpo can instrument "<<MpoPVCt << " out of "<< IPVCt << " i.e " << (MpoPVCt/IPVCt)*100 << "% partially vectorized loops"<<endl;	
	
	if(noMacpoAnalysisNV.size() > 0 || noMacpoAnalysisPV.size() > 0)
		cout<<endl<<"INFORMATION (IF ANY) ON LOOPS NOT INSTRUMENTABLE BY MACPO"<<endl;
	
	if(noMacpoAnalysisNV.size() > 0){
		cout << endl << noMacpoAnalysisNV.size() << " out of " << (INVCt - MpoNVCt) << " loops (NOT VECTORIZED) that are not instrumentable by Macpo are listed as follows:" << endl << endl;
		for(auto ii=noMacpoAnalysisNV.begin();ii!=noMacpoAnalysisNV.end();ii++)
			cout<<*ii;
	}

        if(noMacpoAnalysisPV.size() > 0){
                cout << endl << noMacpoAnalysisPV.size() << " out of " << (IPVCt - MpoPVCt) << " loops (PARTIALLY VECTORIZED) that are not instrumentable by Macpo are listed as follows:"<< endl << endl;
                for(auto ii=noMacpoAnalysisPV.begin();ii!=noMacpoAnalysisPV.end();ii++)
                        cout<<*ii;
        }
	
	//set up environment variables to run perfexpert
         char* path = NULL;
	 if(path = getenv("PATH")){
	 	strcat(path,perfExpertPath1);
	        setenv("PATH",path,1);
	        strcat(path,perfExpertPath2);
	        setenv("PATH",path,1);
	        strcat(path,perfExpertPath3);
	        setenv("PATH",path,1);
	 }
	 path = NULL;
	
	//run perfexpert to find the hot loops 
	//check the percentage of hot loops out of the loops that can be instrumented by macpo 
	set<string> hotLoops;
	bool isPerfExpertEnabled = getHotLoops(execName,hotLoops);
	if(!isPerfExpertEnabled){
		cout<<endl<<"Check Perfexpert"<<endl;
		return 0;
	}

	cout<<endl<<"Instrumented by Macpo (NV+PV):"<<endl;
	for(auto ii=macpoAnalysisNV.begin();ii!=macpoAnalysisNV.end();ii++)
		cout<<*ii<<endl;

        cout<<endl<<"Hotloops:"<<endl;
        for(auto ii=hotLoops.begin();ii!=hotLoops.end();ii++)
                cout<<*ii<<endl;		
	
	vector<string> v1(macpoAnalysisNV.begin(),macpoAnalysisNV.end());
	vector<string> v2(hotLoops.begin(),hotLoops.end());
	vector<string> target(v1.size() + v2.size());
	sort(v1.begin(),v1.end());
	sort(v2.begin(),v2.end());
	auto it = std::set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),target.begin());
	target.resize(it - target.begin());

        cout<<endl<<"Hot Loops that are instrumentable by Macpo:"<<endl;
        for(auto ii=target.begin();ii!=target.end();ii++)
                cout<<*ii<<endl;	
	
	cout << endl << ((float)target.size()/(float)macpoAnalysisNV.size())*100 << "% loops instrumentable by Macpo are hotloops" << endl;
	
	//embed in source code
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
	
	cout<<endl;
	return(0);
 }
