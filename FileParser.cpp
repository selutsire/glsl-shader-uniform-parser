#include "FileParser.h"

#include <unordered_set>
#include <cctype>

void FileParser::getNextWord(const char* string, 
                            unsigned int index,
                            int& beginTokenHolder,
                            int& endTokenHolder,
                            int& braceDepthHolder,
                            int& parenthesisDepthHolder,
                            bool& newExpressionHolder,
                            bool& equalDetectedHolder,
                            bool excludeComments){
    
    unsigned int stringBufferIndex = index-1;
    char character;
    int beginTokenIndex = -1;
    int endTokenIndex = -1;
    int braceDepth = braceDepthHolder;
    int parenthesisDepth = parenthesisDepthHolder;
    bool isComment = false;
    bool isBlockComment = false;
    bool newExpression = false;
    bool equalDetected = false;

    do{
        
        character = string[++stringBufferIndex];
        //are we excluding comments? are we in a comment? 
        if(excludeComments && isComment){
            if(isBlockComment){
                if(character == '*' && string[stringBufferIndex+1] == '/'){
                    isComment = false;
                    isBlockComment = false;
                    stringBufferIndex++;
                }
            }else if(character == '\n' || character == '\r'){
                isComment = false;
                if(character == '\r' && string[stringBufferIndex+1] == '\n'){
                    stringBufferIndex++;
                }
            }
            continue;
        }

        //if it isnt a letter or number char, has a token been detected yet? 
        //if there is a beginning index, yes. set endTokenIndex and finish
        if(!isLetterOrNum(character)){

            if(beginTokenIndex < 0){
                switch(character){
                    case '{':
                        braceDepth++;
                    break;
                    case '}':
                        braceDepth--;
                    break;
                    case '(':
                        parenthesisDepth++;
                    break;
                    case ')':
                        parenthesisDepth--;
                    break;
                    case '=':
                        equalDetected = true;
                    break;
                    case ';':
                        newExpression = true;
                    break;
                }
            }
            
            if(beginTokenIndex >= 0 ){
                endTokenIndex = stringBufferIndex;
                break;
            }

            //check if we have hit a comment.
            if(excludeComments && character == '/'){
                if(string[stringBufferIndex+1] == '/'){
                    isComment = true;
                }
                else if(string[stringBufferIndex+1] == '*'){
                    isComment = true;
                    isBlockComment = true;
                }
            }

        }
        //if the character isnt a delimiter
        //if we havent found a token yet, set beginTokenIndex to the beginning of the token
        else if(beginTokenIndex < 0){
            beginTokenIndex = stringBufferIndex;
        }

    }while(character != '\0');

    if(character == '\0'){
        //if reached null char (end of string)
        //if we have detected a token already, set the index to the last char since this one is null
        //else set beginTokenIndex to -1 to signal end of string
        if(beginTokenIndex >= 0){
            endTokenIndex = stringBufferIndex;
        }
    }

    beginTokenHolder = beginTokenIndex;
    endTokenHolder = endTokenIndex;
    braceDepthHolder = braceDepth;
    parenthesisDepthHolder = parenthesisDepth;
    newExpressionHolder = newExpression;
    equalDetectedHolder = equalDetected;
        
}

bool FileParser::isLetterOrNum(char character){
    if (character == '_') return true;
    if(character <= 0x2F  ||
        (character >= 0x3A  && character <= 0x40) ||
        (character >= 0x5B  && character <= 0x60) ||
         character >= 0x7B){
        return false;
    }
    return true;
}

void FileParser::parseGLSLUniforms(const std::string& sourceCode, std::vector<std::string>& uniformVector){
    const char* string = sourceCode.c_str();
    
    enum GLSLParserState {
        DEFAULT,
        UNIFORM_DETECTED,
        STRUCT_DETECTED,
        STRUCT_INSTANCE_DETECTED,
        STRUCT_INNER_STRUCT_DETECTED
    };

    int beginTokenIndex = 0;
    int endTokenIndex;
    int index = 0;

    int braceDepth = 0;
    int braceDepthRecord = 0;

    int parenthesisDepth = 0;
    int parenthesisDepthRecord = 0;

    int currentStructIndex;
    int innerCurrentStructIndex;

    bool structNameRecorded = false;
    bool newExpression = false;
    bool equalDetected = false;

    GLSLParserState state = DEFAULT;
    
    std::unordered_map<std::string, unsigned int> structMap;
    std::vector<std::vector<std::string>> structMemberList;
    std::vector<std::string> structList;
    std::string newString = "";

    while(beginTokenIndex >= 0){
        
        begin_switch:
        switch(state){
            case UNIFORM_DETECTED://uniform detected, this token will either be a type or a symbol
                if(equalDetected){
                    state = DEFAULT;
                }
                else if(newExpression) {
                    state = DEFAULT;
                    goto begin_switch;

                }else if (braceDepth != braceDepthRecord){
                    state = DEFAULT; //if brace depth changed, parser is inside a ubo, dont need to keep parsing
                }
                if(structMap.find(newString) != structMap.end()){//if its a known struct type, we found an instance
                    currentStructIndex = structMap[newString];
                    state = STRUCT_INSTANCE_DETECTED;

                }else if( (glslkeywords.count(newString) == 0)){ 
                    uniformVector.push_back(newString);
                }else if(newString == "struct"){
                    state = STRUCT_DETECTED;
                }
            break;
            
            case STRUCT_DETECTED:
                if(state != STRUCT_DETECTED ) break;
                if(!structNameRecorded){ //getting the name of the type of the struct first

                    currentStructIndex = structList.size();
                    structMap[newString] = structList.size();
                    structList.push_back(newString);
                    structMemberList.push_back(std::vector<std::string>());
                    structNameRecorded = true;

                }else if(braceDepth != braceDepthRecord){

                    if(auto it = structMap.find(newString); it != structMap.end()){ //if prev found struct here, add its members to this struct
                        innerCurrentStructIndex = it->second;
                        state = STRUCT_INNER_STRUCT_DETECTED;
                        
                    }
                    else if( glslkeywords.count(newString) == 0 ){  //if its not a keyword its a struct member
                        structMemberList[currentStructIndex].push_back(newString);

                    }

                }else if(glslkeywords.count(newString) == 0 && ( structMap.find(newString) == structMap.end() )){//if its not a known type or keyword, but its after
                    state = STRUCT_INSTANCE_DETECTED;                                                            //curly braces, its an instance of the struct after its definition
                    currentStructIndex = structList.size()-1;
                    newExpression = false;
                    goto begin_switch;
                }else{
                    state = DEFAULT;
                    structNameRecorded = false;
                    currentStructIndex = -1;
                    innerCurrentStructIndex = -1;
                    goto begin_switch;
                }
            break;
            
            case STRUCT_INSTANCE_DETECTED:  //if instance of struct found, take the name of variable and add all of its type's members to the uniforms
                if(newExpression){
                    state = DEFAULT;
                    goto begin_switch;
                }
                if( !(glslkeywords.count(newString) > 0 || std::isdigit(newString[0])) ){
                    for(int i = 0; i < structMemberList[currentStructIndex].size(); i++){
                        uniformVector.push_back((newString + "." + structMemberList[currentStructIndex][i]));
                    }
                }
            break;

            case STRUCT_INNER_STRUCT_DETECTED:
                if(newExpression) {
                    state = STRUCT_DETECTED;
                    goto begin_switch;
                }
                for(int i = 0; i <= structMemberList[innerCurrentStructIndex].size()-1; i++){
                    structMemberList[currentStructIndex].push_back( newString + "." + structMemberList[innerCurrentStructIndex][i] );
                }
            break;
            
            case DEFAULT:
                if(newString == "uniform"){
                    braceDepthRecord = braceDepth;
                    state = UNIFORM_DETECTED;

                }else if(newString == "struct"){
                    braceDepthRecord = braceDepth;
                    state = STRUCT_DETECTED;
                }
            break;
        }

        getNextWord(string, index, beginTokenIndex, endTokenIndex, braceDepth, parenthesisDepth, newExpression, equalDetected, true);
        
        if(beginTokenIndex < 0) break;
        newString = sourceCode.substr(beginTokenIndex, endTokenIndex-beginTokenIndex);
        index = endTokenIndex;
    }

    return;

    

}