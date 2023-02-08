#include <memory>
#include <vector>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    class Node {
    public:
        size_t moves = 0;
        bool deleted = false;
        bool end = false;
        bool used = false;
        std::pair<const KeyType, ValueType> val;

        explicit Node(bool e = false) {
            end = e;
        }

        Node(size_t m, std::pair<const KeyType, ValueType> val): val(val) {
            moves = m;
            used = true;
        }
    };

    class iterator {
    public:
        explicit iterator(Node** pointer) {
            ptr_ = pointer;
        }

        iterator() {
            ptr_ = nullptr;
        }

        std::pair<const KeyType, ValueType>& operator*() const {
            return (*ptr_)->val;
        }

        std::pair<const KeyType, ValueType>* operator->() const {
            return &((*ptr_)->val);
        }

        iterator& operator=(iterator other) {
            ptr_ = other.ptr_;
            return *this;
        }

        iterator& operator++() {
            ++ptr_;
            while (ptr_ != nullptr && !(*ptr_)->end && !(*ptr_)->used) {
                ++ptr_;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator copy(ptr_);
            ++ptr_;
            while (ptr_ != nullptr && !(*ptr_)->end && !(*ptr_)->used) {
                ++ptr_;
            }
            return copy;
        }

        bool operator==(const iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const iterator& other) const {

            return ptr_ != other.ptr_;
        }

    protected:
        Node** ptr_ = nullptr;
    };

    class const_iterator {
    public:
        explicit const_iterator(Node* const* pointer) {
            ptr_ = pointer;
        }

        const_iterator() {
            ptr_ = nullptr;
        }

        const std::pair<const KeyType, ValueType>& operator*() const {
            return (*ptr_)->val;
        }

        const std::pair<const KeyType, ValueType>* operator->() const {
            return &((*ptr_)->val);
        }

        const_iterator& operator=(const_iterator other) {
            ptr_ = other.ptr_;
            return *this;
        }

        const_iterator& operator++() {
            ++ptr_;
            while (ptr_ != nullptr && !(*ptr_)->end && !(*ptr_)->used) {
                ++ptr_;
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator copy(ptr_);
            ++ptr_;
            while (ptr_ != nullptr && !(*ptr_)->end && !(*ptr_)->used) {
                ++ptr_;
            }
            return copy;
        }

        bool operator==(const const_iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const const_iterator& other) const {
            return ptr_ != other.ptr_;
        }

    protected:
        Node* const* ptr_ = nullptr;
    };

    HashMap(): hash_() {}

    explicit HashMap(Hash hash): hash_(hash) {}

    HashMap(const HashMap<KeyType, ValueType, Hash>& map) {
        for (auto el : map) {
            insert(el);
        }
    }

    HashMap(typename HashMap<KeyType, ValueType>::iterator begin, typename HashMap<KeyType, ValueType>::iterator end): hash_() {
        while (begin != end) {
            insert(*(begin++));
        }
    }

    HashMap(typename HashMap<KeyType, ValueType>::iterator begin, typename HashMap<KeyType, ValueType>::iterator end, Hash hash): hash_(hash) {
        while (begin != end) {
            insert(*(begin++));
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list): hash_() {
        for (auto p : list) {
            insert(p);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash hash): hash_(hash) {
        for (auto& p : list) {
            insert(p);
        }
    }

    HashMap& operator=(const HashMap<KeyType, ValueType, Hash>& map) {
        std::vector<std::pair<KeyType, ValueType>> temp;
        for (auto el : map) {
            temp.push_back(el);
        }
        clear();
        for (auto el : temp) {
            insert(el);
        }
        return *this;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return hash_;
    }

    void insert(std::pair<const KeyType, ValueType> p) {
        check();
        if (find(p.first) != end()) {
            return;
        }
        auto n = new Node(0, p);
        size_t i = hash_(p.first) % capacity_;
        while (table_[i]->used) {
            if (n->moves > table_[i]->moves) {
                std::swap(n, table_[i]);
            }
            ++n->moves;
            ++i;
            if (i == capacity_) {
                i = 0;
            }
        }
        deleted_ -= table_[i]->deleted;
        std::swap(table_[i], n);
        ++size_;
        delete n;
    }

    void erase(const KeyType& key) {
        check();
        size_t i = hash_(key) % capacity_;
        while ((table_[i]->used || table_[i]->deleted) && table_[i]->val.first != key) {
            ++i;
            if (i == capacity_) {
                i = 0;
            }
             if (i == hash_(key) % capacity_) {
                 return;
             }
        }
        if (table_[i]->used && table_[i]->val.first == key) {
            table_[i]->used = false;
            table_[i]->deleted = true;
            --size_;
            ++deleted_;
        }
    }

    iterator find(const KeyType& key) {
        size_t i = hash_(key) % capacity_;
        while ((table_[i]->used || table_[i]->deleted) && !(table_[i]->val.first == *const_cast<const KeyType*>(&key))) {
            ++i;
            if (i == capacity_) {
                i = 0;
            }
            if (i == hash_(key) % capacity_) {
                return end();
            }
        }
        if (table_[i]->used) {
            return iterator(&table_[i]);
        }
        return end();
    }

    const_iterator find(const KeyType& key) const {
        size_t i = hash_(key) % capacity_;
        while ((table_[i]->used || table_[i]->deleted) && !(table_[i]->val.first == *const_cast<const KeyType*>(&key))) {
            ++i;
            if (i == capacity_) {
                i = 0;
            }
            if (i == hash_(key) % capacity_) {  // костыль на какой-то очень тупой случай, когда все ячейки заняты,
                return end();                  // но нужного значения нет (не уверен, что такое вообще бывает)
            }
        }
        if (table_[i]->used) {
            return const_iterator(&table_[i]);
        }
        return end();
    }

    ValueType& operator[](const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            insert({key, ValueType()});
            it = find(key);
        }
        return it->second;
    }

    const ValueType& at(const KeyType& key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("");
        }
        return it->second;
    }

    void clear() {
        for (size_t i = 0; i <= capacity_; ++i) {
            delete table_[i];
            table_[i] = new Node();
        }
        table_.back()->end = true;
        size_ = 0;
        deleted_ = 0;
    }

    iterator begin() {
        size_t i = 0;
        while (i < capacity_ && !table_[i]->used) {
            ++i;
        }
        return iterator(&table_[i]);
    }

    iterator end() {
        return iterator(&table_.back());
    }

    const_iterator begin() const {
        size_t i = 0;
        while (i < capacity_ && !table_[i]->used) {
            ++i;
        }
        return const_iterator(&table_[i]);
    }

    const_iterator end() const {
        return const_iterator(&table_.back());
    }

    ~HashMap() {
        for (size_t i = 0; i <= capacity_; ++i) {
            delete table_[i];
        }
    }

protected:
    size_t capacity_ = 1;
    size_t size_ = 0;
    size_t deleted_= 0;
    const double max_load_ = 0.69;  // nice
    std::vector<Node*> table_{new Node(), new Node(true)};
    Hash hash_;

    void check() {
        if (deleted_ > size_) {
            remove_deleted();
        }
        if (size_ + deleted_ > max_load_ * capacity_) {
            expand();
        }
    }

    void expand() {
        capacity_ *= 2;
        table_.resize(capacity_ + 1);
        std::vector<std::pair<const KeyType, ValueType>> temp;
        for (size_t i = 0; i <= capacity_; ++i) {
            if (table_[i] != nullptr && table_[i]->used) {
                temp.push_back(table_[i]->val);
            }
            delete table_[i];
            table_[i] = new Node();
        }
        table_.back()->end = true;
        size_ = 0;
        deleted_ = 0;
        for (auto elem : temp) {
            insert(elem);
        }
    }

    void remove_deleted() {
        std::vector<std::pair<const KeyType, ValueType>> temp;
        for (size_t i = 0; i <= capacity_; ++i) {
            if (table_[i] != nullptr && table_[i]->used) {
                temp.push_back(table_[i]->val);
            }
            delete table_[i];
            table_[i] = new Node();
        }
        table_.back()->end = true;
        size_ = 0;
        deleted_ = 0;
        for (auto elem : temp) {
            insert(elem);
        }
    }
};
