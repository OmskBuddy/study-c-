#include "tree/Tree.hpp"

#include <cmath>

AVL::Node::Node(int value) : value(value) {}

AVL::~AVL() {
    destruct(root);
}

// public methods

std::size_t AVL::size() const noexcept {
    return getSize(root);
}

bool AVL::empty() const noexcept {
    return size() == 0;
}

bool AVL::insert(int value) {
    size_t old   = size();
    root         = insertImpl(root, value);
    size_t young = size();
    return young > old;
}

bool AVL::remove(int value) {
    size_t old   = size();
    root         = removeImpl(root, value);
    size_t young = size();
    return young < old;
}

bool AVL::contains(int value) const noexcept {
    return containsImpl(root, value);
}

std::vector<int> AVL::values() const noexcept {
    std::vector<int> result;
    result.reserve(size());
    valuesImpl(root, result);
    return result;
}

// private methods

void AVL::destruct(AVL::Node* node) {
    if (!node) {
        return;
    }
    destruct(node->left);
    destruct(node->right);
    delete node;
}

int AVL::getHeight(AVL::Node* node) {
    return node ? node->height : 0;
}

size_t AVL::getSize(AVL::Node* n) {
    return n ? n->size : 0;
}

void AVL::updateHeight(AVL::Node* node) {
    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
}

void AVL::updateSize(AVL::Node* node) {
    node->size = getSize(node->left) + getSize(node->right) + 1;
}

void AVL::update(AVL::Node* node) {
    updateHeight(node);
    updateSize(node);
}

int AVL::getBalance(AVL::Node* node) {
    return getHeight(node->right) - getHeight(node->left);
}

AVL::Node* AVL::rightRotate(AVL::Node* node) {
    Node* left  = node->left;
    node->left  = left->right;
    left->right = node;

    update(node);
    update(left);

    return left;
}

AVL::Node* AVL::leftRotate(AVL::Node* node) {
    Node* right = node->right;
    node->right = right->left;
    right->left = node;

    update(node);
    update(right);

    return right;
}

AVL::Node* AVL::bigLeftRotate(AVL::Node* node) {
    node->right = rightRotate(node->right);
    return leftRotate(node);
}

AVL::Node* AVL::bigRightRotate(AVL::Node* node) {
    node->left = leftRotate(node->left);
    return rightRotate(node);
}

AVL::Node* AVL::makeBalance(AVL::Node* node) {
    update(node);
    int balance = getBalance(node);
    if (balance == 2) {
        if (getBalance(node->right) < 0) {
            return bigLeftRotate(node);
        } else {
            return leftRotate(node);
        }
    } else if (balance == -2) {
        if (getBalance(node->left) > 0) {
            return bigRightRotate(node);
        } else {
            return rightRotate(node);
        }
    } else {
        return node;
    }
}

AVL::Node* AVL::containsImpl(AVL::Node* n, int value) {
    if (!n)
        return nullptr;

    update(n);

    if (value < n->value) {
        return containsImpl(n->left, value);
    } else if (value > n->value) {
        return containsImpl(n->right, value);
    } else {
        return n;
    }
}

AVL::Node* AVL::insertImpl(AVL::Node* n, int value) {
    if (!n) {
        return new Node(value);
    }

    update(n);

    if (value < n->value) {
        n->left = insertImpl(n->left, value);
    } else if (value > n->value) {
        n->right = insertImpl(n->right, value);
    }
    return makeBalance(n);
}

AVL::Node* AVL::getMin(AVL::Node* node) {
    return node->left ? getMin(node->left) : node;
}

AVL::Node* AVL::removeImpl(AVL::Node* n, int value) {
    if (!n) {
        return nullptr;
    }

    update(n);

    if (value < n->value) {
        n->left = removeImpl(n->left, value);
    } else if (value > n->value) {
        n->right = removeImpl(n->right, value);
    } else {
        if (!n->right) {
            Node* l = n->left;
            delete n;
            return l;
        }

        Node* min = getMin(n->right);
        n->value  = min->value;
        n->right  = removeImpl(n->right, min->value);
    }
    return makeBalance(n);
}

void AVL::valuesImpl(AVL::Node* n, std::vector<int>& result) {
    if (n->left) {
        valuesImpl(n->left, result);
    }
    result.push_back(n->value);
    if (n->right) {
        valuesImpl(n->right, result);
    }
}
