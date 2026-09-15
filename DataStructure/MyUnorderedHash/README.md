# MyUnorderedHash

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

`MyHashTable`은 STL의 unordered 컨테이너를 참고해 구현한 해시 테이블입니다. 공통 테이블 위에 `Set`, `Map`, `MultiSet`, `MultiMap`을 구성했습니다.

### 구현 중심점

- 버킷 배열과 충돌 처리용 연결 노드
- 원소 저장 및 순회를 위한 전역 연결 리스트
- 자동 재해시와 `Reserve`, `Rehash`
- 중복 키 허용 여부를 구분하는 네 어댑터

## 포함 기능

| 구분 | 메서드 |
| --- | --- |
| 공통 테이블 | `Insert`, `Find`, `Remove`, `Reserve`, `Rehash`, `Size` |
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

```text
9 passed, 0 failed
```

<details>
<summary>1. Set 중복 키·충돌 테스트 코드</summary>

```cpp
void test_set_unique_and_collision() {
    MyUnorderedSet<int> values;
    values.Insert(1);
    values.Insert(9); // Both keys initially map to the same bucket.
    values.Insert(1);
    require(values.Size() == 2, "set accepted a duplicate key");
    require(values.Find(1) != nullptr && values.Find(9) != nullptr, "set lookup failed");
    values.Remove(1);
    require(values.Find(1) == nullptr && values.Find(9) != nullptr, "collision removal failed");
    require(values.Size() == 1, "set size after removal is wrong");
}
```

</details>

<details>
<summary>2. MultiSet 중복 삭제 테스트 코드</summary>

```cpp
void test_multiset_duplicates() {
    MyUnorderedMultiSet<int> values;
    values.Insert(5);
    values.Insert(5);
    values.Insert(13);
    require(values.Size() == 3, "multiset rejected a duplicate");
    values.Remove(5);
    require(values.Size() == 1 && values.Find(5) == nullptr, "multiset did not remove all matches");
    require(values.Find(13) != nullptr, "multiset removed a colliding key");
}
```

</details>

<details>
<summary>3. Map 값 접근 테스트 코드</summary>

```cpp
void test_map_access() {
    MyUnorderedMap<int, std::string> values;
    values.Insert({2, "two"});
    values.Insert({2, "duplicate"});
    require(values.Size() == 1 && values.at(2) == "two", "map duplicate changed the value");
    values[3] = "three";
    require(values.Size() == 2 && values.at(3) == "three", "map subscript failed");
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
    values.Insert({4, "first"});
    values.Insert({4, "second"});
    values.Insert({12, "collision"});
    require(values.Size() == 3 && values.Find(4) != nullptr, "multimap insertion failed");
    values.Remove(4);
    require(values.Size() == 1 && values.Find(4) == nullptr, "multimap did not remove all matches");
    require(values.Find(12) != nullptr, "multimap removed a colliding key");
}
```

</details>

<details>
<summary>5. 재해시·예약 테스트 코드</summary>

```cpp
void test_rehash_and_reserve() {
    MyUnorderedSet<int> values;
    for (int key = 0; key < 128; ++key) values.Insert(key);
    values.Reserve(512);
    values.Rehash(1024);
    require(values.Size() == 128, "rehash changed the element count");
    for (int key = 0; key < 128; ++key)
        require(values.Find(key) != nullptr, "rehash lost a key");
}
```

</details>

<details>
<summary>6. 원소 순회 테스트 코드</summary>

```cpp
void test_iteration() {
    MyUnorderedSet<int> values;
    values.Insert(1);
    values.Insert(9);
    values.Insert(17);
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
    source.Insert({1, "one"});
    source.Insert({9, "nine"});
    MyUnorderedMap<int, std::string> copy(source);
    source.Remove(1);
    require(copy.Size() == 2 && copy.at(1) == "one", "copy shares elements with source");

    MyUnorderedMap<int, std::string> assigned;
    assigned = copy;
    assigned = assigned;
    require(assigned.Size() == 2 && assigned.at(9) == "nine", "copy assignment failed");
}
```

</details>

<details>
<summary>8. 이동 및 원본 재사용 테스트 코드</summary>

```cpp
void test_move_and_reuse() {
    MyUnorderedSet<int> source;
    source.Insert(1);
    source.Insert(9);
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.Size() == 2 && moved.Find(9) != nullptr, "move construction lost elements");
    require(source.Size() == 0, "move-constructed source is not empty");
    source.Insert(17);
    require(source.Find(17) != nullptr, "move-constructed source cannot be reused");

    MyUnorderedSet<int> assigned;
    assigned.Insert(100);
    assigned = std::move(moved);
    require(assigned.Size() == 2 && assigned.Find(1) != nullptr, "move assignment lost elements");
    require(moved.Size() == 0, "move-assigned source is not empty");
    assigned = std::move(assigned);
    require(assigned.Size() == 2, "self move assignment changed the object");
    moved.Insert(25);
    require(moved.Find(25) != nullptr, "move-assigned source cannot be reused");
}
```

</details>

<details>
<summary>9. 빈 테이블 이동 테스트 코드</summary>

```cpp
void test_empty_move() {
    MyUnorderedSet<int> source;
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.Size() == 0 && !(moved.begin() != moved.end()), "empty move construction failed");
    moved.Insert(1);
    source.Insert(2);
    require(moved.Find(1) != nullptr && source.Find(2) != nullptr, "empty moved objects cannot be reused");

    MyUnorderedSet<int> empty;
    moved = std::move(empty);
    require(moved.Size() == 0 && !(moved.begin() != moved.end()), "empty move assignment failed");
    moved.Insert(3);
    empty.Insert(4);
    require(moved.Find(3) != nullptr && empty.Find(4) != nullptr, "empty move assignment broke reuse");
}
```

</details>

### 부하 테스트

#### 측정 조건

로드 팩터별 **조회 성능**을 `MyUnorderedSet<int>`와 MSVC STL의 `std::unordered_set<int>`로 비교했습니다. 원소 8,192개를 미리 삽입하고, 각 측정에서 존재하는 키 8,192개와 없는 키 8,192개를 조회합니다. 삽입·테이블 생성·버킷 준비 시간은 측정에 포함하지 않습니다.

현재 `MyHashTable`에는 최대 로드 팩터 변경 API가 없으므로, 자동 재해시를 끄고 `Rehash`로 버킷 수를 미리 설정해 **실제 원소 수 / 버킷 수**를 조절했습니다. STL도 `max_load_factor`와 `rehash`로 같은 목표를 설정했습니다. 아래 네 조건은 실제 로드 팩터와 버킷 수가 두 컨테이너에서 동일했습니다. 로드 팩터 2.0은 사용자 지정 컨테이너의 기본 자동 재해시 기준 1.0을 변경한 것이 아니라, 자동 재해시를 끈 측정 조건입니다.

- 빌드 구성: `Release x64`
- 원소 타입과 수: `int` 8,192개
- 측정 연산: 존재하는 키 8,192개와 없는 키 8,192개 조회
- 측정 횟수: 로드 팩터·컨테이너별 15회
- 시간 단위: 마이크로초(μs)
- 통계: 각 회차의 조회 전체 시간에 대한 평균·중앙값

#### Release 측정 결과

아래 수치는 **한 번의 실행 결과**입니다.

| 실제 로드 팩터 | 버킷 수 | MyUnorderedSet 평균 / 중앙값 | std::unordered_set 평균 / 중앙값 |
| ---: | ---: | ---: | ---: |
| 0.25 | 32,768 | 25 / 24 μs | 45 / 43 μs |
| 0.5 | 16,384 | 26 / 25 μs | 54 / 51 μs |
| 1.0 | 8,192 | 31 / 31 μs | 46 / 46 μs |
| 2.0 | 4,096 | 42 / 42 μs | 75 / 51 μs |

#### 해석

이번 실행에서는 두 구현 모두 로드 팩터 2.0에서 조회 시간이 늘었습니다. `MyUnorderedSet`이 모든 조건에서 더 짧게 측정됐지만, 이 결과는 **정수 키의 조회 연산만** 비교한 것으로 삽입·삭제나 전체 컨테이너 성능의 우위를 뜻하지 않습니다. 측정값은 실행 환경에 따라 달라질 수 있습니다.

#### 실행 방법

```powershell
& '.\DataStructure\MyUnorderedHash\Bin\Release\MyUnorderedHash.exe' benchmark 8192 15
```

저장소 루트에서 실행하는 명령입니다. 인자는 순서대로 `benchmark [원소 수] [조건별 반복 횟수]`입니다. 인자를 생략하면 8,192개와 15회를 사용합니다. `benchmark` 없이 실행하면 위의 유효성 테스트 9개가 실행됩니다.
