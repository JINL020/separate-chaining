#ifndef ADS_SET_H
#define ADS_SET_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

enum Mode { normal,
            speziell };

// type name Key is the typ of keys the set takes
// size_t N is the starting size of the ADS_set
// both these parameters are specified at compile time
template <typename Key, size_t N = 9>
class ADS_set {
   public:
    class Iterator;
    //using alias
    using value_type = Key;
    using key_type = Key;
    using reference = key_type &;
    using const_reference = const key_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Iterator;
    using const_iterator = Iterator;
    using key_compare = std::less<key_type>;
    using key_equal = std::equal_to<key_type>;  // Hashing
    using hasher = std::hash<key_type>;         // Hashing
   private:
    struct Entry;
    struct List;
    size_type buckets;  // number of max buckets
    size_type current_size;
    List *values;  //pointer to first List array

   public:
    // default constructor
    ADS_set() : buckets{N}, current_size{0}, values{new List[N]} {}

    //initializer list constructor
    ADS_set(std::initializer_list<key_type> ilist) : ADS_set{std::begin(ilist), std::end(ilist)} {}

    //copy constructor
    ADS_set(const ADS_set &other) : ADS_set(other.begin(), other.end()) {
    }

    //range constructor
    template <typename InputIt>
    ADS_set(InputIt first, InputIt last) : ADS_set{} {
        this->insert(first, last);
    }

    // destructor
    ~ADS_set() {
        delete[] values;
    }

    //size
    size_type size() const {
        return current_size;
    }

    //empty
    bool empty() const {
        return !current_size;
    }

    // assignment operator
    ADS_set &operator=(const ADS_set &other) {
        if (this == &other)
            return *this;
        ADS_set temp{other};
        swap(temp);
        return *this;
    }

    // assignment operator
    ADS_set &operator=(std::initializer_list<key_type> ilist) {
        ADS_set tmp{ilist};
        swap(tmp);
        return *this;
    }

    //swap
    void swap(ADS_set &other) {
        using std::swap;
        swap(values, other.values);
        swap(current_size, other.current_size);
        swap(buckets, other.buckets);
    }

    // initializer list inserts
    void insert(std::initializer_list<key_type> ilist) {
        this->insert(std::begin(ilist), std::end(ilist));
    }

    // single key inserts
    std::pair<iterator, bool> insert(const key_type &key) {
        if (!count(key)) {
            ++current_size;
            expand();
            values[hash(key)].add_Entry(key);
            return std::pair<iterator, bool>(find(key), true);
        }
        return std::pair<iterator, bool>(find(key), false);
    }

    //range insert
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto elem_it{first}; elem_it != last; ++elem_it) {
            if (!count(*elem_it)) {
                ++current_size;
                expand();
                values[hash(*elem_it)].add_Entry(*elem_it);
            }
        }
    }

    //expand
    void expand() {
        if (current_size > buckets) {
            buckets *= 3;
            List *expanded{new List[buckets]};
            //divide becuase we need the old bucket number
            for (size_type i{0}; i < buckets / 3; ++i) {
                for (Entry *temp{values[i].head}; temp != nullptr; temp = temp->next) {
                    expanded[hash(temp->data)].add_Entry(temp->data);
                }
            }
            delete[] values;
            values = expanded;
        }
    }

    // returns hashcode
    size_type hash(const key_type &key) const {
        return hasher{}(key) % buckets;
    }

    // clear
    void clear() {
        ADS_set temp;
        swap(temp);
    }

    //erase
    size_type erase(const key_type &key) {
        size_type hashKey{hash(key)};
        Entry *current{values[hashKey].head};
        Entry *previous{nullptr};
        if (current == nullptr)
            return false;
        if (key_equal{}(current->data, key)) {
            values[hashKey].head = current->next;
            delete current;
            --current_size;
            return true;
        }
        while (!key_equal{}(current->data, key)) {
            previous = current;
            current = current->next;
            if (current == nullptr)
                return false;
        }
        previous->next = current->next;
        delete current;
        --current_size;
        return true;
    }

    //count
    size_type count(const key_type &key) const {
        for (Entry *temp{values[hash(key)].head}; temp != nullptr; temp = temp->next) {
            if (key_equal{}(temp->data, key)) {
                return 1;
            }
        }
        return 0;
    }

    //find
    iterator find(const key_type &key) const {
        size_type hashKey{hash(key)};
        for (Entry *temp{values[hashKey].head}; temp != nullptr; temp = temp->next) {
            if (key_equal{}(temp->data, key)) {
                return Iterator(values + hashKey, values + buckets, temp);
            }
        }
        return end();
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
        if (lhs.current_size != rhs.current_size)
            return false;
        for (const auto &key : rhs) {
            if (!lhs.count(key))
                return false;
        }
        return true;
    }

    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
        return !(lhs == rhs);
    }

    //dump
    void dump(std::ostream &o = std::cerr) const {
        o << "\n\nSEPARATE CHAINING\n\n";
        for (size_type i{0}; i < buckets; ++i) {
            o << "Bucket " << i << "\n";
            for (Entry *temp{values[i].head}; temp != nullptr; temp = temp->next) {
                o << " --> " << temp->data;
            }
            o << "\n\n\n\n";
        }
    }

    // begin
    const_iterator begin() const {
        for (size_type i{0}; i < buckets; ++i) {
            if (values[i].head != nullptr) {
                return const_iterator(values + i, values + buckets, values[i].head);
            }
        }
        return const_iterator(values + buckets, values + buckets, nullptr);
    }

    // end
    const_iterator end() const {
        return const_iterator(values + buckets, values + buckets, nullptr);
    }

    const_iterator z() const {
        if (current_size == 0)
            return end();
        Entry *last;
        List *lastList;
        for (size_type i{buckets - 1}; i >= 0; --i) {
            if (values[i].head != nullptr) {
                lastList = values + i;
                Entry *temp{values[i].head};
                while (temp != nullptr) {
                    last = temp;
                    temp = temp->next;
                }
                break;
            }
        }
        for (size_type i{0}; i < buckets; ++i) {
            if (values[i].head != nullptr) {
                return const_iterator(values + i, values + buckets, values[i].head, Mode::speziell, lastList, last);
            }
        }
        return end();
    }
};  // end of ADS_set class

// Entry struct
template <typename Key, size_t N>
struct ADS_set<Key, N>::Entry {
    key_type data;
    Entry *next;
    Entry(key_type data, Entry *next) : data{data}, next{next} {}
};  // end of Entry struct

// List struct
template <typename Key, size_t N>
struct ADS_set<Key, N>::List {
    Entry *head;
    Entry *tail;

    List() : head{nullptr}, tail{nullptr} {}
    ~List() {
        Entry *toDelete{head};
        while (toDelete != nullptr) {
            Entry *next{toDelete->next};
            delete toDelete;
            toDelete = next;
        }
    }

    // adds an Entry to the front of the List
    void add_Entry(const key_type &value) {
        Entry *temp{new Entry(value, nullptr)};
        temp->next = head;
        head = temp;
        if (head == nullptr) {
            tail = temp;
        }
    }
};  // end of List struct

// Iterator class
template <typename Key, size_t N>
class ADS_set<Key, N>::Iterator {
   public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;

   private:
    List *current;
    List *end;
    Entry *entry;
    Mode mode;
    List *lastList;
    Entry *last;
    size_type counter = 0;
    void skip() {
        if (current == end) {
            return;
        } else if (entry->next != nullptr) {
            entry = entry->next;
        } else {
            ++current;
            while (current != end && current->head == nullptr) {
                ++current;
            }
            entry = (current == end ? nullptr : current->head);
        }
    }

   public:
    explicit Iterator(List *current, List *end, Entry *entry, Mode mode = Mode::normal, List *lastList = nullptr, Entry *last = nullptr) : current{current}, end{end}, entry{entry}, mode{mode}, lastList{lastList}, last{last} {
    }

    Iterator() : current{nullptr}, end{nullptr}, entry{nullptr}, mode{Mode::normal}, lastList{nullptr}, last{nullptr} {
    }

    reference operator*() const {
        return entry->data;
    }

    pointer operator->() const {
        return &(entry->data);
    }

    //präfix
    Iterator &operator++() {
        if (mode == Mode::speziell) {
            if (counter == 0) {
                current = lastList;
                entry = last;
                ++counter;
            } else {
                current = end;
                entry = nullptr;
            }
            return *this;
        }
        skip();
        return *this;
    }

    //postfix
    Iterator operator++(int) {
        auto temp(*this);
        skip();
        return temp;
    }

    friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
        return lhs.operator->() == rhs.operator->();
    }

    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
        return !(lhs == rhs);
    }
};  // end of Iterator class

template <typename Key, size_t N>
void swap(ADS_set<Key, N> &lhs, ADS_set<Key, N> &rhs) { lhs.swap(rhs); }

#endif  // ADS_SET_H