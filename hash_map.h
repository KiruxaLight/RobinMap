#include <vector>
#include <list>
#include <iostream>
#include <utility>
#include <functional>
#include <list>
#include <memory>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <map>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    struct Node {
        std::pair<const KeyType, ValueType> element;
        size_t dist;
        size_t index;
        Node* prev = nullptr, *next = nullptr;

        Node() = default;

        Node(std::pair<const KeyType, ValueType> element_, size_t dist_ = 0, size_t index_ = 0) : element(element_), dist(dist_), index(index_) {}

        ~Node() {
        }
    };

    class const_iterator {
    public:
        explicit const_iterator(const Node* pointer) {
          current_ = pointer;
        }

        const_iterator() = default;

        const std::pair<const KeyType, ValueType>& operator*() const {
          return current_->element;
        }

        const std::pair<const KeyType, ValueType>* operator->() const {
          return &(current_->element);
        }

        const_iterator& operator++() {
          current_ = current_->next;
          return *this;
        }

        const_iterator operator++(int) {
          const_iterator now(*this);
          current_ = current_->next;
          return now;
        }

        bool operator==(const const_iterator& other) const {
          return current_ == other.current_;
        }

        bool operator!=(const const_iterator& other) const {
          return current_ != other.current_;
        }

    private:
        const Node* current_;
    };

    class iterator {
    public:
        explicit iterator(Node* pointer) {
          current_ = pointer;
        }

        iterator() = default;

        std::pair<const KeyType, ValueType>& operator*() const {
          return current_->element;
        }

        std::pair<const KeyType, ValueType>* operator->() const {
          return &(current_->element);
        }

        iterator& operator++() {
          current_ = current_->next;
          return *this;
        }

        iterator operator++(int) {
          iterator now(*this);
          current_ = current_->next;
          return now;
        }

        bool operator==(const iterator& other) const {
          return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const {
          return current_ != other.current_;
        }
    private:
        Node* current_;
    };

    const_iterator begin() const {
      return const_iterator(first_);
    }

    const_iterator end() const {
      return const_iterator(nullptr);
    }

    iterator begin() {
      return iterator(first_);
    }

    iterator end() {
      return iterator(nullptr);
    }

    HashMap() {
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
      capacity_ = 1;
      link_.resize(capacity_);
    }

    HashMap(iterator start, iterator fin, Hash hasher = Hash()) : hasher_(hasher) {
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
      capacity_ = 1;
      link_.resize(capacity_);
      for (auto it = start; it != fin; ++it) {
        insert(*it);
      }
    }

    HashMap(const_iterator start, const_iterator fin, Hash hasher = Hash()) : hasher_(hasher) {
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
      capacity_ = 1;
      link_.resize(capacity_);
      for (auto it = start; it != fin; ++it) {
        insert(*it);
      }
    }

    HashMap(std::initializer_list<std::pair<const KeyType, ValueType>> lst, Hash hasher = Hash()) : hasher_(hasher) {
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
      capacity_ = 1;
      link_.resize(capacity_);
      for (auto x : lst) {
        insert(x);
      }
    }

    HashMap(Hash hasher) : hasher_(hasher) {
      size_ = 0;
      capacity_ = 1;
      link_.resize(capacity_);
      first_ = nullptr;
      last_ = nullptr;
    }

    HashMap(const HashMap& other) : hasher_(other.hasher_) {
      first_ = nullptr;
      last_ = nullptr;
      *this = other;
    }

    ~HashMap() {
      Node* cur = first_;
      if (cur != nullptr) {
        while (cur->next != nullptr) {
          cur = cur->next;
          link_[cur->prev->index] = nullptr;
          delete cur->prev;
        }
        link_[last_->index] = nullptr;
        delete last_;
      }
      size_ = 0;
      first_ = nullptr;
      last_ = nullptr;
    }

    void clear() {
      Node* cur = first_;
      if (cur != nullptr) {
        while (cur->next != nullptr) {
          cur = cur->next;
          delete cur->prev;
        }
        delete last_;
      }
      size_ = 0;
      capacity_ = 1;
      first_ = nullptr;
      last_ = nullptr;
      link_.clear();
    }

    Hash hash_function() const {
      return hasher_;
    }

    const_iterator find(const KeyType key) const {
      if (size_ == 0) {
        return const_iterator(end());
      }
      size_t start_bucket = hasher_(key) % capacity_;
      size_t index = start_bucket;
      for (size_t active_dist = 0; active_dist < capacity_; ++active_dist, ++index) {
        if (index == capacity_) {
          index = 0;
        }
        if (link_[index] != nullptr) {
          KeyType cur_key = link_[index]->element.first;
          if (cur_key == key) {
            return const_iterator(link_[index]);
          }
        } else if (index != start_bucket) {
          break;
        }
      }
      return const_iterator(end());
    }

    iterator find(const KeyType key) {
      if (size_ == 0) {
        return iterator(end());
      }
      size_t start_bucket = hasher_(key) % capacity_;
      size_t index = start_bucket;
      for (size_t active_dist = 0; active_dist < capacity_; ++active_dist, ++index) {
        if (index == capacity_) {
          index = 0;
        }
        if (link_[index] != nullptr) {
          KeyType cur_key = link_[index]->element.first;
          if (cur_key == key) {
            return iterator(link_[index]);
          }
        } else if (index != start_bucket) {
          break;
        }
      }
      return iterator(end());
    }

    void insert_inside(Node* add) {
      size_t start_bucket = hasher_(add->element.first) % capacity_;
      if (size_ == 0) {
        first_ = add;
        last_ = add;
        link_[start_bucket] = add;
        add->index = start_bucket;
        ++size_;
        return;
      }
      std::swap(add, last_);
      last_->prev = add;
      add->next = last_;

      int first_index = -1;
      for (size_t i = start_bucket, active_dist = 0; ; ++i, ++active_dist, ++(last_->dist)) {
        if (i == capacity_) {
          i = 0;
        }
        if (link_[i] == nullptr) {
          last_->index = i;
          link_[i] = last_;
          ++size_;
          break;
        } else {
          size_t cur_dist = link_[i]->dist;
          if (cur_dist < active_dist) {
            if (first_index == -1) {
              first_index = static_cast<int>(i);
            }
            last_->index = i;
            std::swap(link_[i], last_);
          }
        }
      }
      if (first_index != -1) {
        last_ = link_[first_index];
      }
    }

    void insert(std::pair <const KeyType, ValueType> element) {
      if (size_ > 0 && find(element.first) != end()) {
        return;
      }
      size_t start_bucket = hasher_(element.first) % capacity_;
      Node* add = new Node(element, 0, 0);
      if (size_ == 0) {
        first_ = add;
        last_ = add;
        link_[start_bucket] = add;
        add->index = start_bucket;
        ++size_;
        if (size_ * 2 >= capacity_) {
          size_ = 0;
          capacity_ *= 2;
          std::vector<Node*> who;
          link_ = std::vector<Node*>(capacity_);
          Node* cur = first_;
          if (cur != nullptr) {
            while (cur != nullptr) {
              cur->dist = 0;
              who.push_back(cur);
              cur = cur->next;
            }
          }
          for (auto x : who) {
            insert_inside(x);
          }
        }
        return;
      }
      std::swap(add, last_);
      last_->prev = add;
      add->next = last_;

      int first_index = -1;
      for (size_t i = start_bucket, active_dist = 0; ; ++i, ++active_dist, ++(last_->dist)) {
        if (i == capacity_) {
          i = 0;
        }
        if (link_[i] == nullptr) {
          last_->index = i;
          link_[i] = last_;
          if (first_index != -1) {
            last_ = link_[first_index];
          }
          ++size_;
          if (size_ * 2 >= capacity_) {
            size_ = 0;
            capacity_ *= 2;
            std::vector<Node*> who;
            link_ = std::vector<Node*>(capacity_);
            Node* cur = first_;
            if (cur != nullptr) {
              while (cur != nullptr) {
                cur->dist = 0;
                who.push_back(cur);
                cur = cur->next;
              }
            }
            for (auto x : who) {
              insert_inside(x);
            }
            return;
          }
          break;
        } else {
          size_t cur_dist = link_[i]->dist;
          if (cur_dist < active_dist) {
            if (first_index == -1) {
              first_index = static_cast<int>(i);
            }
            last_->index = i;
            std::swap(link_[i], last_);
          }
        }
      }
      if (first_index != -1) {
        last_ = link_[first_index];
      }
    }

    void erase(const KeyType key) {
      if (size_ == 0 || (size_ > 0 && find(key) == end())) {
        return;
      }
      int start_bucket = hasher_(key) % capacity_;
      size_t pos = 0;
      for (size_t i = start_bucket, active_dist = 0; active_dist < capacity_; ++i, ++active_dist) {
        if (i == capacity_) {
          i = 0;
        }
        if (link_[i] == nullptr) {
          continue;
        }
        size_t index = (start_bucket + active_dist) % capacity_;
        KeyType cur_key = link_[index]->element.first;
        if (cur_key == key) {
          if (link_[i]->next != nullptr) {
            if (link_[i]->prev != nullptr) {
              link_[i]->next->prev = link_[i]->prev;
            } else {
              link_[i]->next->prev = nullptr;
            }
          }
          if (link_[i]->prev != nullptr) {
            if (link_[i]->next != nullptr) {
              link_[i]->prev->next = link_[i]->next;
            } else {
              link_[i]->prev->next = nullptr;
            }
          }
          if (last_ == link_[i]) {
            if (link_[i]->prev != nullptr) {
              last_ = link_[i]->prev;
            } else {
              last_ = nullptr;
            }
          }
          if (first_ == link_[i]) {
            if (link_[i]->next != nullptr) {
              first_ = link_[i]->next;
            } else {
              first_ = nullptr;
            }
          }
          delete link_[i];
          link_[i] = nullptr;
          --size_;
          pos = i;
          break;
        }
      }
      for (size_t i = pos + 1; ; ++i) {
        if (i >= capacity_) {
          i -= capacity_;
        }
        if (link_[i] == nullptr || link_[i]->dist == 0) {
          break;
        }
        int prev = i - 1;
        if (prev < 0) {
          prev += capacity_;
        }
        link_[i]->index = prev;
        std::swap(link_[i], link_[prev]);
        link_[prev]->dist -= 1;
      }
    }

    HashMap& operator=(const HashMap& other) {
      if (this == &other) {
        return *this;
      }
      hasher_ = other.hasher_;
      capacity_ = other.capacity_;
      size_ = other.size_;
      link_ = std::vector<Node*>(other.capacity_);
      Node* cur = other.first_;
      if (cur != nullptr) {
        while (cur != nullptr) {
          auto x = new Node(cur->element, cur->dist, cur->index);
          link_[cur->index] = x;
          if (first_ == nullptr) {
            first_ = x;
          } else {
            last_->next = x;
            x->prev = last_;
          }
          last_ = x;
          cur = cur->next;
        }
      }
      return *this;
    }

    ValueType& operator[](const KeyType& key) {
      auto it = find(key);
      if (it == end()) {
        std::pair<const KeyType, ValueType> el(std::make_pair(key, ValueType()));
        insert(el);
      }
      it = find(key);
      return it->second;
    }

    const ValueType& at(KeyType key) const {
      auto it = find(key);
      if (it == end()) {
        throw std::out_of_range("out_of_range");
      }
      return it->second;
    }

    size_t size() const {
      return size_;
    }

    bool empty() const {
      return (size_ == 0);
    }

private:
    std::vector<Node*> link_;
    Node* first_;
    Node* last_;
    Hash hasher_;

    size_t size_;
    size_t capacity_;
};