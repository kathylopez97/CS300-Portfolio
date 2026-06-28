//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Kathy Lopez
// Version     : 2.0
// Description : CS 300 Project Two - Advising Assistance Program
//============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

// Course stores the course number, course title, and prerequisite course numbers.
struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};

// Node stores one course and the left/right links used by the binary search tree.
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(const Course& aCourse) {
        course = aCourse;
        left = nullptr;
        right = nullptr;
    }
};

// BinarySearchTree stores courses by course number so an in-order traversal prints
// the course list in alphanumeric order.
class BinarySearchTree {
private:
    Node* root;

    void addNode(Node* node, const Course& course) {
        if (course.courseNumber < node->course.courseNumber) {
            if (node->left == nullptr) {
                node->left = new Node(course);
            }
            else {
                addNode(node->left, course);
            }
        }
        else if (course.courseNumber > node->course.courseNumber) {
            if (node->right == nullptr) {
                node->right = new Node(course);
            }
            else {
                addNode(node->right, course);
            }
        }
    }

    void inOrder(Node* node) const {
        if (node != nullptr) {
            inOrder(node->left);
            cout << node->course.courseNumber << ", " << node->course.title << endl;
            inOrder(node->right);
        }
    }

    void deleteTree(Node* node) {
        if (node != nullptr) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

public:
    BinarySearchTree() {
        root = nullptr;
    }

    ~BinarySearchTree() {
        deleteTree(root);
    }

    void Clear() {
        deleteTree(root);
        root = nullptr;
    }

    bool IsEmpty() const {
        return root == nullptr;
    }

    void Insert(const Course& course) {
        if (root == nullptr) {
            root = new Node(course);
        }
        else {
            addNode(root, course);
        }
    }

    Course Search(const string& courseNumber) const {
        Node* current = root;

        while (current != nullptr) {
            if (current->course.courseNumber == courseNumber) {
                return current->course;
            }
            else if (courseNumber < current->course.courseNumber) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }

        Course emptyCourse;
        return emptyCourse;
    }

    void PrintCourseList() const {
        inOrder(root);
    }
};

// Removes leading and trailing spaces from a string.
string trim(const string& value) {
    size_t start = value.find_first_not_of(" \t\r\n");
    size_t end = value.find_last_not_of(" \t\r\n");

    if (start == string::npos || end == string::npos) {
        return "";
    }

    return value.substr(start, end - start + 1);
}

// Converts input to uppercase so course searches are not case-sensitive.
string toUpperCase(string value) {
    for (char& character : value) {
        character = static_cast<char>(toupper(static_cast<unsigned char>(character)));
    }

    return value;
}

// Splits one comma-separated line into fields.
vector<string> splitCSVLine(const string& line) {
    vector<string> tokens;
    string token;
    stringstream lineStream(line);

    while (getline(lineStream, token, ',')) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

// Reads the CSV file into a temporary vector. This first pass helps validate the
// basic file format before any course is inserted into the tree.
bool readCourseFile(const string& fileName, vector<Course>& courses) {
    ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        cout << "Error: Could not open file " << fileName << "." << endl;
        return false;
    }

    courses.clear();
    string line;
    int lineNumber = 0;
    bool fileIsValid = true;

    while (getline(inputFile, line)) {
        lineNumber++;

        if (trim(line).empty()) {
            continue;
        }

        vector<string> tokens = splitCSVLine(line);

        // Each valid row must include a course number and a course title.
        if (tokens.size() < 2 || tokens.at(0).empty() || tokens.at(1).empty()) {
            cout << "Error: Invalid format on line " << lineNumber << "." << endl;
            fileIsValid = false;
            continue;
        }

        Course course;
        course.courseNumber = toUpperCase(tokens.at(0));
        course.title = tokens.at(1);

        for (size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens.at(i).empty()) {
                course.prerequisites.push_back(toUpperCase(tokens.at(i)));
            }
        }

        courses.push_back(course);
    }

    inputFile.close();
    return fileIsValid && !courses.empty();
}

// Validates that every prerequisite listed in the file also appears as its own course.
bool validatePrerequisites(const vector<Course>& courses) {
    unordered_set<string> validCourseNumbers;
    bool prerequisitesAreValid = true;

    for (const Course& course : courses) {
        validCourseNumbers.insert(course.courseNumber);
    }

    for (const Course& course : courses) {
        for (const string& prerequisite : course.prerequisites) {
            if (validCourseNumbers.find(prerequisite) == validCourseNumbers.end()) {
                cout << "Error: " << course.courseNumber << " lists missing prerequisite "
                     << prerequisite << "." << endl;
                prerequisitesAreValid = false;
            }
        }
    }

    return prerequisitesAreValid;
}

// Loads validated courses into the binary search tree.
bool loadCourses(const string& fileName, BinarySearchTree& courseTree) {
    vector<Course> courses;

    if (!readCourseFile(fileName, courses)) {
        cout << "Course data was not loaded." << endl;
        return false;
    }

    if (!validatePrerequisites(courses)) {
        cout << "Course data was not loaded because the file contains invalid prerequisites." << endl;
        return false;
    }

    courseTree.Clear();

    for (const Course& course : courses) {
        courseTree.Insert(course);
    }

    cout << courses.size() << " courses loaded successfully." << endl;
    return true;
}

// Prints one course and its prerequisites.
void printCourseInformation(const BinarySearchTree& courseTree) {
    string courseNumber;

    cout << "What course do you want to know about? ";
    cin >> courseNumber;

    courseNumber = toUpperCase(trim(courseNumber));
    Course course = courseTree.Search(courseNumber);

    if (course.courseNumber.empty()) {
        cout << "Course " << courseNumber << " was not found." << endl;
        return;
    }

    cout << course.courseNumber << ", " << course.title << endl;

    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
    }
    else {
        cout << "Prerequisites: ";

        for (size_t i = 0; i < course.prerequisites.size(); ++i) {
            cout << course.prerequisites.at(i);

            if (i < course.prerequisites.size() - 1) {
                cout << ", ";
            }
        }

        cout << endl;
    }
}

// Displays the required menu options.
void displayMenu() {
    cout << endl;
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << "What would you like to do? ";
}

int main() {
    BinarySearchTree courseTree;
    string fileName;
    int userChoice = 0;
    bool dataLoaded = false;

    cout << "Welcome to the course planner." << endl;

    while (userChoice != 9) {
        displayMenu();

        if (!(cin >> userChoice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }

        switch (userChoice) {
        case 1:
            cout << "Enter the file name: ";
            cin.ignore(10000, '\n');
            getline(cin, fileName);
            fileName = trim(fileName);

            if (fileName.empty()) {
                fileName = "ABCU_Advising_Program_Input.csv";
            }

            dataLoaded = loadCourses(fileName, courseTree);
            break;

        case 2:
            if (!dataLoaded || courseTree.IsEmpty()) {
                cout << "Please load the course data first." << endl;
            }
            else {
                cout << "Here is a sample schedule:" << endl;
                courseTree.PrintCourseList();
            }
            break;

        case 3:
            if (!dataLoaded || courseTree.IsEmpty()) {
                cout << "Please load the course data first." << endl;
            }
            else {
                printCourseInformation(courseTree);
            }
            break;

        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;

        default:
            cout << userChoice << " is not a valid option." << endl;
            break;
        }
    }

    return 0;
}
