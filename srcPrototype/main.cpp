#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

//This may be best as a class vs struct. One big benefit for it being a class is the control/safety rails over the vector.
class Sentence {
private:
    int maxRewrites = 20; //was a constant until I learned you can't copy objects with constants. (swap function)
    string contents, idea;
    vector<string> rewrites;

public:
    void setContents (string &s){
        this->contents = s;
    }
    string getContents (){
        return this->contents;
    }
    void setIdea (string &s){
        this->idea = s;
    }
    string getIdea (){
        return this->idea;
    }
    void addRewrite (string s){
        string punctuation = "?.!";
        //Checks if there is punctuation at the end of s. If not, it appends a period.
        //This fixed an index out of bounds but when loading from file. When the user entered a sentence w/o punctuation
        //the createParagraphFromTxt function would attempt to assign a vector of rewrites to sentence that did not exist.
        if (punctuation.find(s.back()) == string::npos)
            s += '.';

        if (rewrites.size() < maxRewrites)
            this->rewrites.push_back(" " + s); //The space fixes sentences starting right after punctuation.Example.
        else{
            cout << "You have reached the max amount of rewrites (20).\n"
                    "You must delete some if you wish to add more.\n"
                    "Press enter to continue...";
            cin.get();
        }
    }
    string getRewritesIndexedList (){
        int count = 0;
        string rewritesConcat;
        for (const string& s : this->rewrites)
            rewritesConcat += to_string(count++) + ")" + s + "\n";

        return rewritesConcat;
    }
    void insertSelectedRewrite (int selection){
        try {
            this->contents = rewrites.at(selection);
        }catch (const out_of_range& e){
            cout << "There is no rewrite #" << selection << ".\n Invalid entry, press enter to continue.";
            cin.get();
        }
    }
    void swap (Sentence &foreignSentence){
        string tempContents = this->contents;
        string tempIdea = this->idea;
        vector<string> tempRewrites;
        string rewriteElement;

        //swap content strings.
        this->contents = foreignSentence.getContents();
        foreignSentence.setContents(tempContents);

		//swap the idea strings.
        this->idea = foreignSentence.getIdea();
        foreignSentence.setIdea(tempIdea);

		//temp store rewrites
        for (const string &s : this->rewrites)
            tempRewrites.push_back(s);

        //empty this.rewrites and copy foreign into it.
        this->rewrites.clear();
        do { //
            rewriteElement = foreignSentence.takeRewrite();
            if (rewriteElement == "Vector is Empty")
                break;
            this->rewrites.push_back(rewriteElement);
        }while (true);

        //Load the tempRewrites into the foreign rewrites.
        for (const string &s : tempRewrites)
            foreignSentence.addRewrite(s);
    }
    string takeRewrite (){
		//Modified pop_back that returns the last index deleted.
        if (rewrites.empty()) {
            return "Vector is Empty";
        } else {
            string s = rewrites.back();
            rewrites.pop_back();
            return s;
        }
    }
    void clear (){
        rewrites.clear();
        contents = "";
		idea = "";
    }
    void deleteRewrite (int selection){
        try {
            rewrites.erase(rewrites.begin() + selection);
        }catch (const out_of_range &e){
            cout << "Rewrite not found in position " << selection << ".\n Press enter to continue...";
            cin.get();
        }
    }
};

void createParagraph (string &rawPara, vector<Sentence> &para);
string displaySentences (vector<Sentence> &para, int &workingIndex);
string displayFinishedContents(vector<Sentence> &para);
void autoSave (vector<Sentence> &para);
void createParagraphFromTxt (vector<Sentence> &para);

const int LINE_LENGTH = 100;
const int ASCII_OFFSET = 48;
const string FAKE_NEW_WINDOW = string(32, '\n');
const string SAVE_FILE_NAME = "save1.txt";

int main() {
    string rawParagraph;
    vector<Sentence> paragraph;
    string selection;
    Sentence sentenceToInsert;
    int workingIndex = 0;
    int deleteSelection = 0;
    char menuSelection = ' ';

    cout << "Paste paragraph to rewrite or hit enter to load previous work: ";
    getline(cin, rawParagraph);
	//If the user just hits enter it will load from file. Anything else will create a paragraph from the text entered.
    if (rawParagraph.empty())
        createParagraphFromTxt (paragraph);
    else {
        createParagraph(rawParagraph, paragraph);
        autoSave(paragraph);
    }
    //User menu.
    do {
        //push previous menu somewhere far-far away to simulate a refreshing window.
        cout << FAKE_NEW_WINDOW;
        cout << "-------------------------------------------<===|Paragraph|===>------------------------------------------\n"
             << displaySentences(paragraph, workingIndex);
        cout << "\n\n----------<===||Sentence Idea/Goal|===>-----------------------------------------------------------------\n"
        << paragraph[workingIndex].getIdea();
        cout << "\n\n----------<===|Rewrites|===>----------------------------------------------------------------------------\n"
        << paragraph[workingIndex].getRewritesIndexedList();
        cout << "\n----------<===|Menu|===>--------------------------------------------------------------------------------\n"
                "Type a new rewrite followed by enter key.\n"
                "Entering numbers will insert corresponding rewrite to paragraph.\n"
                "\tA) Go to next sentence.  | B) Go to previous sentence.\n"
                "\tC) Delete rewrite.       | D) Move sentence forward.\n"
                "\tE) Move sentence back.   | F) Enter Idea/Goal of Sentence.\n"
                "\tG) Insert New Sentence.  | H) Exit Program.\n";
        getline(cin, selection);
        menuSelection = selection.at(0);

        if (isdigit(selection[0]) && isdigit(selection[1])) { //Will catch double-digit rewriting insertions.
            int tensPlace = (selection[0] - ASCII_OFFSET) * 10, onesPlace = selection[1] - ASCII_OFFSET;
            paragraph[workingIndex].insertSelectedRewrite(tensPlace + onesPlace);
        }else if (isdigit(selection[0])){ //Will catch single-digit rewriting insertions.
            int i = selection[0] - ASCII_OFFSET;
            paragraph[workingIndex].insertSelectedRewrite(i);
        }else if (selection.length() == 1){ //Will catch menu entries.
            menuSelection = tolower(menuSelection);

            switch (menuSelection) {
                case 'a':
                    if (workingIndex < paragraph.size())
                        workingIndex++;
                    break;
                case 'b':
                    if (workingIndex > 0)
                        workingIndex--;
                    break;

                case 'c':
                    cout << FAKE_NEW_WINDOW << "Select rewrite to delete.\n\n"
                    << paragraph[workingIndex].getRewritesIndexedList();
                    cin >> deleteSelection;
                    paragraph[workingIndex].deleteRewrite(deleteSelection);
                    cin.ignore();
                    break;
                case 'd':
                    if (workingIndex < paragraph.size()){
                        paragraph[workingIndex].swap(paragraph[(workingIndex) + 1]);
                        workingIndex++;
                    }
                    break;
                case 'e':
                    if (workingIndex > 0){
                        paragraph[workingIndex].swap(paragraph[(workingIndex) - 1]);
                        workingIndex--;
                    }
                    break;
                case 'f':
                    cout << FAKE_NEW_WINDOW << "Type in the idea of this sentence... \n\""
                         << paragraph[workingIndex].getContents()
                         << "\"\nWhat do you want the reader to think? Don't forget to consider the context.\n\n"
                         << displaySentences(paragraph, workingIndex);
                    cout << "\n\nEnter Idea: ";
                    getline(cin, selection);
                    paragraph[workingIndex].setIdea(selection);
                    break;
                case 'g':
                    sentenceToInsert.clear();
                    cout << "Enter New Sentence: ";
                    getline(cin, selection);
                    sentenceToInsert.setContents(selection);
                    sentenceToInsert.addRewrite(selection);
                    paragraph.insert(paragraph.begin() + workingIndex, sentenceToInsert);
                    break;
                case 'h':
                    cout << FAKE_NEW_WINDOW
                    << "---------------<===|Finished copy and paste friendly version below|===>---------------\n\n"
                    << displayFinishedContents(paragraph) << "\n\n Press enter to quit application...";
                    autoSave(paragraph);

                    cin.get();

                    return 0;
                default:
                    break;
            }
        }else { //Tosses anything else in a rewrite vector.
            paragraph[workingIndex].addRewrite(selection);
        }
        autoSave(paragraph);

    } while (1); //No reason to exit loop.
}

void createParagraph (string &rawPara, vector<Sentence> &para){
    string token, delimiters = "?.!";
    int i = 0;
    size_t pos = 0; //size_t is used in the find function.
    size_t start = 0;

    while ((pos = rawPara.find_first_of(delimiters, start)) != string::npos){
        token = rawPara.substr(start,pos - start + 1);
        Sentence sentence;
        sentence.setContents(token);
        sentence.addRewrite(sentence.getContents());
        para.push_back(sentence);
        start = pos +1;
    }
    //Clears the Paragraph string in case user wants to rewrite another paragraph after they finish current.
    rawPara = "";
}
string displaySentences(vector<Sentence> &para, int &workingIndex) {
    string sentence;
    int lastSpace = -1;
    char specialChar = 219;

    //Create string out of all para Sentences contents
    for (int i = 0; i < para.size(); i++) {
        if (!para.at(i).getContents().empty() && i == workingIndex)
            sentence += specialChar + para.at(i).getContents() + specialChar; //Shows what sentence is being operated on.
        else if (!para[i].getContents().empty())
            sentence += para.at(i).getContents();
    }

    for (int i = 0; i < sentence.length(); i++) {
        //Keeps track of where the last space was.
        if (sentence[i] == ' ')
            lastSpace = i;

        //Checks if text has reached line length constant. If it has, it will insert a newline at the last space.
        if (i % LINE_LENGTH == 0 && lastSpace != -1) {
            sentence[lastSpace] = '\n';
            lastSpace = -1;
        }
    }
    return string(6,' ') + sentence;
}
string displayFinishedContents(vector<Sentence> &para) {
    string sentence;

    //Create string out of all para Sentences contents
    for (int i = 0; i < para.size(); i++) {
        if (!para.at(i).getContents().empty())
            sentence += para.at(i).getContents();
    }
    return sentence;
}
void autoSave (vector<Sentence> &para){
    ofstream file(SAVE_FILE_NAME);
    string rewrites;

    cout << "\nSaving...\n";

    //Stores the paragraph. Next line (endl) will be the delimiter.
    if (file.is_open())
        file << displayFinishedContents(para) << endl;
    else{
        cout << "\nError creating save file.\n";
        return;
    }

    //Stores the rewrites.
    for (int i = 0; i < para.size(); ++i){
        if (!para.at(i).getContents().empty()){
            file << para.at(i).getRewritesIndexedList();
            file << "Advance Index\n";
        }
    }

    file.close();
}
void createParagraphFromTxt (vector<Sentence> &para){
    ifstream file(SAVE_FILE_NAME);
    string paragraph;
    int i = 0;

    //Checks the file is open loads next line onto the string and resolves as false when it reaches the end of the file.
    while (file.is_open() && getline(file, paragraph)){
        if (para.empty())
            createParagraph(paragraph, para);
        else if (paragraph == "Advance Index"){
            para.at(i).deleteRewrite(0);
            i++;
        } else {
            paragraph.erase(0,3);
            para.at(i).addRewrite(paragraph);
        }
        paragraph.clear();
    }

    file.close();
}