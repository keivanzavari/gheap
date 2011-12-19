// Tests for gheap.
//
// Pass -DGHEAP_CPP11 to compiler for gheap_cpp11.hpp tests,
// otherwise gheap_cpp03.hpp will be tested.

#ifdef GHEAP_CPP11
#  include "gheap_cpp11.hpp"
#else
#  include "gheap_cpp03.hpp"
#endif

#include <cstdlib>    // for srand(), rand()
#include <iostream>   // for cout
#include <vector>

using namespace std;

namespace {

// Verifies correctness of parent and child index calculations.
template <size_t Fanout, size_t PageChunks>
void test_parent_child(const size_t start_index, const size_t n)
{
  assert(start_index > 0);
  assert(start_index <= SIZE_MAX - n);

  typedef gheap<Fanout, PageChunks> heap;

  for (size_t i = 0; i < n; ++i) {
    const size_t u = start_index + i;
    size_t v = heap::get_child_index(u);
    if (v < SIZE_MAX) {
      assert(v > u);
      v = heap::get_parent_index(v);
      assert(v == u);
    }

    v = heap::get_parent_index(u);
    assert(v < u);
    v = heap::get_child_index(v);
    assert(v <= u && u - v < Fanout);
  }

  cout << "test_parent_child(start_index=" << start_index << ", n=" << n <<
      ", Fanout=" << Fanout << ", PageChunks=" << PageChunks << ") OK" << endl;
}

// Verifies is_heap_until() and is_heap() correctness.
template <size_t Fanout, size_t PageChunks>
void test_is_heap(const size_t n)
{
  assert(n > 0);

  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;

  // Verify that ascending sorted array creates one-item heap.
  a.clear();
  for (size_t i = 0; i < n; ++i) {
    a.push_back(i);
  }
  assert(heap::is_heap_until(a.begin(), a.end()) == a.begin() + 1);
  assert(heap::is_heap(a.begin(), a.begin() + 1));
  if (n > 1) {
    assert(!heap::is_heap(a.begin(), a.end()));
  }

  // Verify that descending sorted array creates valid heap.
  a.clear();
  for (size_t i = 0; i < n; ++i) {
    a.push_back(n - i);
  }
  assert(heap::is_heap_until(a.begin(), a.end()) == a.end());
  assert(heap::is_heap(a.begin(), a.end()));

  // Verify that array containing identical items creates valid heap.
  a.clear();
  for (size_t i = 0; i < n; ++i) {
    a.push_back(n);
  }
  assert(heap::is_heap_until(a.begin(), a.end()) == a.end());
  assert(heap::is_heap(a.begin(), a.end()));
}

// Fills the given array with n random integers.
void init_array(vector<int> *const a, const size_t n)
{
  a->clear();
  a->reserve(n);

  srand(0);
  for (size_t i = 0; i < n; ++i) {
    a->push_back(rand());
  }
}

// Verifies that items in the given range are sorted in ascending order.
template <class RandomAccessIterator>
void assert_sorted_asc(const RandomAccessIterator &first,
    const RandomAccessIterator &last)
{
  assert(last > first);

  const size_t size = last - first;
  for (size_t i = 1; i < size; ++i) {
    assert(first[i] >= first[i - 1]);
  }
}

// Verifies that items in the given range are sorted in descending order.
template <class RandomAccessIterator>
void assert_sorted_desc(const RandomAccessIterator &first,
    const RandomAccessIterator &last)
{
  assert(last > first);

  const size_t size = last - first;
  for (size_t i = 1; i < size; ++i) {
    assert(first[i] <= first[i - 1]);
  }
}

bool inverse_less_comparer(const int a, const int b)
{
  return (b < a);
}

// Verifies correctness of heapsort built on top of "make_heap(); build_heap()".
template <size_t Fanout, size_t PageChunks>
void test_heapsort(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;

  // Verify ascending sorting with default less_comparer.
  init_array(&a, n);
  heap::make_heap(a.begin(), a.end());
  assert(heap::is_heap(a.begin(), a.end()));
  heap::sort_heap(a.begin(), a.end());
  assert_sorted_asc(a.begin(), a.end());

  // Verify descending sorting with custom less_comparer.
  init_array(&a, n);
  heap::make_heap(a.begin(), a.end(), inverse_less_comparer);
  assert(heap::is_heap(a.begin(), a.end(), inverse_less_comparer));
  heap::sort_heap(a.begin(), a.end(), inverse_less_comparer);
  assert_sorted_desc(a.begin(), a.end());

  cout << "test_heapsort(n=" << n << ", Fanout=" << Fanout << ", PageChunks=" <<
      PageChunks << ") OK" << endl;
}

// Verifies push_heap() correctness.
template <size_t Fanout, size_t PageChunks>
void test_push_heap(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;
  init_array(&a, n);

  for (size_t i = 0; i < n; ++i) {
    heap::push_heap(a.begin(), a.begin() + i + 1);
  }
  assert(heap::is_heap(a.begin(), a.end()));

  cout << "test_push_heap(n=" << n << ", Fanout=" << Fanout <<
      ", PageChunks=" << PageChunks << ") OK" << endl;
}

template <size_t Fanout, size_t PageChunks>
void test_pop_heap(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;
  init_array(&a, n);

  heap::make_heap(a.begin(), a.end());
  assert(heap::is_heap(a.begin(), a.end()));
  for (size_t i = 0; i < n; ++i) {
    heap::pop_heap(a.begin(), a.end() - i);
  }
  assert_sorted_asc(a.begin(), a.end());

  cout << "test_pop_heap(n=" << n << ", Fanout=" << Fanout <<
      ", PageChunks=" << PageChunks << ") OK" << endl;
}

// Verifies restore_heap_after_item_increase() correctness.
template <size_t Fanout, size_t PageChunks>
void test_restore_heap_after_item_increase(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;
  init_array(&a, n);

  heap::make_heap(a.begin(), a.end());
  assert(heap::is_heap(a.begin(), a.end()));
  for (size_t i = 0; i < n; ++i) {
    const size_t item_index = rand() % n;
    const int old_item = a[item_index];

    // Don't allow integer overflow.
    size_t fade = SIZE_MAX;
    do {
      // Division by zero is impossible here.
      a[item_index] = old_item + rand() % fade;
      fade /= 2;
    } while (a[item_index] < old_item);
    heap::restore_heap_after_item_increase(a.begin(), a.begin() + item_index);
    assert(heap::is_heap(a.begin(), a.end()));
  }

  cout << "test_restore_heap_after_item_increase(n=" << n << ", Fanout=" <<
      Fanout << ", PageChunks=" << PageChunks << ") OK" << endl;
}

// Verifies restore_heap_after_item_decrease() correctness.
template <size_t Fanout, size_t PageChunks>
void test_restore_heap_after_item_decrease(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;
  init_array(&a, n);

  heap::make_heap(a.begin(), a.end());
  assert(heap::is_heap(a.begin(), a.end()));
  for (size_t i = 0; i < n; ++i) {
    const size_t item_index = rand() % n;
    const int old_item = a[item_index];

    // Don't allow integer underflow.
    size_t fade = SIZE_MAX;
    do {
      // Division by zero is impossible here.
      a[item_index] = old_item - rand() % fade;
      fade /= 2;
    } while (a[item_index] > old_item);
    heap::restore_heap_after_item_decrease(a.begin(), a.begin() + item_index,
        a.end());
    assert(heap::is_heap(a.begin(), a.end()));
  }

  cout << "test_restore_heap_after_item_decrease(n=" << n << ", Fanout=" <<
      Fanout << ", PageChunks=" << PageChunks << ") OK" << endl;
}

template <size_t Fanout, size_t PageChunks>
void test_remove_from_heap(const size_t n)
{
  typedef gheap<Fanout, PageChunks> heap;

  vector<int> a;
  init_array(&a, n);

  heap::make_heap(a.begin(), a.end());
  assert(heap::is_heap(a.begin(), a.end()));
  for (size_t i = 0; i < n; ++i) {
    const size_t item_index = rand() % (n - i);
    heap::remove_from_heap(a.begin(), a.begin() + item_index, a.end() - i);
    assert(heap::is_heap(a.begin(), a.end() - i - 1));
  }

  cout << "test_remove_from_heap(n=" << n << ", Fanout=" << Fanout <<
      ", PageChunks=" << PageChunks << ") OK" << endl;
}

// Runs all tests for the given Fanout and PageChunks.
template <size_t Fanout, size_t PageChunks>
void test_all()
{
  // Verify parent-child calculations for indexes close to zero and
  // indexes close to SIZE_MAX.
  static const size_t n = 1000000;
  test_parent_child<Fanout, PageChunks>(1, n);
  test_parent_child<Fanout, PageChunks>(SIZE_MAX - n, n);

  test_is_heap<Fanout, PageChunks>(1);
  test_is_heap<Fanout, PageChunks>(2);
  test_is_heap<Fanout, PageChunks>(3);
  test_is_heap<Fanout, PageChunks>(1000);

  test_heapsort<Fanout, PageChunks>(1);
  test_heapsort<Fanout, PageChunks>(2);
  test_heapsort<Fanout, PageChunks>(3);
  // heapsort for Fanout = 1 is slow, so sort less items.
  test_heapsort<Fanout, PageChunks>(Fanout > 1 ? 100000 : 1000);

  test_push_heap<Fanout, PageChunks>(1);
  test_push_heap<Fanout, PageChunks>(2);
  test_push_heap<Fanout, PageChunks>(3);
  test_push_heap<Fanout, PageChunks>(1000);

  test_pop_heap<Fanout, PageChunks>(1);
  test_pop_heap<Fanout, PageChunks>(2);
  test_pop_heap<Fanout, PageChunks>(3);
  test_pop_heap<Fanout, PageChunks>(1000);

  test_restore_heap_after_item_increase<Fanout, PageChunks>(1);
  test_restore_heap_after_item_increase<Fanout, PageChunks>(2);
  test_restore_heap_after_item_increase<Fanout, PageChunks>(3);
  test_restore_heap_after_item_increase<Fanout, PageChunks>(1000);

  test_restore_heap_after_item_decrease<Fanout, PageChunks>(1);
  test_restore_heap_after_item_decrease<Fanout, PageChunks>(2);
  test_restore_heap_after_item_decrease<Fanout, PageChunks>(3);
  test_restore_heap_after_item_decrease<Fanout, PageChunks>(1000);

  test_remove_from_heap<Fanout, PageChunks>(1);
  test_remove_from_heap<Fanout, PageChunks>(2);
  test_remove_from_heap<Fanout, PageChunks>(3);
  test_remove_from_heap<Fanout, PageChunks>(1000);
}

}  // End of anonymous namespace.

int main()
{
  test_all<1, 1>();
  test_all<2, 1>();
  test_all<3, 1>();
  test_all<4, 1>();
  test_all<101, 1>();

  test_all<1, 2>();
  test_all<2, 2>();
  test_all<3, 2>();
  test_all<4, 2>();
  test_all<101, 2>();

  test_all<1, 3>();
  test_all<2, 3>();
  test_all<3, 3>();
  test_all<4, 3>();
  test_all<101, 3>();

  test_all<1, 4>();
  test_all<2, 4>();
  test_all<3, 4>();
  test_all<4, 4>();
  test_all<101, 4>();

  test_all<1, 101>();
  test_all<2, 101>();
  test_all<3, 101>();
  test_all<4, 101>();
  test_all<101, 101>();

  cout << "All tests DONE" << endl;
}
