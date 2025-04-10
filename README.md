# Project4: ESet

**Deadline: 2025/5/12 18:00**

## Background

`std::set` 固然好用，但它的常数极其之大，为什么？

本次大作业中，我们想实现一个和它功能类似的数据结构；我们还想探究 `std::set` 慢的原因。

本次大作业旨在练习：动态查找表（跳表、AVL树、红黑树、无旋treap等）的实现；性能调优；信息搜集及报告能力。

## Goal

#### 1. 成为set

你需要start from scratch，**使用红黑树**实现一个 `template<class Key,class Compare = std::less<Key>>` 的 class，名为`ESet`。

`ESet` 拥有迭代器 `iterator`。在`ESet`中插入元素，之前老迭代器不会失效；删除时，指向删除点位置的迭代器失效，但是其他迭代器不会失效，包括`end()`。**（请注意这点与std::set的特性不同）**

Set的`iterator`本身不支持修改值，实质与`const_iterator`无异。

> **Aside：对于其它容器（比如双向链表），同时实现`iterator`和`const_iterator`的最佳方式是使用继承，而非复制一遍代码。**
>
> 比如，`std::vector`的迭代器实现是：
>
> ```c++
> typedef __gnu_cxx::__normal_iterator<pointer, vector> iterator;
> typedef __gnu_cxx::__normal_iterator<const_pointer, vector> const_iterator;
> typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
> typedef std::reverse_iterator<iterator>		reverse_iterator;
> ```



为了减少本次大作业的workload，我们只需要ESet支持以下操作。如果你对操作细节有疑问，请参考 `std::set` 的相关功能。

- ```C++
  ESet();
  ~ESet();
  ```

  构造和析构函数。构造函数最高可接受复杂度 $O(1)$ ，析构函数最高可接受复杂度 $O(n\log n)$ 。

  

- ```c++
  template< class... Args >
  std::pair<iterator, bool> emplace( Args&&... args ); 
  ```

  将一个元素args放入ESet中。如已有该元素，则不插入。返回值第一维是对应位置的迭代器，第二维表示ESet是否真正发生过改变。最高可接受复杂度 $O(\log n)$。

  > Note：在std::set中，使用emplace而非insert可以避免不必要的copy或move操作。
  >
  > 请注意，虽然函数原型是可变长参数，但你只需要处理emplace一个元素的情况即可。如传入多个元素，实际行为可以任选。
  >
  > 在std::set中，emplace会使用std::forward调用元素的构造函数。如果传入多个参数，则会调用多个参数版本的构造函数，如：
  >
  > ```c++
  > std::set<std::pair<int,std::string>> s;
  > s.emplace(1,"a");
  > ```



- 	```c++
	size_t erase( const Key& key );
	```

  将key从ESet中删除。返回值为`ESet`中是否有key（0或1）。最高可接受复杂度 $O(\log n)$ 。

  

- ```c++
  iterator find( const Key& key ) const;
  ```
  
  找到Key对应位置的iterator并返回。如果没找到该元素，返回`end()`。最高可接受复杂度 $O(\log n)$ 。
  
  
  
- ```C++
  ESet( const ESet& other );
  ESet& operator=( const ESet& other );
  ```

​		复制一个ESet到另一个。复制之后两个ESet应该是分离的，即对其中一个的操作不会影响另一个。最高可接受复杂度 $O(n\log n)$ 。



- ```c++
  ESet( ESet&& other );
  ESet& operator=( ESet&& other ) noexcept;
  ```
  
  移动一个ESet到另一个。移动之后不应该有新增的空间，other应当被销毁。最高可接受复杂度 $O(n\log n)$ 。
  
  
  
- ```c++
  size_t range( const Key& l, const Key& r ) const;
  ```

  返回[l,r]内元素的个数。若l>r，返回0。最高可接受复杂度 $O(\log n)$ 。

  
  
- ```c++
  size_t size() const noexcept;
  ```

  返回当前`ESet`总元素个数。最高可接受复杂度 $O(1)$ 。
  
  
  
- ```c++
  iterator lower_bound( const Key& key ) const;
  iterator upper_bound( const Key& key ) const;
  ```
  
  根据上/下界二分查找，返回对应位置的迭代器，期望功能同`lower_bound`和`upper_bound`的常规含义。

  具体来说，lower_bound需找到最小的>=key的元素；upper_bound需找到最小的>key的元素。如果没有这样的元素，返回`end()`。

  最高可接受复杂度 $O(\log n)$ 。
  
  
  
- ```c++
  iterator begin() const noexcept;
  iterator end() const noexcept;
  ```
  
  首/尾迭代器。最高可接受复杂度 $O(1)$ 。



- ```C++
  *it; ++it; --it; it++; it--; it1!=it2; it1==it2;
  ```

	迭代器取值（返回值需要是`const`）、加减、比较。如果迭代器已经是`end()`，`(*it)`应当throw，++应当什么都不做；如果迭代器已经是`begin()`，--应当什么都不做。最高可接受复杂度 $O(\log n)$ 。

------

完成本任务后，你可以将`src.hpp`提交在ACMOJ的`ESet-Basic`上评测（github上也提供了测试点的子集）。评测点的数据规模是：

| 总操作数 N ~ 100000 |         |       |      |              |       |          |
| ------------------- | ------- | ----- | ---- | ------------ | ----- | -------- |
| 操作                | emplace | erase | find | `=` operator | range | it相关   |
| 个数约              | N/2     | N/6   | N/9  | 25           | N/18  | 剩余所有 |

**对于本问，你需要提交一个规范的class，无需提交main程序。**

作为一个鲁棒的数据结构，你不能提前假设N的大小，即不能使用静态数组。



#### 2. 嘲讽set

理论上来说，你的程序和STL都使用了红黑树，为什么会有性能差距呢？在本问中，你需要设计实验、撰写报告，来解释这个现象。切记，**你在本问中提出的所有claim，均需要有实验（ablation study）来佐证。**

a) 设计数据和测试程序，比较你的代码和STL在同等条件下的性能。你可以关注**每种操作的平均时长**、**每种操作的次数**、**调整平衡的次数**等数据。

b) 如果你的性能更优，尝试解释为什么（需要对比STL进行消融实验，解释清楚即可完成本问）？如果你的性能更劣，尝试分析并找到优化的方向。

- 常数优化行不行？（如宏优化、内存优化等）

- 你和STL之间的实现是否有某些区别或新特性？消除这些区别是否能减小差距？如果还有差距，来源是什么？

- 换个数据结构，是否可能在复杂度正确的情况下，提高某些操作的性能？设计简短的代码和计时，证明之。

- 你可以考虑的数据结构有：
  - 跳表
  - AVL树
  - B+树（内存层面）
  - treap
  - 其它平衡树
  - ...

- 尝试解释为什么某些数据结构能超越STL；尝试解释为什么STL不使用这些数据结构。

  

**本问考察的是report和代码是否能够佐证你的结论，你需要针对每种操作给出严格的测试数据&计时&性能分析，但不用和(1)问一样完成大段面向对象的代码。**

请查阅 *perf* 和 *gperf* 的相关内容。或许可以了解*性能调优*。



#### 3. 超越set

现在，考虑set是一个用户数据库系统，我们需要高频对它做备份和fork操作。于是，我们希望`=`运算符（拷贝构造、移动构造）的复杂度为 $\textbf{O(1)}$ 。当然 `=` 看起来的效果还是硬拷贝， `a = b` 之后，修改`b`不会影响到`a`。

>  ”啊？！这怎么可能？！“

实际上，`=`操作符本应该做的事情被均摊到`emplace`和`erase`里面去了。

> （纳尼？这样的话跑任务1岂不是会变慢？！你可以在完成本任务后对比一下）

请查阅*可持久化数据结构*相关的信息。

ACMOJ上，speedtest的数据规模是：

| 总操作数 N ~ 3000000 |         |       |      |              |       |          |
| -------------------- | ------- | ----- | ---- | ------------ | ----- | -------- |
| 操作                 | emplace | erase | find | `=` operator | range | it相关   |
| 个数约               | N/3     | N/9   | N/6  | N/100        | N/9   | 剩余所有 |

**本问是传统题目，提交的程序需要包含main，输入/输出格式请参考test/1.in和test/1.out。** 要实现的功能已经在1.cpp中描述，至于怎么实现就看你了[Doge]

你可以针对数据点的特性，对程序的任何部分进行优化，包括重写main函数、使用面向过程、抛弃部分未使用的特性、离线处理输入等。但是，请注意，作为一个鲁棒的数据结构，你不能提前假设N的大小，即不能使用静态数组，包括`test/1.cpp`当中的`ESet<long long> s[25]`。

请**自己调研**，找出一种最优的数据结构，想办法让你的代码跑得尽可能快！

> Some Hints：
>
> - **无旋treap**或许是一个能过本题，但并非最优的解法。
> - 或许不用严格地实现iterator
> - 1.cpp中的有些操作在某些情况下或许是多余的
> - 自己实现的内存申请、读写、pair、vector等似乎都会比STL更快。为什么？
> - malloc比new快；一次性申请大小为N的内存，比N次申请大小为1的内存快。
> - 非递归或许比递归快。
> - 可以通过gperf查看哪部分代码最耗时，并进行针对性优化。
> - inline、宏函数、cache的局部性、编译选项等，或许是拼图的最后一角。

speedtest按照通过测试点的比例给分；本届总用时前5名将会有图书奖品。



## Grading

- Task 1： 提交.hpp，50%

- Task 2： 提交Report，15%
- Task 3： 提交.cpp，25%
- QA&Code Review：10%

## Copyright

本次大作业参考了22级John班TA陆潇扬设计的ESet大作业，由23级John班TA汪畋宇重造。

本项目为23级John班数据结构课的第四个大作业，也可供后续John班参考使用。

