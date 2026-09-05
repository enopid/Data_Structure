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

#### 결과

| 컨테이너 | 평균 | 중앙값 | 최소 | 최대 |
| --- | ---: | ---: | ---: | ---: |
| MyPriorityQueue | 12,237μs | 12,238μs | 11,170μs | 13,894μs |
| `std::priority_queue` | 4,887μs | 4,880μs | 4,766μs | 5,038μs |

두 구현의 checksum은 `49,787,350,884`로 동일했습니다. 이번 조건에서 `std::priority_queue`의 중앙값은 MyPriorityQueue보다 약 2.51배 빨랐습니다.

MyPriorityQueue는 일반적인 힙 연산 외에도 각 원소의 핸들 슬롯을 생성하고, 노드를 교환할 때마다 슬롯의 힙 인덱스를 갱신합니다. 이 부하 테스트에서는 `update`나 `erase`를 사용하지 않으므로 해당 관리 비용만 추가되고 핸들의 이점은 활용되지 않습니다. 따라서 이 결과는 일반적인 `push`·`pop` 부하를 비교하는 기준이며, decrease-key를 사용하는 Dijkstra 비교는 별도로 측정합니다.

실행 시간은 시스템 상태, 캐시 및 메모리 할당자의 영향을 받으므로 수치는 현재 환경과 입력 조건에 한정합니다.

### 성능 테스트 2: Decrease-Key 기반 Dijkstra

`MyPriorityQueue`의 핸들과 `update`를 이용한 decrease-key 방식이 일반적인 `std::priority_queue` 기반 Dijkstra에 미치는 영향을 비교합니다.

- 문제: [프로그래머스 118669 - 등산코스 정하기](https://school.programmers.co.kr/learn/courses/30/lessons/118669)
- [STL/Decrease-Key 통합 제출 코드](Analysis/Programmers118669.cpp)

하나의 파일에 두 방식을 함께 구성했습니다. 파일 상단의 `USE_DECREASE_KEY`를 `0`으로 설정하면 `std::priority_queue` 중복 삽입 방식을, `1`로 설정하면 커스텀 PriorityQueue의 decrease-key 방식을 사용합니다. 커스텀 PriorityQueue 구현도 파일에 포함되어 있어 그대로 프로그래머스에 제출할 수 있습니다.

#### 비교 기준

- `MyPriorityQueue`: 정점별 핸들을 유지하고 거리 감소 시 기존 원소의 우선순위를 갱신
- `std::priority_queue`: 거리 감소 시 새 항목을 삽입하고, 이후 꺼낸 오래된 항목을 건너뛰는 방식
- 두 구현에 동일한 그래프와 출입구 사용
- 여러 출입구를 시작점으로 하는 Multi-Source Dijkstra 적용
- 경로 비용의 합이 아닌 경로에서 가장 큰 간선 비용인 intensity 갱신
- 산봉우리에서는 인접 정점으로 탐색을 확장하지 않음

#### 측정 조건

- 측정 환경: 프로그래머스 C++ 채점 서버
- 대상 문제: 118669 등산코스 정하기
- 정점 수: 최대 50,000개
- 간선 수: 최대 200,000개
- 시간 단위: 밀리초(ms)
- 메모리 단위: MB
- 제출 방식: `USE_DECREASE_KEY`만 변경하여 두 방식 별도 제출

#### 결과

| 구현 | 평균 시간 | 중앙값 | 최소 | 최대 | 평균 메모리 | 최대 메모리 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyPriorityQueue decrease-key | 5.940ms | 0.440ms | 0.010ms | 37.770ms | 17.602MB | 54.7MB |
| `std::priority_queue` 중복 삽입 | 5.762ms | 0.500ms | 0.010ms | 36.510ms | 17.527MB | 53.5MB |

실행 시간이 비교적 큰 14~25번 테스트만 분리한 결과는 다음과 같습니다.

| 구현 | 평균 시간 | 중앙값 | 최소 | 최대 | 평균 메모리 |
| --- | ---: | ---: | ---: | ---: | ---: |
| MyPriorityQueue decrease-key | 12.315ms | 10.520ms | 1.030ms | 37.770ms | 31.518MB |
| `std::priority_queue` 중복 삽입 | 11.941ms | 10.110ms | 1.080ms | 36.510ms | 31.223MB |

#### 분석

두 방식 모두 25개 테스트를 통과했습니다. Lazy insertion을 적용한 decrease-key 방식의 전체 평균은 5.940ms로, 기존 eager 방식의 6.467ms보다 약 8.1% 개선됐습니다. 중앙값도 0.530ms에서 0.440ms로 감소했습니다.

STL 재측정 결과와 비교하면 decrease-key 방식의 전체 평균은 약 3.1% 높았고, 무거운 14~25번 테스트의 평균도 약 3.1% 높았습니다. 표시된 실행 시간 기준으로 STL 방식이 11개에서 빨랐고 decrease-key 방식이 5개에서 빨랐으며 9개는 같았습니다. 전체 중앙값은 decrease-key 방식이 낮았지만, 무거운 구간의 중앙값은 STL 방식이 낮았습니다.

모든 정점을 미리 삽입하지 않도록 변경하면서 불필요한 힙 노드와 핸들 생성 비용이 줄어 두 구현의 실행 시간이 거의 같은 수준까지 좁혀졌습니다. 다만 decrease-key 방식은 노드 교환 시 핸들 인덱스 갱신과 유효성 검사를 수행하므로 평균에서 STL보다 소폭 느렸습니다.

전체 평균 메모리 차이는 약 0.075MB, 무거운 구간의 평균 차이는 약 0.295MB였습니다. 최대 메모리는 decrease-key 방식 54.7MB, STL 방식 53.5MB로 커스텀 구현이 조금 높았습니다.

프로그래머스 수치는 서로 다른 제출 실행에서 얻은 값이므로 서버 상태와 측정 해상도의 영향을 받을 수 있습니다. 3.1% 정도의 차이만으로 어느 구현이 일반적으로 더 빠르다고 단정하기는 어렵지만, 이번 제출에서는 STL 방식이 소폭 우세했습니다.

## 참고 문헌

- [std::priority_queue - cppreference](https://en.cppreference.com/w/cpp/container/priority_queue.html)
- [Boost.Heap Data Structures](https://www.boost.org/doc/libs/latest/doc/html/heap/data_structures.html)
- [boost::heap::d_ary_heap - mutable handle, update, decrease, erase](https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1heap_1_1d__ary__heap.html)
