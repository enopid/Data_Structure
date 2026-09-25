# MyOrderedTree

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **레드블랙 트리와 컨테이너의 핵심 로직은 직접 설계하고 구현했습니다.**
- `Private/Main.cpp`의 유효성 테스트와 성능 벤치마크는 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

STL의 정렬 연관 컨테이너를 참고해 레드블랙 트리 기반의 `Set`, `Map`, `MultiSet`, `MultiMap`을 구현했습니다. 공통 트리인 `MyRedBlackTree`가 노드 소유권과 균형 복구를 담당하고, 각 어댑터가 키와 값의 추출 방식을 제공합니다.

### 구현 중심점

- 부모·왼쪽·오른쪽 링크를 사용하는 노드 기반 이진 탐색 트리
- 삽입과 삭제 후 색상 변경 및 좌우 회전을 통한 레드블랙 규칙 복구
- 단일 NIL 노드를 공유하는 리프 표현
- 컴파일 타임 `IsMulti` 분기를 통한 중복 키 허용 여부 구분
- 중위 순회 기반 양방향 iterator
- 깊은 복사, 이동 및 이동된 원본의 재사용 지원

## 포함 기능

| 구분 | 기능 |
| --- | --- |
| 공통 트리 | `Insert`, `Find`, `Remove`, `Clear`, `Size` |
| 순회 | `begin`, `end`, `cbegin`, `cend`, 전위·후위 `++`/`--` |
| 객체 관리 | 복사·이동 생성, 복사·이동 대입, 자기 대입 |
| 고유 키 | `MyOrderedSet`, `MyOrderedMap` |
| 중복 키 | `MyOrderedMultiSet`, `MyOrderedMultiMap` |

## 분석

### 유효성 테스트

Release x64에서 공개 API의 정렬 순회, 검색, 삽입·삭제, 복사·이동 및 네 어댑터의 중복 정책을 확인했습니다. 무작위 테스트는 30,000회의 삽입·탐색·삭제를 수행하며 크기와 탐색 결과를 매회, 정렬된 전체 원소를 100회마다 `std::set`과 비교합니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| 고유 키·검색 | 중복 거부, 정렬 순회, 존재·부재 키 검색 | PASS |
| 양방향 iterator | `--end()`, 역순 순회, 후위 증가 | PASS |
| 삭제 유형 | 리프, 자식 1개, 자식 2개, 루트, 없는 키 | PASS |
| 무작위 비교 | 30,000회 삽입·탐색·삭제를 `std::set`과 비교 | PASS |
| Multi 컨테이너 | MultiSet·MultiMap 중복 키 | PASS |
| Map | 중복 거부, 키 순서 순회, 값 보존 | PASS |
| 복사·이동 | 독립 복사, 자기 대입, 이동 후 원본 재사용 | PASS |
| Clear | 빈 순회, 전체 삭제, 삭제 후 재사용 | PASS |

```text
[PASS 1/8] Unique insertion, sorted iteration, and Find
[PASS 2/8] Bidirectional iterator movement
[PASS 3/8] Leaf, one-child, two-child, root, and missing-key removal
[PASS 4/8] 30,000 random operations against std::set
[PASS 5/8] MultiSet and MultiMap duplicate handling
[PASS 6/8] Map insertion and ordered iteration
[PASS 7/8] Copy, move, self-assignment, and moved-from reuse
[PASS 8/8] Clear and empty iteration

8 passed, 0 failed
```

<details>
<summary>1. 고유 키 삽입·검색 테스트</summary>

```cpp
MyOrderedSet<int> values;
for (int value : {7, 3, 9, 1, 5, 8, 10, 3}) values.Insert(value);
require(values.Size() == 7, "set accepted a duplicate key");
require(collect_keys(values) == std::vector<int>({1, 3, 5, 7, 8, 9, 10}),
        "in-order iteration is not sorted");
require(*values.Find(5) == 5, "Find failed for an existing key");
require(values.Find(99) == values.end(), "Find returned a node for a missing key");
```

</details>

<details>
<summary>2. 양방향 iterator 테스트</summary>

```cpp
auto it = values.end();
std::vector<int> reversed;
for (int count = 0; count < values.Size(); ++count) {
    --it;
    reversed.push_back(*it);
}
require(reversed == std::vector<int>({7, 6, 5, 4, 3, 2, 1}),
        "reverse iteration order is wrong");

auto post = values.begin();
auto old = post++;
require(*old == 1 && *post == 2, "post-increment semantics are wrong");
```

</details>

<details>
<summary>3. 삭제 유형 테스트</summary>

```cpp
for (int value : {20, 10, 30, 5, 15, 25, 40, 1, 7, 12, 17, 22, 27, 35, 50}) {
    values.Insert(value);
    expected.insert(value);
}
for (int value : {1, 5, 30, 20, 50, 999}) {
    values.Remove(value);
    expected.erase(value);
    require(values.Size() == static_cast<int>(expected.size()),
            "size changed incorrectly after removal");
    require(collect_keys(values) == std::vector<int>(expected.begin(), expected.end()),
            "removal damaged tree ordering or connectivity");
}
```

</details>

<details>
<summary>4. std::set과 30,000회 무작위 연산 비교 테스트</summary>

```cpp
for (int operation = 0; operation < 2000; ++operation) {
    const int value = value_distribution(random);
    if (insert_operation(random)) {
        actual.Insert(value);
        expected.insert(value);
    } else {
        actual.Remove(value);
        expected.erase(value);
    }
    require(actual.Size() == static_cast<int>(expected.size()), "randomized size mismatch");
    require(collect_keys(actual) == std::vector<int>(expected.begin(), expected.end()),
            "randomized contents mismatch");
}
```

</details>

<details>
<summary>5. MultiSet·MultiMap 중복 테스트</summary>

```cpp
MyOrderedMultiSet<int> values;
for (int value : {2, 1, 2, 3, 2}) values.Insert(value);
require(values.Size() == 5, "multiset rejected duplicate keys");
require(collect_keys(values) == std::vector<int>({1, 2, 2, 2, 3}),
        "multiset iteration order is wrong");
values.Remove(2);
require(values.Size() == 4, "multiset Remove did not remove exactly one element");
```

</details>

<details>
<summary>6. Map 정렬 순회 테스트</summary>

```cpp
MyOrderedMap<int, std::string> values;
values.Insert({3, "three"});
values.Insert({1, "one"});
values.Insert({2, "two"});
values.Insert({2, "duplicate"});
std::vector<int> keys;
for (auto it = values.begin(); it != values.end(); ++it) keys.push_back(it->first);
require(keys == std::vector<int>({1, 2, 3}), "map iteration order is wrong");
require(values.Size() == 3, "map accepted a duplicate key");
require(values.Find(2)->second == "two", "map duplicate replaced the original value");
```

</details>

<details>
<summary>7. 복사·이동 테스트</summary>

```cpp
MyOrderedSet<int> copy(source);
source.Remove(5);
require(copy.Size() == 5 && collect_keys(copy) == std::vector<int>({1, 3, 5, 7, 9}),
        "copy construction did not create an independent tree");

MyOrderedSet<int> moved(std::move(source));
require(source.Size() == 0 && moved.Size() == 4, "move construction size is wrong");
source.Insert(11);
require(*source.Find(11) == 11, "move-constructed source cannot be reused");
```

</details>

<details>
<summary>8. Clear 및 빈 순회 테스트</summary>

```cpp
MyOrderedSet<int> values;
require(values.begin() == values.end(), "empty begin and end differ");
values.Insert(1);
values.Insert(2);
values.Clear();
require(values.Size() == 0 && values.begin() == values.end(), "Clear did not empty the tree");
values.Insert(3);
require(values.Size() == 1 && *values.begin() == 3, "tree cannot be reused after Clear");
```

</details>

### 부하 테스트

#### 실행 방법

```powershell
# 유효성 테스트
.\DataStructure\06_MyOrderedTree\Bin\Release\MyOrderedTree.exe

# 기본 부하 테스트: benchmark [원소 수] [반복 횟수]
.\DataStructure\06_MyOrderedTree\Bin\Release\MyOrderedTree.exe benchmark 100000 15

# Ordered/Unordered 비교: compare [원소 수] [반복 횟수]
.\DataStructure\06_MyOrderedTree\Bin\Release\MyOrderedTree.exe compare 100000 15

# 기존 복합 연산 테스트: mixed [원소 수] [반복 횟수]
.\DataStructure\06_MyOrderedTree\Bin\Release\MyOrderedTree.exe mixed 10000 15
```

#### 측정 조건

##### 1. 기본 부하 테스트

고정 시드로 셔플한 동일한 고유 정수 키를 `MyOrderedSet<int>`와 MSVC STL의 `std::set<int>`에 삽입하고, 전체 키를 탐색한 뒤 같은 순서로 모두 삭제합니다. 해시 프로젝트의 기본 부하 테스트와 입력·원소 수·반복 횟수·측정 연산을 동일하게 구성했습니다.

- 빌드 구성: `Release x64`
- 원소 수: 100,000개
- 반복 횟수: 컨테이너별 15회
- 입력 순서: 고정 시드 `20260925`로 셔플한 동일 배열
- 측정 연산: 전체 삽입, 전체 탐색, 전체 삭제
- 시간 단위: 마이크로초(μs)
- 통계: 단계별 전체 시간의 평균·중앙값

##### 2. Ordered/Unordered 비교

동일하게 셔플한 고유 정수 키를 `MyOrderedSet`, `MyUnorderedSet`, `std::set`, `std::unordered_set`에 삽입하고, 전체 키를 탐색한 뒤 같은 순서로 모두 삭제합니다. 네 컨테이너는 매 회차 새로 생성하며 각 단계가 끝난 뒤 크기와 탐색 성공 횟수를 검증합니다.

- 빌드 구성: `Release x64`
- 원소 수: 100,000개
- 반복 횟수: 컨테이너별 15회
- 입력 순서: 고정 시드로 셔플한 동일 배열
- 측정 연산: 전체 삽입, 전체 탐색, 전체 삭제
- 시간 단위: 마이크로초(μs)
- 통계: 단계별 전체 시간의 평균·중앙값

#### Release 측정 결과

##### 1. 기본 부하 테스트

| 컨테이너 | 통계 | 삽입 | 탐색 | 삭제 | 합계 |
| --- | --- | ---: | ---: | ---: | ---: |
| MyOrderedSet | 평균 | 24,099 | 14,632 | 20,530 | 59,261 |
| MyOrderedSet | 중앙값 | 23,869 | 14,072 | 20,149 | 58,090 |
| `std::set` | 평균 | 20,792 | 15,326 | 30,246 | 66,364 |
| `std::set` | 중앙값 | 20,331 | 15,326 | 29,393 | 65,050 |

##### 2. Ordered/Unordered 비교

| 컨테이너 | 통계 | 삽입 | 탐색 | 삭제 | 합계 |
| --- | --- | ---: | ---: | ---: | ---: |
| MyOrderedSet | 평균 | 20,699 | 13,500 | 17,611 | 51,810 |
| MyOrderedSet | 중앙값 | 20,276 | 13,455 | 17,629 | 51,360 |
| MyUnorderedSet | 평균 | 8,025 | 456 | 2,356 | 10,837 |
| MyUnorderedSet | 중앙값 | 7,921 | 427 | 2,311 | 10,659 |
| `std::set` | 평균 | 19,506 | 16,142 | 26,884 | 62,532 |
| `std::set` | 중앙값 | 19,206 | 15,991 | 26,704 | 61,901 |
| `std::unordered_set` | 평균 | 7,096 | 890 | 3,034 | 11,020 |
| `std::unordered_set` | 중앙값 | 6,950 | 875 | 2,923 | 10,748 |

#### Release 측정 결과 분석

<details>
<summary><strong>0. STL과의 벤치마크 결과</strong></summary>

100,000개의 고유 정수를 삽입·탐색·삭제한 결과, `std::set`은 삽입에서 약 1.17배 빨랐고 탐색은 유사했습니다. `MyOrderedSet`은 삭제에서 약 1.46배 빨랐으며, 전체 중앙값은 58,090μs와 65,050μs로 약 10.7% 낮았습니다. 두 컨테이너에 동일한 작업을 적용했으므로 단계별 구현 특성을 직접 비교할 수 있습니다.

</details>

<details>
<summary><strong>1. Ordered/Unordered 벤치마크 결과</strong></summary>

100,000개의 고유 정수를 삽입·탐색·삭제한 결과, Unordered 컨테이너가 모든 측정 단계에서 Ordered 컨테이너보다 빠르게 측정됐습니다.

노드를 통합한 개선 버전의 MyUnorderedSet은 MyOrderedSet보다 삽입에서 약 2.56배, 탐색에서 약 31.51배, 삭제에서 약 7.63배 빨랐습니다. 전체 중앙값은 10,659μs와 51,360μs로 MyUnorderedSet이 약 4.82배 빨랐습니다.

`std::unordered_set`은 `std::set`보다 삽입에서 약 2.76배, 탐색에서 약 18.28배, 삭제에서 약 9.14배 빨랐습니다. 전체 중앙값은 10,748μs와 61,901μs로 `std::unordered_set`이 약 5.76배 빨랐습니다.

이번 실행에서 MyUnorderedSet과 `std::unordered_set`의 전체 중앙값은 각각 10,659μs와 10,748μs로 유사했습니다. MyUnorderedSet은 삽입에서는 STL보다 약 14.0% 느렸지만 탐색과 삭제는 각각 약 2.05배, 1.26배 빠르게 측정됐습니다. 이는 연속 정수 키와 충돌이 적은 입력을 사용한 결과이며, 일반적인 모든 키 분포에서 STL보다 우수하다는 의미는 아닙니다.

Hash Table은 평균적으로 삽입·탐색·삭제를 $O(1)$에 처리하므로, 각 연산에 $O(\log N)$이 필요한 Red-Black Tree보다 이번 키 기반 작업에서 유리했습니다. 반면 Red-Black Tree는 키의 정렬 상태를 유지하고 최악의 경우에도 $O(\log N)$의 연산 복잡도를 보장합니다. 따라서 정렬 순회와 범위 기반 처리가 필요하지 않은 단순 키 조회에는 Unordered 컨테이너가 적합하고, 정렬된 데이터와 안정적인 최악 시간 복잡도가 필요하면 Ordered 컨테이너가 적합합니다.

</details>

## 참고 문헌

- [std::set - cppreference](https://en.cppreference.com/w/cpp/container/set)
- [std::map - cppreference](https://en.cppreference.com/w/cpp/container/map)
