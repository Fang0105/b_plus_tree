#ifndef INDEX_H_
#define INDEX_H_

#include<vector>
#include<iostream>
using namespace std;

struct Node{
    vector<int>keys;
    vector<int>values;
    bool is_leaf;
    vector<Node*>children;
    Node* parent;
    Node* next_leaf;

    Node(bool leaf);

};



class Index {
    private:
        Node* root;
        void insert(Node* &current, int key, int value);
        void split_node(Node* &node);
        int search(Node* &node, int key);
        int range_search(Node* &node, int low, int high);
        Node* range_search_starter(Node* &node, int low);
        void clear_node(Node* &node);

    public:
        Index(int num_rows, vector<int> key, vector<int> value);
        void key_query(vector<int> query_keys);
        void range_query(vector<pair<int, int>> query_pairs);
        void clear_index();

};



#endif