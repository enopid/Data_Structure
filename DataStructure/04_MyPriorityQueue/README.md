# MyPriorityQueue

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

`MyPriorityQueue<T, Comp>`는 STL의 `std::priority_queue`처럼 Binary Heap을 기반으로 우선순위가 가장 높은 원소를 먼저 꺼내는 자료구조입니다.<br>
추가로 각 원소를 식별하는 핸들을 제공하여, 힙 내부를 순회하지 않고 원소의 우선순위를 변경하거나 삭제할 수 있도록 구현했습니다.

### 구현 중심점

- 배열 기반 Binary Heap
- 비교자 `Comp`를 통한 최대 힙·최소 힙 구성<br>
  (comp를 통한 커스텀 비교 힙 지원)
- 세대 번호가 포함된 핸들을 통한 원소 추적<br>
  (decreasing key 지원)
- 원소 이동 시 핸들이 가리키는 힙 인덱스 동기화
- `update`를 이용한 우선순위 증가 및 감소
- 제거되거나 재사용된 핸들의 유효성 검사

## 포함 기능

| 구분 | 메서드 |
| --- | --- |
| 생성 및 소유권 | `MyPriorityQueue`, 복사/이동 생성자, 복사/이동 대입, 소멸자 |
| 상태 조회 | `size`, `empty` |
| 우선순위 접근 | `top` |
| 원소 변경 | `push`, `pop`, `clear` |
| 핸들 연산 | `get`, `valid`, `update`, `erase` |
| 비교 정책 | 템플릿 비교자 `Comp` |

## 분석

### 유효성 테스트

별도의 테스트 프레임워크 없이 자체 테스트 러너를 사용합니다. 기본 최대 힙뿐만 아니라 사용자 비교자, 핸들 수명 및 양방향 우선순위 변경을 확인했습니다. 기본 `push`, `top`, `pop` 동작은 우선순위 순서로 검증하고, 핸들 기반 무작위 테스트는 `std::multiset`을 기준 컨테이너로 사용합니다. `std::priority_queue`는 임의 원소의 `update`와 `erase`를 지원하지 않기 때문에 해당 연산까지 동일하게 반영할 수 있는 `std::multiset`으로 크기와 최댓값을 비교했습니다. 이는 성능 비교가 아니라 연산 결과의 정합성을 검증하기 위한 구성입니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| 빈 큐 | 기본 상태와 빈 큐의 `top` 예외 | PASS |
| 최대 힙 | 중복값을 포함한 `push`, `top`, `pop` 순서 | PASS |
| 사용자 비교자 | `std::greater`를 이용한 최소 힙 순서 | PASS |
| 핸들 접근 | `valid`, `get`과 원소 추적 | PASS |
| 우선순위 변경 | 값 증가·감소 후 상향·하향 이동 | PASS |
| 삭제와 핸들 | `erase`, `pop` 이후 핸들 무효화 | PASS |
| 초기화와 소유권 | `clear`, 슬롯 재사용, 복사 및 이동 | PASS |
| 무작위 차등 비교 | 30,000회 `push`, `update`, `erase`, `pop`, `clear`와 핸들을 `std::multiset`과 비교 | PASS |

```text
MyPriorityQueue validity tests (Release)

[PASS 1/8] empty queue and top exception
[PASS 2/8] max-heap push/pop order
[PASS 3/8] custom comparator
[PASS 4/8] handle validity and access
[PASS 5/8] priority update in both directions
[PASS 6/8] erase/pop handle invalidation
[PASS 7/8] clear, copy, and move
[PASS 8/8] 30,000 random handle operations against std::multiset

8 passed, 0 failed
```

<details>
<summary>1. 빈 큐 테스트 코드</summary>

```cpp
void test_empty_queue() {
    MyPriorityQueue<int> queue;
    require(queue.empty(), "new queue must be empty");
    require(queue.size() == 0, "new queue size must be zero");

    bool threw = false;
    try { (void)queue.top(); }
    catch (const std::out_of_range&) { threw = true; }
    require(threw, "top() must reject an empty queue");
}
```

</details>

<details>
<summary>2. 최대 힙 순서 테스트 코드</summary>

```cpp
void test_max_heap_order() {
    MyPriorityQueue<int> queue;
    const std::vector<int> input{4, 1, 9, 3, 9, -2, 7};
    const std::vector<int> expected{9, 9, 7, 4, 3, 1, -2};

    for (int value : input) queue.push(value);
    for (int value : expected) {
        require(queue.top() == value, "max-heap pop order mismatch");
        queue.pop();
    }
    require(queue.empty(), "queue must be empty after every element is popped");
}
```

</details>

<details>
<summary>3. 사용자 비교자 테스트 코드</summary>

```cpp
void test_custom_comparator() {
    MyPriorityQueue<int, std::greater<int>> queue;
    for (int value : {4, 1, 9, 3}) queue.push(value);

    for (int expected : {1, 3, 4, 9}) {
        require(queue.top() == expected, "custom comparator order mismatch");
        queue.pop();
    }
}
```

</details>

<details>
<summary>4. 핸들 접근 테스트 코드</summary>

```cpp
void test_handle_access() {
    MyPriorityQueue<int> queue;
    const auto first = queue.push(10);
    const auto second = queue.push(30);

    require(queue.valid(first), "first handle must be valid");
    require(queue.valid(second), "second handle must be valid");
    require(queue.get(first) == 10, "first handle points to the wrong value");
    require(queue.get(second) == 30, "second handle points to the wrong value");
}
```

</details>

<details>
<summary>5. 우선순위 변경 테스트 코드</summary>

```cpp
void test_update() {
    MyPriorityQueue<int> queue;
    const auto first = queue.push(10);
    const auto second = queue.push(20);
    const auto third = queue.push(30);

    queue.update(first, 40);
    require(queue.top() == 40, "increased value did not move upward");
    queue.update(first, 5);
    require(queue.top() == 30, "decreased value did not move downward");
    require(queue.get(second) == 20 && queue.get(third) == 30,
            "update changed another handle's value");
}
```

</details>

<details>
<summary>6. 삭제 및 핸들 무효화 테스트 코드</summary>

```cpp
void test_erase_and_pop_invalidate_handles() {
    MyPriorityQueue<int> queue;
    const auto low = queue.push(10);
    const auto middle = queue.push(20);
    const auto high = queue.push(30);

    queue.erase(middle);
    require(!queue.valid(middle), "erased handle must be invalid");
    require(queue.top() == 30, "erase broke heap order");
    queue.pop();
    require(!queue.valid(high), "popped handle must be invalid");
    require(queue.valid(low) && queue.top() == 10,
            "remaining handle or value is invalid");
}
```

</details>

<details>
<summary>7. 초기화·복사·이동 테스트 코드</summary>

```cpp
void test_clear_copy_and_move() {
    MyPriorityQueue<int> original;
    const auto old_handle = original.push(10);
    original.push(40);

    MyPriorityQueue<int> copied(original);
    MyPriorityQueue<int> moved(std::move(copied));
    require(moved.size() == 2 && moved.top() == 40, "move construction failed");

    original.clear();
    require(original.empty(), "clear did not empty the queue");
    require(!original.valid(old_handle), "clear did not invalidate an old handle");
    const auto new_handle = original.push(25);
    require(original.valid(new_handle), "queue cannot be reused after clear");
    require(!original.valid(old_handle), "reused slot revived a stale handle");
}
```

</details>

<details>
<summary>8. std::multiset과 30,000회 무작위 핸들 연산 비교</summary>

`std::priority_queue`로 직접 비교할 수 있는 연산은 `push`, `top`, `pop`으로 제한됩니다. `MyPriorityQueue`가 추가로 제공하는 `update(handle, value)`와 `erase(handle)`를 포함해 검증하기 위해, 값의 삽입·임의 삭제·최댓값 조회가 가능한 `std::multiset`을 기준으로 사용했습니다. 업데이트는 기존 값을 삭제한 뒤 변경된 값을 삽입하는 방식으로 동일한 상태를 만들며, 매 연산 후 크기와 최댓값을 비교합니다. 핸들의 유효성과 연결된 값은 별도의 활성 핸들 목록으로 확인합니다.

```cpp
for (int step = 0; step < 30000; ++step) {
    // push, update, erase, pop, clear 중 하나를 수행하고
    // 동일 변경을 std::multiset과 활성 핸들 목록에 반영합니다.
    require(actual.size() == static_cast<int>(expected.size()), "randomized size mismatch");
    require(actual.empty() == expected.empty(), "randomized empty-state mismatch");
    if (!expected.empty())
        require(actual.top() == *expected.rbegin(), "randomized top mismatch");

    if (step % 100 == 0) {
        for (const Entry& entry : active) {
            require(actual.valid(entry.handle), "live handle became invalid");
            require(actual.get(entry.handle) == entry.value, "handle value mismatch");
        }
    }
}
```

</details>

### 부하 테스트

동일한 입력에 대해 `MyPriorityQueue`와 `std::priority_queue`의 삽입 및 제거 성능을 비교합니다.

```powershell
MyPriorityQueue.exe --benchmark 100000 30
```

첫 번째 숫자는 각 큐에 삽입할 원소 수이고, 두 번째 숫자는 컨테이너별 반복 측정 횟수입니다. 생략하면 각각 100,000개와 10회를 사용합니다.

#### 측정 조건

- 빌드 구성: `Release x64`
- 비교 대상: `MyPriorityQueue`, `std::priority_queue`
- 측정 연산: 전체 원소 `push` 후 우선순위 순서로 전체 `pop`
- 입력: 고정 시드로 생성한 0~1,000,000 범위의 정수 100,000개
- 측정 횟수: 각 컨테이너 30회
- 시간 단위: 마이크로초(μs)
- 통계: 평균값, 중앙값, 최솟값, 최댓값
- 입력 생성 시간: 측정에서 제외
- 결과 검증: 모든 제거 원소의 합계가 같은지 checksum으로 확인

#### Release 측정 결과

| 컨테이너 | 평균 | 중앙값 | 최소 | 최대 |
| --- | ---: | ---: | ---: | ---: |
| MyPriorityQueue | 12,237μs | 12,238μs | 11,170μs | 13,894μs |
| `std::priority_queue` | 4,887μs | 4,880μs | 4,766μs | 5,038μs |

두 구현의 checksum은 `49,787,350,884`로 동일했습니다.

#### Release 측정 결과 분석

<details>
<summary><strong>0. STL과의 벤치마크 결과</strong></summary>

이번 조건에서 `std::priority_queue`의 중앙값은 MyPriorityQueue보다 약 2.51배 빨랐습니다.

MyPriorityQueue는 `std::priority_queue`보다 성능이 낮지만, 이는 각 원소의 핸들을 관리하고 노드 교환 시 힙 인덱스를 함께 갱신하는 비용 때문입니다. 

대신 `std::priority_queue`가 지원하지 않는 decrease-key와 임의 원소 갱신·삭제를 핸들을 통해 지원합니다.

</details>

## 참고 문헌

- [std::priority_queue - cppreference](https://en.cppreference.com/w/cpp/container/priority_queue.html)
- [Boost.Heap Data Structures](https://www.boost.org/doc/libs/latest/doc/html/heap/data_structures.html)
- [boost::heap::d_ary_heap - mutable handle, update, decrease, erase](https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1heap_1_1d__ary__heap.html)
