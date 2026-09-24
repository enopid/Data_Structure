# MyDeque

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

C++ STL의 `std::deque`를 참고해 청크 단위의 분할된 연속 메모리와 링 버퍼 형태의 청크 맵으로 구현한 Deque입니다.

- 양 끝에서 원소 삽입 및 삭제
- 청크 내부의 연속 메모리 사용
- 논리적 시작 위치를 이용한 원형 인덱싱
- 복사·이동 생성 및 대입 지원
- 템플릿 인자를 통한 원소 사이즈에따른 컴파일타임 가변 청크 크기 

```cpp
MyDeque<int> defaultDeque;       // 기본 1024Byte
MyDeque<int, 64> smallDeque;     // 64Byte
MyDeque<int, 4096> largeDeque;   // 4096Byte
```

## 포함 기능

- 삽입: `push_front`, `push_back`
- 삭제: `pop_front`, `pop_back`, `clear`
- 접근: `front`, `back`, `operator[]`
- 상태: `empty`, `size`
- 객체 관리: 복사 생성, 복사 대입, 이동 생성, 이동 대입
- 분석 보조: `print_info`, `print_elements`

참고 문헌: [cppreference - std::deque](https://en.cppreference.com/w/cpp/container/deque.html)

## 분석

### 유효성 테스트

Release x64에서 모든 테스트를 통과했습니다.

```text
[PASS 1/7] default state and invalid-access exceptions
[PASS 2/7] push/pop and chunk boundaries
[PASS 3/7] circular wrapping and chunk growth
[PASS 4/7] clear and storage reuse with std::string
[PASS 5/7] copy construction, assignment and self-assignment
[PASS 6/7] move construction and assignment
[PASS 7/7] 5,000 random operations against std::deque

7 passed, 0 failed
```

<details>
<summary>1. 기본 상태 및 잘못된 접근 예외</summary>

```cpp
MyDeque<int> values;
require(values.empty(), "new deque must be empty");
require(values.size() == 0, "new deque size must be zero");

bool front_threw = false;
bool back_threw = false;
bool index_threw = false;
try { (void)values.front(); } catch (const std::out_of_range&) { front_threw = true; }
try { (void)values.back(); } catch (const std::out_of_range&) { back_threw = true; }
try { (void)values[0]; } catch (const std::out_of_range&) { index_threw = true; }
require(front_threw && back_threw && index_threw,
        "empty access must throw std::out_of_range");
```

</details>

<details>
<summary>2. 청크 경계를 넘는 양방향 삽입·삭제</summary>

```cpp
MyDeque<int, 16> actual;
std::deque<int> expected;
for (int value = 0; value < 40; ++value) {
    if (value % 2 == 0) {
        actual.push_front(value);
        expected.push_front(value);
    } else {
        actual.push_back(value);
        expected.push_back(value);
    }
}
require_equal(actual, expected);

for (int i = 0; i < 10; ++i) {
    actual.pop_front();
    expected.pop_front();
    actual.pop_back();
    expected.pop_back();
}
require_equal(actual, expected);
```

</details>

<details>
<summary>3. 링 버퍼 순환 및 청크 확장</summary>

```cpp
MyDeque<int, 12> actual;
std::deque<int> expected;
for (int value = 0; value < 18; ++value) {
    actual.push_back(value);
    expected.push_back(value);
}
for (int i = 0; i < 11; ++i) {
    actual.pop_front();
    expected.pop_front();
}
for (int value = 18; value < 55; ++value) {
    actual.push_back(value);
    expected.push_back(value);
}
for (int value = -1; value >= -20; --value) {
    actual.push_front(value);
    expected.push_front(value);
}
require_equal(actual, expected);
```

</details>

<details>
<summary>4. 문자열 clear 및 청크 재사용</summary>

```cpp
MyDeque<std::string, 64> values;
for (int i = 0; i < 20; ++i) values.push_back("old-" + std::to_string(i));
values.clear();
require(values.empty() && values.size() == 0, "clear did not empty deque");

values.push_front("middle");
values.push_front("first");
values.push_back("last");
require(values[0] == "first" && values[1] == "middle" && values[2] == "last",
        "reused deque element order mismatch");
```

</details>

<details>
<summary>5. 복사 생성·대입 및 자기 대입</summary>

```cpp
MyDeque<std::string, 64> original;
std::deque<std::string> expected;
for (int i = 0; i < 15; ++i) {
    original.push_back("value-" + std::to_string(i));
    expected.push_back("value-" + std::to_string(i));
}
for (int i = 0; i < 5; ++i) {
    original.pop_front();
    expected.pop_front();
}
original.push_back("tail");
expected.push_back("tail");

MyDeque<std::string, 64> copied(original);
require_equal(copied, expected);
copied[0] = "changed";
require(original[0] == expected[0], "copy constructor shared element storage");

MyDeque<std::string, 64> assigned;
assigned.push_back("discarded");
assigned = original;
require_equal(assigned, expected);
assigned = assigned;
require_equal(assigned, expected);
```

</details>

<details>
<summary>6. 이동 생성 및 이동 대입</summary>

```cpp
MyDeque<std::string, 64> source;
source.push_front("first");
source.push_back("second");

MyDeque<std::string, 64> moved(std::move(source));
require(source.empty(), "move-constructed source is not empty");
require(moved.size() == 2 && moved[0] == "first" && moved[1] == "second",
        "move constructor mismatch");

MyDeque<std::string, 64> assigned;
assigned.push_back("discarded");
assigned = std::move(moved);
require(moved.empty(), "move-assigned source is not empty");
require(assigned.front() == "first" && assigned.back() == "second",
        "move assignment mismatch");
```

</details>

<details>
<summary>7. std::deque와 5,000회 무작위 연산 비교</summary>

```cpp
MyDeque<int, 20> actual;
std::deque<int> expected;
std::mt19937 random(20260905);

for (int step = 0; step < 5000; ++step) {
    const int operation = static_cast<int>(random() % 4);
    if (operation == 0 || expected.empty()) {
        const int value = static_cast<int>(random() % 100000);
        actual.push_front(value);
        expected.push_front(value);
    } else if (operation == 1) {
        const int value = static_cast<int>(random() % 100000);
        actual.push_back(value);
        expected.push_back(value);
    } else if (operation == 2) {
        actual.pop_front();
        expected.pop_front();
    } else {
        actual.pop_back();
        expected.pop_back();
    }
    require_equal(actual, expected);
}
```

</details>

### 부하 테스트

#### 실행 방법

```text
# 기존 혼합 연산 및 청크 크기 비교
MyDeque.exe --benchmark [연산 수] [측정 횟수]

# 순차 push_back 전용 비교
MyDeque.exe --push-back-benchmark [원소 수] [측정 횟수]
```

`--push-back-benchmark`는 기존 혼합 연산 테스트를 변경하지 않고, 동일한 개수의 `int`를 순차적으로 추가하는 별도 비교 케이스입니다. MyDeque의 16B, 64B, 256B, 1024B, 4096B 청크 설정과 `std::deque`를 비교합니다. Vector와 List는 각 프로젝트에서 동일한 원소 수로 측정한 기존 결과를 사용합니다.

#### 측정 조건

문서의 결과는 다음 조건으로 측정했습니다.

- 빌드 구성: Visual Studio 2022 `Release x64`
- 원소 타입: `int`
- 입력 크기: 각 컨테이너에 원소 1,000,000개 추가
- 측정 횟수: 각 케이스 30회
- 측정 연산: 순차 `push_back`
- 시간 단위: 마이크로초(μs)
- 모든 케이스 체크섬: `1,999,999`

#### Release 측정 결과

| 구현 | 청크/블록 크기 | 청크당 int | 평균 | 중앙값 | 최소 | 최대 | std::deque 대비 중앙값 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `MyDeque<int, 16>` | 16B | 4 | 16,022μs | 15,903μs | 14,860μs | 18,401μs | 0.94배 |
| `MyDeque<int, 64>` | 64B | 16 | 7,121μs | 7,028μs | 6,022μs | 10,640μs | 0.41배 |
| `MyDeque<int, 256>` | 256B | 64 | 3,545μs | 3,493μs | 2,940μs | 4,499μs | 0.21배 |
| `MyDeque<int, 1024>` | 1,024B | 256 | 2,755μs | 2,689μs | 2,365μs | 3,350μs | 0.16배 |
| `MyDeque<int, 4096>` | 4,096B | 1,024 | 2,450μs | 2,394μs | 2,192μs | 2,966μs | 0.14배 |
| `std::deque<int>` | 16B¹ | 4 | 16,979μs | 16,968μs | 15,481μs | 18,975μs | 기준 |

¹ 이번 측정에 사용한 MSVC STL 14.44 구현 기준입니다. STL의 내부 블록 정책은 구현체와 버전에 따라 달라질 수 있습니다.

#### Release 측정 결과 분석

<details>
<summary><strong>0. List, Vector와 비교</strong></summary>

각 자료구조에 1,000,000개의 `int`를 순차적으로 `push_back`한 결과를 비교했습니다. Vector와 List는 각 프로젝트에서 동일한 입력 크기로 측정한 기존 Release 결과를 사용했습니다.

| 자료구조 | 설정 | 중앙값 |
| --- | --- | ---: |
| MyLinkedList | 노드 단위 할당 | 50,680μs |
| MyDeque | 16B 청크 | 15,903μs |
| MyDeque | 64B 청크 | 7,028μs |
| MyDeque | 256B 청크 | 3,493μs |
| MyDeque | 1,024B 청크 | 2,689μs |
| MyDeque | 4,096B 청크 | 2,394μs |
| MyVector | Factor 1.5 | 2,992μs |
| MyVector | Factor 2.0 | 1,875μs |

Deque는 여러 원소를 하나의 청크에 저장하므로 청크가 커질수록 청크 개수가 줄어 링버퍼의 관리비용은 감소하고, 같은 청크 안에서 연속된 메모리를 사용하여 접근 지역성도 개선됩니다.
<br>테스트 결과, 청크의 크기가 작을수록 리스트에 가까운 성능을 보이고 청크의 크기가 커질수록 벡터에 가까운 성능을 보였습니다.
<br>이를 통해 적절한 크기의 청크 설정시, 벡터와 같은 연속적인 메모리와 임의 접근의 장점을 유지한 체 리스트와 같이 양 끝단에서 노드의 삽입/삭제가 가능한 구조임을 비교를 통해 파악할수있습니다.

</details>

<details>
<summary><strong>1. STL Deque와 비교</strong></summary>

stl의 데크의 경우 청크당 크기가 비교적 작은 16B로 고정되어 1,000,000개의 `int`를 저장하려면 약 250,000개의 작은 블록이 필요하므로 블록 할당 횟수와 블록 포인터 관리 비용이 커집니다.

| 구현 | 청크당 int | 필요한 청크 수(근사) | 중앙값 |
| --- | ---: | ---: | ---: |
| `std::deque<int>` | 4 | 250,000 | 16,968μs |
| `MyDeque<int, 16>` | 4 | 250,000 | 15,903μs |
| `MyDeque<int, 64>` | 16 | 62,500 | 7,028μs |
| `MyDeque<int, 256>` | 64 | 15,625 | 3,493μs |
| `MyDeque<int, 1024>` | 256 | 약 3,907 | 2,689μs |
| `MyDeque<int, 4096>` | 1,024 | 약 977 | 2,394μs |

MSVC STL의 16B 블록 정책은 적은 원소를 저장할 때 메모리 낭비를 줄일 수 있지만, `int`를 대량으로 순차 삽입하는 경우 블록 하나에 원소가 4개만 들어가 지나치게 많은 작은 블록이 생성됩니다. 
<br> 청크의 크기가 4096B일때는 성능 차이가 8배 가량 발생하며 stl deque의 작은 블록 정책이 가지는 치명적인 단점을 확인할 수 있습니다.

</details>
