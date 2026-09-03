# MyPriorityQueue

## AI 활용 범위

> `Main.cpp`의 유효성 테스트와 벤치마크 코드에는 AI를 적극적으로 활용했습니다. 생성된 코드는 직접 검토하고 Release 환경에서 실행하여 결과를 확인합니다.

- 테스트 항목 설계, 자체 테스트 러너 및 벤치마크 코드 작성: AI 활용
- `MyPriorityQueue`의 힙, 핸들 및 decrease-key 핵심 로직: 직접 설계 및 구현
- AI 생성 코드의 역할: 직접 구현한 자료구조의 유효성 검증 및 성능 측정

## 개요

`MyPriorityQueue<T, Comp>`는 STL의 `std::priority_queue`처럼 Binary Heap을 기반으로 우선순위가 가장 높은 원소를 먼저 꺼내는 자료구조입니다.<br>
추가로 각 원소를 식별하는 핸들을 제공하여, 힙 내부를 순회하지 않고 원소의 우선순위를 변경하거나 삭제할 수 있도록 구현했습니다.

### 구현 중심점

- 배열 기반 Binary Heap
- 비교자 `Comp`를 통한 최대 힙·최소 힙 구성
- 세대 번호가 포함된 핸들을 통한 원소 추적
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

별도의 테스트 프레임워크 없이 자체 테스트 러너를 사용합니다. 기본 최대 힙뿐만 아니라 사용자 비교자, 핸들 수명 및 양방향 우선순위 변경을 확인했습니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| 빈 큐 | 기본 상태와 빈 큐의 `top` 예외 | PASS |
| 최대 힙 | 중복값을 포함한 `push`, `top`, `pop` 순서 | PASS |
| 사용자 비교자 | `std::greater`를 이용한 최소 힙 순서 | PASS |
| 핸들 접근 | `valid`, `get`과 원소 추적 | PASS |
| 우선순위 변경 | 값 증가·감소 후 상향·하향 이동 | PASS |
| 삭제와 핸들 | `erase`, `pop` 이후 핸들 무효화 | PASS |
| 초기화와 소유권 | `clear`, 슬롯 재사용, 복사 및 이동 | PASS |

```text
MyPriorityQueue validity tests (Release)

[PASS 1/7] empty queue and top exception
[PASS 2/7] max-heap push/pop order
[PASS 3/7] custom comparator
[PASS 4/7] handle validity and access
[PASS 5/7] priority update in both directions
[PASS 6/7] erase/pop handle invalidation
[PASS 7/7] clear, copy, and move

7 passed, 0 failed
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

### 성능 테스트 1: STL Priority Queue 부하 비교

동일한 입력에 대해 `MyPriorityQueue`와 `std::priority_queue`의 삽입 및 제거 성능을 비교합니다.

#### 측정 조건

- 빌드 구성: `Release x64`
- 비교 대상: `MyPriorityQueue`, `std::priority_queue`
- 측정 연산: 전체 원소 `push` 후 우선순위 순서로 전체 `pop`
- 입력 크기: 추후 측정 시 기록
- 측정 횟수: 명령행 인자로 지정
- 시간 단위: 마이크로초(μs)
- 통계: 평균값, 중앙값, 최솟값, 최댓값

#### 결과

| 컨테이너 | 평균 | 중앙값 | 최소 | 최대 |
| --- | ---: | ---: | ---: | ---: |
| MyPriorityQueue | - | - | - | - |
| `std::priority_queue` | - | - | - | - |

> 벤치마크 코드와 Release 측정 결과를 추가한 뒤 작성합니다.

### 성능 테스트 2: Decrease-Key 기반 Dijkstra

`MyPriorityQueue`의 핸들과 `update`를 이용한 decrease-key 방식이 일반적인 `std::priority_queue` 기반 Dijkstra에 미치는 영향을 비교합니다.

#### 비교 기준

- `MyPriorityQueue`: 정점별 핸들을 유지하고 거리 감소 시 기존 원소의 우선순위를 갱신
- `std::priority_queue`: 거리 감소 시 새 항목을 삽입하고, 이후 꺼낸 오래된 항목을 건너뛰는 방식
- 두 구현에 동일한 그래프와 시작 정점 사용
- 최종 최단 거리 결과가 동일한지 검증 후 실행 시간 비교

#### 측정 조건

- 빌드 구성: `Release x64`
- 그래프 생성 조건: 추후 작성
- 정점 및 간선 수: 추후 작성
- 시작 정점: 추후 작성
- 측정 횟수: 추후 작성
- 시간 단위: 마이크로초(μs)

#### 결과

| 구현 | 평균 | 중앙값 | 최소 | 최대 | 큐 삽입 | 우선순위 갱신/중복 건너뜀 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyPriorityQueue decrease-key | - | - | - | - | - | - |
| `std::priority_queue` 중복 삽입 | - | - | - | - | - | - |

#### 분석

<!-- Dijkstra 구현 및 측정 후 직접 작성 -->

## 참고 문헌

- [std::priority_queue - cppreference](https://en.cppreference.com/w/cpp/container/priority_queue.html)
