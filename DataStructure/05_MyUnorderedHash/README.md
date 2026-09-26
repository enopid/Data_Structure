# MyUnorderedHash

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

`MyHashTable`은 STL의 unordered 컨테이너를 참고해 구현한 해시 테이블입니다. <br>
공통 테이블을 상속받아 `Set`, `Map`, `MultiSet`, `MultiMap`을 구성했습니다.

### 구현 중심점

- 버킷 배열과 충돌 처리용 연결 노드<br>
  (버킷 별로 리스트 구조를 지닌 stl의 해시테이블 구조 구현)
- 원소 저장 및 순회를 위한 전역 연결 리스트<br>
  (순회를 위한 전역 연결리스트와 버킷내 리스트를 따로 관리하지 않고, 전역 연결 리스트의 노드가 버킷 내 리스트의 역할까지 수행)
- 자동 재해시와 `Reserve`, `Rehash`<br>
  (벡터와 유사하게 지수적 성장을 하는 버킷 배열과 이를 지원하는 함수 사용)
- 중복 키 허용 여부를 구분하는 네 어댑터<br>
  (해시테이블을 상속받아 map/set, 중복허용 여부에따른 4가지 클래스 구현)

## 포함 기능

| 구분 | 메서드 |
| --- | --- |
| 공통 테이블 | `insert`, `find`, `remove`, `reserve`, `rehash`, `size` |
| 순회·소유권 | `begin`, `end`, 복사·이동 생성 및 대입 |
| `MyUnorderedMap` | `at`, `operator[]` |
| 중복 키 | `MyUnorderedMultiSet`, `MyUnorderedMultiMap` |

템플릿 구현은 선언(`.h`)과 정의(`.inl`)로 분리했고, 전역 원소 리스트는 이 저장소의 `MyLinkedList`를 사용합니다.

## 분석

### 유효성 테스트

네 어댑터의 기본 동작, 충돌, 재해시, 복사 및 이동 후 재사용을 자체 테스트 러너로 확인했습니다. 아래는 **Release x64 실제 실행 결과**입니다. 각 항목을 펼치면 실행한 테스트 함수가 나옵니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| Set | 중복 거부, 충돌 키 조회·삭제 | PASS |
| MultiSet | 중복 삽입·동일 키 전체 삭제 | PASS |
| Map | 중복 키, 값 접근, 없는 키 예외 | PASS |
| MultiMap | 중복 삽입·동일 키 전체 삭제 | PASS |
| 재해시 | 자동 증가, 예약·재해시 후 원소 보존 | PASS |
| 순회 | 삽입 원소 전체 방문 | PASS |
| 복사 | 복사 생성·대입·자기 대입 | PASS |
| 이동 | 이동 생성·대입·자기 대입·원본 재사용 | PASS |
| 빈 원본 이동 | 빈 테이블 이동과 양쪽 재사용 | PASS |
| Multi 버킷 전체 삭제 | 동일 키 전체 삭제 후 빈 버킷과 빈 순회 확인 | PASS |
| 무작위 차등 비교 | 50,000회 삽입·탐색·삭제·예약을 `std::unordered_set`과 비교 | PASS |

```text
11 passed, 0 failed
```

<details>
<summary>1. Set 중복 키·충돌 테스트 코드</summary>

```cpp
void test_set_unique_and_collision() {
    MyUnorderedSet<int> values;
    values.insert(1);
    values.insert(9); // Both keys initially map to the same bucket.
    values.insert(1);
    require(values.size() == 2, "set accepted a duplicate key");
    require(values.find(1) != nullptr && values.find(9) != nullptr, "set lookup failed");
    values.remove(1);
    require(values.find(1) == nullptr && values.find(9) != nullptr, "collision removal failed");
    require(values.size() == 1, "set size after removal is wrong");
}
```

</details>

<details>
<summary>2. MultiSet 중복 삭제 테스트 코드</summary>

```cpp
void test_multiset_duplicates() {
    MyUnorderedMultiSet<int> values;
    values.insert(5);
    values.insert(5);
    values.insert(13);
    require(values.size() == 3, "multiset rejected a duplicate");
    values.remove(5);
    require(values.size() == 1 && values.find(5) == nullptr, "multiset did not remove all matches");
    require(values.find(13) != nullptr, "multiset removed a colliding key");
}
```

</details>

<details>
<summary>3. Map 값 접근 테스트 코드</summary>

```cpp
void test_map_access() {
    MyUnorderedMap<int, std::string> values;
    values.insert({2, "two"});
    values.insert({2, "duplicate"});
    require(values.size() == 1 && values.at(2) == "two", "map duplicate changed the value");
    values[3] = "three";
    require(values.size() == 2 && values.at(3) == "three", "map subscript failed");
    bool threw = false;
    try { values.at(99); } catch (const std::runtime_error&) { threw = true; }
    require(threw, "map at did not reject a missing key");
}
```

</details>

<details>
<summary>4. MultiMap 중복 삭제 테스트 코드</summary>

```cpp
void test_multimap_duplicates() {
    MyUnorderedMultiMap<int, std::string> values;
    values.insert({4, "first"});
    values.insert({4, "second"});
    values.insert({12, "collision"});
    require(values.size() == 3 && values.find(4) != nullptr, "multimap insertion failed");
    values.remove(4);
    require(values.size() == 1 && values.find(4) == nullptr, "multimap did not remove all matches");
    require(values.find(12) != nullptr, "multimap removed a colliding key");
}
```

</details>

<details>
<summary>5. 재해시·예약 테스트 코드</summary>

```cpp
void test_rehash_and_reserve() {
    MyUnorderedSet<int> values;
    for (int key = 0; key < 128; ++key) values.insert(key);
    values.reserve(512);
    values.rehash(1024);
    require(values.size() == 128, "rehash changed the element count");
    for (int key = 0; key < 128; ++key)
        require(values.find(key) != nullptr, "rehash lost a key");
}
```

</details>

<details>
<summary>6. 원소 순회 테스트 코드</summary>

```cpp
void test_iteration() {
    MyUnorderedSet<int> values;
    values.insert(1);
    values.insert(9);
    values.insert(17);
    int count = 0;
    int sum = 0;
    for (auto it = values.begin(); it != values.end(); ++it) {
        ++count;
        sum += *it;
    }
    require(count == 3 && sum == 27, "iteration did not visit every element");
}
```

</details>

<details>
<summary>7. 복사 테스트 코드</summary>

```cpp
void test_copy() {
    MyUnorderedMap<int, std::string> source;
    source.insert({1, "one"});
    source.insert({9, "nine"});
    MyUnorderedMap<int, std::string> copy(source);
    source.remove(1);
    require(copy.size() == 2 && copy.at(1) == "one", "copy shares elements with source");

    MyUnorderedMap<int, std::string> assigned;
    assigned = copy;
    assigned = assigned;
    require(assigned.size() == 2 && assigned.at(9) == "nine", "copy assignment failed");
}
```

</details>

<details>
<summary>8. 이동 및 원본 재사용 테스트 코드</summary>

```cpp
void test_move_and_reuse() {
    MyUnorderedSet<int> source;
    source.insert(1);
    source.insert(9);
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.size() == 2 && moved.find(9) != nullptr, "move construction lost elements");
    require(source.size() == 0, "move-constructed source is not empty");
    source.insert(17);
    require(source.find(17) != nullptr, "move-constructed source cannot be reused");

    MyUnorderedSet<int> assigned;
    assigned.insert(100);
    assigned = std::move(moved);
    require(assigned.size() == 2 && assigned.find(1) != nullptr, "move assignment lost elements");
    require(moved.size() == 0, "move-assigned source is not empty");
    assigned = std::move(assigned);
    require(assigned.size() == 2, "self move assignment changed the object");
    moved.insert(25);
    require(moved.find(25) != nullptr, "move-assigned source cannot be reused");
}
```

</details>

<details>
<summary>9. 빈 테이블 이동 테스트 코드</summary>

```cpp
void test_empty_move() {
    MyUnorderedSet<int> source;
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.size() == 0 && !(moved.begin() != moved.end()), "empty move construction failed");
    moved.insert(1);
    source.insert(2);
    require(moved.find(1) != nullptr && source.find(2) != nullptr, "empty moved objects cannot be reused");

    MyUnorderedSet<int> empty;
    moved = std::move(empty);
    require(moved.size() == 0 && !(moved.begin() != moved.end()), "empty move assignment failed");
    moved.insert(3);
    empty.insert(4);
    require(moved.find(3) != nullptr && empty.find(4) != nullptr, "empty move assignment broke reuse");
}
```

</details>
<details>
<summary>10. MultiSet 버킷 전체 삭제 테스트 코드</summary>

```cpp
void test_multiset_bucket_becomes_empty() {
    MyUnorderedMultiSet<int> values;
    values.insert(5);
    values.insert(5);
    values.remove(5);
    require(values.size() == 0, "removing every duplicate did not empty the table");
    require(values.find(5) == nullptr, "removed duplicate key is still present");
    require(!(values.begin() != values.end()), "empty multiset iteration is invalid");
}
```

</details>

<details>
<summary>11. std::unordered_set과 50,000회 무작위 연산 비교</summary>

```cpp
for (int step = 0; step < 50000; ++step) {
    const int base = static_cast<int>(random() % 4096);
    const int key = base + static_cast<int>(random() % 16) * 65536;
    // 의도적으로 충돌하는 키로 insert, remove, find, reserve를 수행합니다.

    require(actual.size() == static_cast<int>(expected.size()), "randomized size mismatch");
    if (step % 100 == 0) {
        std::unordered_set<int> observed;
        for (auto it = actual.begin(); it != actual.end(); ++it) observed.insert(*it);
        require(observed == expected, "randomized contents mismatch");
    }
}
```

</details>

### 부하 테스트

#### 실행 방법

```powershell
# 기본 부하 테스트: benchmark [원소 수] [반복 횟수]
.\DataStructure\05_MyUnorderedHash\Bin\Release\MyUnorderedHash.exe benchmark 100000 15

# Load Factor별 조회 테스트: load-factor [원소 수] [반복 횟수]
.\DataStructure\05_MyUnorderedHash\Bin\Release\MyUnorderedHash.exe load-factor 8192 15
```

저장소 루트에서 실행하는 명령입니다. `benchmark`의 기본값은 100,000개와 15회이며, `load-factor`의 기본값은 8,192개와 15회입니다. 실행 모드를 생략하면 위의 유효성 테스트 9개가 실행됩니다.

#### 측정 조건

##### 1. 기본 부하 테스트

고정 시드로 셔플한 동일한 고유 정수 키를 `MyUnorderedSet<int>`와 MSVC STL의 `std::unordered_set<int>`에 삽입하고, 전체 키를 탐색한 뒤 같은 순서로 모두 삭제합니다. RBT 프로젝트의 기본 부하 테스트와 입력·원소 수·반복 횟수·측정 연산을 동일하게 구성했습니다.

- 빌드 구성: `Release x64`
- 원소 수: 100,000개
- 반복 횟수: 컨테이너별 15회
- 입력 순서: 고정 시드 `20260925`로 셔플한 동일 배열
- 측정 연산: 전체 삽입, 전체 탐색, 전체 삭제
- 시간 단위: 마이크로초(μs)
- 통계: 단계별 전체 시간의 평균·중앙값

##### 2. Load Factor별 조회 테스트

로드 팩터별 **조회 성능**을 `MyUnorderedSet<int>`와 MSVC STL의 `std::unordered_set<int>`로 비교했습니다. 원소 8,192개를 미리 삽입하고, 각 측정에서 존재하는 키 8,192개와 없는 키 8,192개를 조회합니다. 삽입·테이블 생성·버킷 준비 시간은 측정에 포함하지 않습니다.

현재 `MyHashTable`에는 최대 로드 팩터 변경 API가 없으므로, 자동 재해시를 끄고 `rehash`로 버킷 수를 미리 설정해 **실제 원소 수 / 버킷 수**를 조절했습니다. STL도 `max_load_factor`와 `rehash`로 같은 목표를 설정했습니다. 아래 네 조건은 실제 로드 팩터와 버킷 수가 두 컨테이너에서 동일했습니다. 로드 팩터 2.0은 사용자 지정 컨테이너의 기본 자동 재해시 기준 1.0을 변경한 것이 아니라, 자동 재해시를 끈 측정 조건입니다.

- 빌드 구성: `Release x64`
- 원소 타입과 수: `int` 8,192개
- 측정 연산: 존재하는 키 8,192개와 없는 키 8,192개 조회
- 측정 횟수: 로드 팩터·컨테이너별 15회
- 시간 단위: 마이크로초(μs)
- 통계: 각 회차의 조회 전체 시간에 대한 평균·중앙값

#### Release 측정 결과

##### 1. 기본 부하 테스트

| 컨테이너 | 통계 | 삽입 | 탐색 | 삭제 | 합계 |
| --- | --- | ---: | ---: | ---: | ---: |
| MyUnorderedSet | 평균 | 9,651 | 573 | 3,238 | 13,462 |
| MyUnorderedSet | 중앙값 | 8,401 | 439 | 2,784 | 11,624 |
| `std::unordered_set` | 평균 | 6,985 | 902 | 3,100 | 10,987 |
| `std::unordered_set` | 중앙값 | 6,814 | 878 | 2,986 | 10,678 |

##### 2. Load Factor별 조회 테스트

| 실제 로드 팩터 | 버킷 수 | MyUnorderedSet 평균 / 중앙값 | std::unordered_set 평균 / 중앙값 |
| ---: | ---: | ---: | ---: |
| 0.25 | 32,768 | 24 / 23 μs | 40 / 40 μs |
| 0.5 | 16,384 | 24 / 24 μs | 52 / 49 μs |
| 1.0 | 8,192 | 30 / 30 μs | 45 / 45 μs |
| 2.0 | 4,096 | 43 / 43 μs | 54 / 48 μs |

#### Release 측정 결과 분석

<details>
<summary><strong>0. STL과의 벤치마크 결과</strong></summary>

100,000개의 고유 정수를 삽입·탐색·삭제한 결과, 중앙값 기준으로 커스텀 구현은 STL보다 삽입에서 약 23.3% 느렸고 삭제에서 약 6.8% 빨랐습니다. 탐색에서는 커스텀 구현이 약 2배 빠르게 측정됐습니다.

커스텀 구현은 노드 포인터와 `nullptr`를 직접 비교하지만 STL은 iterator 반환 규약을 사용하므로 측정하는 API 비용이 완전히 같지는 않습니다. 다만 이 차이만으로 약 2배의 결과를 모두 설명하기는 어려우며, 조회당 절대 시간 차이가 수 ns 수준이라는 점도 함께 고려해야 합니다.

두 구현 모두 탐색이 삽입·삭제보다 빠르게 측정됐습니다. 탐색에는 노드의 동적 할당·해제와 버킷 또는 연결 구조의 변경이 포함되지 않기 때문입니다.

삽입이 삭제보다 오래 걸린 결과에는 노드 할당뿐만 아니라 버킷 배열을 확장하고 기존 원소를 재배치하는 재해시 비용이 삽입 구간에 포함된 점이 영향을 줍니다.

</details>

<details>
<summary><strong>1. Load Factor에 따른 조회 성능</strong></summary>

MyUnorderedSet의 중앙값은 로드 팩터 0.25에서 23μs, 0.5에서 24μs, 1.0에서 30μs, 2.0에서 43μs로 증가했습니다. `std::unordered_set`은 같은 조건에서 각각 40μs, 49μs, 45μs, 48μs였습니다.

로드 팩터가 높아지면 버킷당 평균 원소 수가 증가하여, 충돌이 발생한 버킷에서 확인해야 하는 노드 수도 증가할 수 있습니다.

커스텀 구현에서는 로드 팩터가 높아질수록 탐색 시간이 증가하는 경향이 명확하게 나타났습니다. STL 결과는 40μs, 49μs, 45μs, 48μs로 단조롭게 증가하지 않았으므로, 이번 측정만으로 STL의 로드 팩터와 탐색 시간 사이에서 동일한 증가 경향을 확인하기는 어렵습니다.

</details>
