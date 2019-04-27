#ifndef PATH_H
#define PATH_H

#include <vector>
#define MAX_FOLDER_NAME_SIZE 32


class Node {
private:
    int depth = 0;
    Node* parent;
    std::vector<Node*> children;
    char folder_name[MAX_FOLDER_NAME_SIZE];
public:
    Node();
    Node(Node* parent);
    ~Node();

    void addChild(Node* child);
    bool addChildAt(Node* child, char* parent);
    Node* getParent() const;
    void setParent(Node* _parent);
    bool checkNameOnce(char * name);
    Node* checkName(char * name);
    std::vector<Node*> getChildren() const;
    char* getFolderName();
    int setFolderName(char* new_name);

    void printPath();
};

#endif
