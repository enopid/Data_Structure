# MyLinkedList

## AI 활용 범위

> `Main.cpp`의 유효성 테스트와 벤치마크 코드에는 AI를 적극적으로 활용했습니다. 생성된 코드는 직접 검토하고 Release 환경에서 실행하여 결과를 확인했습니다.

- 테스트 항목 설계, 자체 테스트 러너 및 벤치마크 코드 작성: AI 활용
- `MyLinkedList`의 노드 연결과 자료구조 핵심 로직: 직접 설계 및 구현
- AI 생성 코드의 역할: 직접 구현한 자료구조의 유효성 검증 및 성능 측정

## 개요

`MyLinkedList<T>`는 STL의 `std::list`를 따라 구현한 양방향 연결 리스트입니다.<br>
각 원소를 독립된 노드에 저장하며, 모든 노드는 이전 노드와 다음 노드를 가리킵니다.

### 구현 중심점

- 이전·다음 노드를 모두 연결하는 Doubly Linked List 구조
- 처음과 끝의 예외 처리를 단순화하기 위한 Sentinel Node 사용
- 일반·상수 반복자를 하나의 템플릿으로 구분
- 정방향 및 역방향 순회 지원
- 복사·이동 생성자와 복사·이동 대입 연산자 구현

## 포함 기능

| 구분 | 메서드 |
| --- | --- |
| 생성 및 소유권 | `MyLinkedList`, 복사/이동 생성자, 복사/이동 대입, 소멸자 |
| 상태 조회 | `size`, `empty` |
| 원소 변경 | `push_front`, `push_back`, `pop_front`, `pop_back`, `insert`, `erase`, `clear` |
| 원소 접근 | `front`, `back` |
| 정방향 순회 | `begin`, `end`, `cbegin`, `cend` |
| 역방향 순회 | `rbegin`, `rend`, `crbegin`, `crend` |
| 출력 | `operator<<` |

## 분석

### 유효성 테스트

별도의 테스트 프레임워크 없이 자체 테스트 러너를 사용합니다. 컨테이너 상태와 원소 순서는 동일한 연산을 적용한 `std::list`의 결과와 비교합니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| 생성자 | 기본 생성, 기본값 생성, 지정값 생성 | PASS |
| 추가·제거·접근 | 양 끝의 추가·제거, `clear`, `front`, `back` | PASS |
| 삽입·삭제 | `std::string` 원소의 중간·끝 삽입과 삭제 | PASS |
| 반복자 | 정방향, 역방향, 상수 순회와 스트림 출력 | PASS |
| 복사 | 복사 생성, 복사 대입, 자기 대입 | PASS |
| 이동 | 이동 생성, 이동 대입, 빈 원본 이동 | PASS |
| 객체 수명 | 비단순 타입의 복사, 삭제 및 잔존 객체 수 | PASS |
| 무작위 비교 | 1,000회의 무작위 연산 결과를 `std::list`와 비교 | PASS |

```text
8 passed, 0 failed
```

<details>
<summary>실제 유효성 테스트 코드 보기</summary>

아래 함수가 8개의 테스트를 순서대로 실행합니다. 각 테스트 함수의 전체 검증 코드는 [`Private/Main.cpp`](Private/Main.cpp)에서 확인할 수 있습니다.

```cpp
int run_tests() {
    TestRunner runner(8);
    std::cout << "MyLinkedList validity tests\n\n";
    runner.run("Default and fill constructors", test_default_and_fill_constructors);
    runner.run("Push, pop, clear and element access", test_push_pop_and_access);
    runner.run("Insert and erase with std::string", test_insert_and_erase);
    runner.run("Forward, reverse and const iteration", test_forward_reverse_and_const_iteration);
    runner.run("Copy construction, assignment and self-assignment", test_copy_semantics);
    runner.run("Move construction, assignment and empty source", test_move_semantics);
    runner.run("Non-trivial object lifetime and destruction", test_non_trivial_lifetime);
    runner.run("1,000 random operations against std::list", test_against_std_list);
    return runner.report();
}
```

</details>

### 벤치마크 실행

```powershell
# int 벤치마크
MyList.exe --benchmark 10000 30 int

# std::string 벤치마크
MyList.exe --benchmark 10000 5 string
```

첫 번째 숫자는 각 컨테이너에 추가할 원소 수이고, 두 번째 숫자는 각 방식을 반복 측정할 횟수입니다. 마지막 인자는 `int` 또는 `string`을 선택합니다. 타입을 생략하면 `int`, 반복 횟수를 생략하면 10회를 사용합니다.

`string` 옵션은 고정된 난수 시드로 미리 생성한 24자 문자열을 사용합니다. 입력 생성 시간은 측정에서 제외하며, 두 컨테이너에는 동일한 데이터를 입력합니다.

### 벤치마크 조건

- 빌드 구성: `Release x64`
- 측정 연산: 순차 `push_back`
- 입력 크기: 각 컨테이너에 원소 10,000개 추가
- 시간 단위: 마이크로초(μs)
- 통계: 평균값, 중앙값, 최솟값, 최댓값

#### `int` 결과

원소 10,000개를 각 컨테이너에서 30회씩 측정했습니다.

| 컨테이너 | 평균 | 중앙값 | 최소 | 최대 |
| --- | ---: | ---: | ---: | ---: |
| MyLinkedList | 448μs | 442μs | 434μs | 492μs |
| `std::list` | 427μs | 425μs | 392μs | 512μs |

#### `std::string` 결과

24자의 무작위 문자열 10,000개를 각 컨테이너에서 5회씩 측정했습니다.

| 컨테이너 | 평균 | 중앙값 | 최소 | 최대 |
| --- | ---: | ---: | ---: | ---: |
| MyLinkedList | 1,234μs | 1,225μs | 1,162μs | 1,324μs |
| `std::list` | 1,190μs | 1,186μs | 1,122μs | 1,318μs |

두 원소 타입 모두 이번 측정에서는 `std::list`의 평균값과 중앙값이 MyLinkedList보다 낮았습니다. `int`의 중앙값 차이는 17μs, `std::string`의 중앙값 차이는 39μs였습니다.

현재 벤치마크는 순차 `push_back`만 비교합니다. 실행 시간은 메모리 할당자, 캐시 및 시스템 상태의 영향을 받으므로 결과는 현재 환경과 입력 조건에 한정합니다.

## 참고 문헌

- [std::list - cppreference](https://en.cppreference.com/w/cpp/container/list.html)
