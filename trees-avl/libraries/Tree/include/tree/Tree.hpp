#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

class AVL {
public:
    [[nodiscard]] bool contains(int value) const noexcept;
    bool insert(int value);
    bool remove(int value);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] std::vector<int> values() const noexcept;

    ~AVL();

private:
    struct Node {
        int value;
        int height{1};
        size_t size{1};
        Node* left{nullptr};
        Node* right{nullptr};
        Node(int);
    };

    Node* root{nullptr};

    static void destruct(Node* n);

    static int getHeight(Node* n);
    static size_t getSize(Node* n);
    static int getBalance(Node* n);

    static void updateHeight(Node* n);
    static void updateSize(Node* n);
    static void update(Node* n);

    static Node* leftRotate(Node* n);
    static Node* rightRotate(Node* n);

    static Node* bigLeftRotate(Node* n);
    static Node* bigRightRotate(Node* n);

    static Node* makeBalance(Node* n);

    static Node* containsImpl(Node* n, int value);

    static Node* insertImpl(Node* n, int value);

    static Node* getMin(Node* n);
    static Node* removeImpl(Node* n, int value);

    static void valuesImpl(Node* n, std::vector<int>& result);
};

#endif
