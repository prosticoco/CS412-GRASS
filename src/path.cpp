#include "path.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>



Node::Node() {
    parent = NULL;
    children = std::vector<Node*>();
}

Node::Node(Node* _parent) {
    parent = _parent;  
}

Node::~Node() {
}

void Node::addChild(Node* child) {
    if(children.size() == 0) {
        depth++;
    }
    children.push_back(child);
}

char* Node::getFolderName() {
    return folder_name;
}

void Node::setParent(Node* _parent) {
    parent = _parent;
}

Node* Node::getParent() const {
    return parent;
}

int Node::setFolderName(char* new_name) {
    if(strlen(new_name) < MAX_FOLDER_NAME_SIZE) {
        bzero(folder_name, MAX_FOLDER_NAME_SIZE);
        strncpy(folder_name, new_name, MAX_FOLDER_NAME_SIZE);
        return 0;
    } else {
        return ERROR_FOLDER_NAME_SIZE;
    }
}

bool Node::addChildAt(Node* child, char* parent) {
    if (strncmp(parent, folder_name, MAX_FOLDER_NAME_SIZE) == 0) {
        addChild(child);
        return true;
    } else {
        for(auto curr : children) {
            if(curr->addChildAt(child, parent)) {
                break;
            }
        }
    }
    return false;
}
Node* Node::checkName(char * name) {

    for(auto child : children) {
        if(child->checkNameOnce(name)) {
            return child;
        }
    }
    return NULL;
}

bool Node::checkNameOnce(char * name) {    
    if(strncmp(folder_name, name, strlen(name))==0 ) {
        return true;
    } else {
        return false;
    } 
}

void Node::printPath() {

    if(children.size() <=0) {
        std::cout << "/" << folder_name;
        return;
    } else {

        for(auto c : children) {
            
            std::cout << "/" << folder_name;
            c->printPath();
            std::cout << std::endl;
        }
    }

}