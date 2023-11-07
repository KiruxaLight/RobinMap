#include "hash_map.h"
#include <iostream>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <map>
#include <cassert>

void fail(const char *message) {
  std::cerr << "Fail:\n";
  std::cerr << message;
  std::cout << "I want to get WA\n";
  exit(0);
}

struct StrangeInt {
    int x;
    static int counter;
    StrangeInt() {
      ++counter;
    }
    StrangeInt(int x): x(x) {
      ++counter;
    }
    StrangeInt(const StrangeInt& rs): x(rs.x) {
      ++counter;
    }
    bool operator ==(const StrangeInt& rs) const {
      return x == rs.x;
    }

    static void init() {
      counter = 0;
    }

    ~StrangeInt() {
      --counter;
    }

    friend std::ostream& operator <<(std::ostream& out, const StrangeInt& x) {
      out << x.x;
      return out;
    }
};
int StrangeInt::counter;

namespace std {
    template<> struct hash<StrangeInt> {
        size_t operator()(const StrangeInt& x) const {
          return x.x;
        }
    };
}

namespace internal_tests {

/* check that hash_map provides correct interface
 * in terms of constness */
    void const_check() {
      const HashMap<int, int> map{{1, 5}, {3, 4}, {2, 1}};
      std::cerr << "check constness\n";
      if (map.empty())
        fail("incorrect empty method");

      static_assert(std::is_same<
              HashMap<int, int>::const_iterator,
              decltype(map.begin())
      >::value, "'begin' returns not a const iterator");
      auto hash_f = map.hash_function();

      HashMap<int, int>::const_iterator it = map.find(3);
      if (it->second != 4)
        fail("not found 3, incorrect find or insert");
      it = map.find(7);
      if (it != map.end())
        fail("found 7? incorrect find or insert");

      static_assert(std::is_same<
              const int,
              std::remove_reference<decltype(map.at(1))>::type
      >::value, "'At' returns non const");
      std::cerr << "ok!\n";
    }

/* check that 'at' raises std::out_of_range */
    void exception_check() {
      const HashMap<int, int> map{{2, 3}, {-7, -13}, {0, 8}};
      std::cerr << "check exception...\n";
      try {
        auto cur = map.at(8);
        std::cerr << cur << "\n";
      }
      catch (const std::out_of_range& e) {
        std::cerr << "ok!\n";
        return;
      }
      catch (...) {
        fail("'at' doesn't throw std::out_of_range");
      }
      fail("'at' doesn't throw anything");
    }

/* check if class correctly implements destructor */
    void check_destructor() {
      std::cerr << "check destructor... ";

      StrangeInt::init();
      {
        HashMap<StrangeInt, int> s{
                {5, 4},
                {3, 2},
                {1, 0}
        };
        if (s.size() != 3)
          fail("wrong size");

      }
      if (StrangeInt::counter)
        fail("wrong destructor (or constructors)");

      {
        HashMap<StrangeInt, int> s{
                {-3, 3},
                {-2, 2},
                {-1, 1}
        };

        HashMap<StrangeInt, int> s1(s);

        s1.insert(std::make_pair(0, 0));
        HashMap<StrangeInt, int> s2(s1);

        if (s1.find(0) == s1.end())
          fail("wrong find");

      }
      if (StrangeInt::counter)
        fail("wrong destructor (or constructors)");
      std::cerr << "ok!\n";
    }


/* check operator [] for reference correctness */
    void reference_check() {
      HashMap<int, int> map{{3, 4}, {3, 5}, {4, 7}, {-1, -3}};
      std::cerr << "check references... ";
      map[3] = 7;
      if (map[3] != 7 || map[0] != 0)
        fail("incorrect [ ]");
      auto it = map.find(4);
      if (it == map.end())
        fail("not found 4, incorrect find or insert");
      it->second = 3;
      auto cur = map.find(4);
      if (cur->second != 3)
        fail("can't modificate through iterator");
      std::cerr << "ok!\n";
    }

    size_t stupid_hash(int /*x*/) {
      return 0;
    }

/* check custom hash functions */
    void hash_check() {
      std::cerr << "check hash functions\n";
      struct Hasher {
          std::hash<std::string> hasher;
          size_t operator()(const std::string& s) const {
            return hasher(s);
          }
      };

      HashMap<std::string, std::string, Hasher> map{
              {"aba", "caba"},
              {"simple", "case"},
              {"test", "test"}
      };
      for (auto cur : map)
        std::cerr << cur.first << " " << cur.second << "\n";
      auto simple_hash = [](unsigned long long x) -> size_t {
          return x % 17239;
      };
      HashMap<int, std::string, decltype(simple_hash)> second_map(simple_hash);
      second_map.insert(std::make_pair(0, "a"));

      second_map.insert(std::make_pair(0, "b"));

      second_map[17239] = "check";

      auto second_hash_fn = second_map.hash_function();
      if (second_hash_fn(17239) != 0)
        fail("wrong hash function in class");

      if (second_map[0] != "a" || second_map[17239] != "check")
        fail("incorrect insert or [ ]");

      for (auto cur : second_map)
        std::cerr << cur.first << " " << cur.second << "\n";

      HashMap<int, int, std::function<size_t(int)>> stupid_map(stupid_hash);
      auto stupid_hash_fn = stupid_map.hash_function();
      for(int i = 0; i < 1000; ++i) {
        stupid_map[i] = i + 1;
        if (stupid_hash_fn(i))
          fail("wrong hash function in class");
      }
      if (stupid_map.size() != 1000)
        fail("Wrong size");
      std::cerr << "ok!\n";
    }

/* check copy constructor and operator = */
    void check_copy() {
      std::cerr << "check copy correctness...\n";
      HashMap<int, int> first;
      HashMap<int, int> second(first);

      second.insert(std::make_pair(1, 1));
      HashMap<int, int> third(second.begin(), second.end());
      third[0] = 5;
      if (third.size() != 2)
        fail("Wrong size");
      first = third;

      second = second = first;
      if (first.find(0)->second != 5)
        fail("wrong find");
      if (second[0] != 5)
        fail("wrong [ ]");
      std::cerr << "ok!\n";

    }

/* check if iterator and const_iterator are implemented correctly */
    void check_iterators() {
      std::cerr << "check iterators...\n";
      {
        HashMap<int, int> first{{0, 0}};

        HashMap<int, int>::iterator just_iterator;
        HashMap<int, int>::iterator it = first.begin();
        static_assert(std::is_same<
                const int,
                std::remove_reference<decltype(it->first)>::type

        >::value, "Iterator's key type isn't const");
        if (it++ != first.begin())
          fail("bad post ++");
        if (!(it == first.end()))
          fail("bad post ++");
        if (++first.begin() != first.end())
          fail("bad pre ++");
        first.erase(0);
        if (first.begin() != first.end())
          fail("bad begin or end");
        just_iterator = first.begin();
      }

      {
        const HashMap<int, int> first{{1, 1}};
        HashMap<int, int>::const_iterator just_iterator;
        HashMap<int, int>::const_iterator it = first.begin();

        if (it++ != first.begin())
          fail("bad post ++");
        if (!(it == first.end()))
          fail("bad post ++");
        if (++first.begin() != first.end())
          fail("bad pre ++");
        just_iterator = it;
      }

      std::cerr << "ok!\n";
    }

    void run_all() {
      const_check();
      exception_check();
      reference_check();
      hash_check();
      check_destructor();
      check_copy();
      check_iterators();
    }
} // namespace internal_tests

void assert_map_equal(HashMap<int, int> map, std::unordered_map<int, int> orig_map) {
  std::vector<std::pair<int, int>> vec1, vec2;
  for(auto it = map.begin(); it != map.end(); ++it) {
    vec1.push_back({it->first, it->second});
  }
  for(auto it = orig_map.begin(); it != orig_map.end(); ++it) {
    vec2.push_back({it->first, it->second});
  }
  std::sort(vec1.begin(), vec1.end());
  std::sort(vec2.begin(), vec2.end());
  assert(vec1.size() == vec2.size());
  for (int i = 0; i < vec1.size(); ++i) {
    assert(vec1[i] == vec2[i]);
  }
}

int main() {
  internal_tests::run_all();

  HashMap<int, int> map;
  using HashMapIterator = HashMap<int, int>::iterator;
  int n;
  std::cin >> n;
  for (int i = 0; i < n; ++i) {
    char code;
    int key;
    std::cin >> code;
    if (code != '!' && code != '<')
      std::cin >> key;
    if (code == '-')
      map.erase(key);
    else if (code == '?') {
      auto it = map.find(key);
      std::cout << (it == map.end() ? -1 : it->second) << "\n";
    } else if (code == '+') {
      int val;
      std::cin >> val;

      map[key] = val;
    } else if (code == '<') {
      for(HashMapIterator it = map.begin(); it != map.end(); ++it)
        std::cout << it->first << " " << it->second << "\n";
    } else if (code == '!')
      map.clear();
  }

  std::cout << map.size() << "\n";
  return 0;
}