#include "index.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>

using namespace std;

int level;

Node::Node(bool leaf){
    is_leaf = leaf;
    next_leaf =  nullptr;
    parent = nullptr;
}

Index::Index(int num_rows, vector<int> key, vector<int> value){
    level = (int)(sqrt(num_rows));
    root = nullptr;
    for(int i=0;i<num_rows;i++){
        insert(root, key[i], value[i]);
    }
}

void Index::insert(Node* &current, int key, int value){
    if(current==nullptr){
        current = new Node(true);
        current->keys.push_back(key);
        current->values.push_back(value);
    }else{
        int index = lower_bound(current->keys.begin(), current->keys.end(), key) - current->keys.begin();

        if(current->is_leaf==true){
            current->keys.insert(current->keys.begin()+index, key);
            current->values.insert(current->values.begin()+index, value);

            if(current->keys.size() >= level){
                split_node(current);
            }
        }else{
            insert(current->children[index], key, value);
        }
    }
}   

void Index::split_node(Node* &node){
    if(node->parent==nullptr){
        // [ node是root ]
        if(node->is_leaf==true){
            // [ node是root node也是leaf ]
            Node* new_node = new Node(true);
            int mid = level/2;

            //把keys和values分成兩邊
            new_node->keys.assign(node->keys.begin()+mid, node->keys.end());
            new_node->values.assign(node->values.begin()+mid, node->values.end());
            node->keys.erase(node->keys.begin()+mid, node->keys.end());
            node->values.erase(node->values.begin()+mid, node->values.end());

            //把leaf的linked list接好
            new_node->next_leaf = node->next_leaf; 
            node->next_leaf = new_node; 

            Node* new_root = new Node(false);

            //把median提到root
            new_root->keys.push_back(new_node->keys[0]);

            //把兩個leaf node的parent都設為新的root
            node->parent = new_root;
            new_node->parent = new_root;

            //新的root的兩個children為node和new_node
            new_root->children.push_back(node);
            new_root->children.push_back(new_node);

            //把b+ tree的root更新為新的root
            root = new_root;
        }else{
            // [ node是root node不是leaf ]
            Node* l_node = new Node(false);
            Node* r_node = new Node(false);
            int mid = level/2;

            //把keys和children分成兩邊
            int median = node->keys[mid]; //先把中間值取出來，因為internal node的split不會保留中間值
            l_node->keys.assign(node->keys.begin(), node->keys.begin()+mid);
            l_node->children.assign(node->children.begin(), node->children.begin()+mid+1);
            r_node->keys.assign(node->keys.begin()+mid+1, node->keys.end());
            r_node->children.assign(node->children.begin()+mid+1, node->children.end());

            //原本的node不需要了所以delete
            delete node;

            //更新左右兩個新node的children的parent
            for(Node* &n:l_node->children){
                n->parent = l_node;
            }
            for(Node* &n:r_node->children){
                n->parent = r_node;
            }
            
            Node* new_root = new Node(false);

            //把median放到new_root中
            new_root->keys.push_back(median);

            //把左右兩個新的節點加到新的root的children中
            new_root->children.push_back(l_node);
            new_root->children.push_back(r_node);

            //左右兩個新的node的parent為new_root
            l_node->parent = new_root;
            r_node->parent = new_root;

            //把b+ tree的root更新為新的root
            root = new_root;
        }
    }else if(node->is_leaf==true){
        // [ node是leaf ]
        Node* new_node = new Node(true);
        int mid = level/2;

        //把keys和values分成兩邊
        new_node->keys.assign(node->keys.begin()+mid, node->keys.end());
        new_node->values.assign(node->values.begin()+mid, node->values.end());
        node->keys.erase(node->keys.begin()+mid, node->keys.end());
        node->values.erase(node->values.begin()+mid, node->values.end());

        //把leaf的linked list接好
        new_node->next_leaf = node->next_leaf; 
        node->next_leaf = new_node; 

        //node和new_node的parent要一樣
        new_node->parent = node->parent;

        //把median提到parent
        int median = new_node->keys[0];
        int index = lower_bound(node->parent->keys.begin(), node->parent->keys.end(), median) - node->parent->keys.begin();
        node->parent->keys.insert(node->parent->keys.begin()+index, median);

        //把new_node加到parent的children
        node->parent->children.insert(node->parent->children.begin()+index+1, new_node);
        
        //如果parent爆了，就要split
        if(new_node->parent->keys.size() >= level){
            split_node(new_node->parent);
        }
    }else{
        // [ node是internal ]
        Node* l_node = new Node(false);
        Node* r_node = new Node(false);
        int mid = level/2;

        //把keys和children分成兩邊
        int median = node->keys[mid]; //先把中間值取出來，因為internal node的split不會保留中間值
        l_node->keys.assign(node->keys.begin(), node->keys.begin()+mid);
        l_node->children.assign(node->children.begin(), node->children.begin()+mid+1);
        r_node->keys.assign(node->keys.begin()+mid+1, node->keys.end());
        r_node->children.assign(node->children.begin()+mid+1, node->children.end());

        //在parent的children中找到原本的node並用l_node代替
        for(Node* &n:node->parent->children){
            if(n==node){
                n = l_node;
                break;
            }
        }

        //把左右兩個node的parent設成node的parent
        l_node->parent = node->parent;
        r_node->parent = node->parent;

        //原本的node不需要了所以delete
        delete node;

        //更新左右兩個新node的children的parent
        for(Node* &n:l_node->children){
            n->parent = l_node;
        }
        for(Node* &n:r_node->children){
            n->parent = r_node;
        }

        //把median提到parent
        int index = lower_bound(l_node->parent->keys.begin(), l_node->parent->keys.end(), median) - l_node->parent->keys.begin();
        l_node->parent->keys.insert(l_node->parent->keys.begin()+index, median);

        //把new_node加到parent的children
        l_node->parent->children.insert(l_node->parent->children.begin()+index+1, r_node);

        //如果parent爆了，就要split
        if(l_node->parent->keys.size() >= level){
            split_node(l_node->parent);
        }
    }
}



void Index::key_query(vector<int> query_keys){
    ofstream out;
    out.open("key_query_out.txt");
    for(int i:query_keys){
        out << search(root, i) << endl;
    }
    out.close();
}

int Index::search(Node* &node, int key){
    
    if(node->is_leaf==true){
        int index = lower_bound(node->keys.begin(), node->keys.end(), key)-(node->keys.begin());
        if(node->keys[index]==key){
            return node->values[index];
        }else{
            return -1;
        }
    }else{
        int index = upper_bound(node->keys.begin(), node->keys.end(), key)-(node->keys.begin());
        return search(node->children[index], key);
    }
}

void Index::range_query(vector<pair<int, int> > query_pairs){
    ofstream out;
    out.open("range_query_out.txt");
    for(pair<int,int>p:query_pairs){
        out << range_search(root, p.first, p.second) << endl;
    }
    out.close();
}

int Index::range_search(Node* &node, int low, int high){
    Node* current = range_search_starter(root, low);
    bool find_key = false;
    int mn;
    
    bool over_high = false;
    while(over_high==false){
        for(int i=0;i<current->keys.size();i++){
            if(current->keys[i]>=low && current->keys[i]<=high){
                if(find_key==false){
                    find_key = true;
                    mn = current->values[i];
                }
                mn = min(mn, current->values[i]);
            }else if(current->keys[i]>high){
                over_high = true;
                break;
            }
        }
        current = current->next_leaf;
        if(current==nullptr){
            break;
        }
    }
    if(find_key==false){
        return -1;
    }else{
        return mn;
    }

}

Node* Index::range_search_starter(Node* &node, int low){
    if(node->is_leaf==true){
        return node;
    }else{
        int index = upper_bound(node->keys.begin(), node->keys.end(), low)-(node->keys.begin());
        return range_search_starter(node->children[index], low);
    }
}

void Index::clear_index(){
    clear_node(root);
}

void Index::clear_node(Node* &node){
    if (node != nullptr) {
        for(Node* &n:node->children){
            clear_node(n);
        }
        delete node;
    }
}