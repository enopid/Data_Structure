# MyDeque

> 자료구조의 핵심 로직은 직접 구현했으며, `Main.cpp`의 유효성 테스트와 벤치마크 코드는 AI를 활용해 작성하고 직접 실행하여 검증했습니다.

## 개요

C++ STL의 `std::deque`를 참고해 청크 단위의 분할된 연속 메모리와 링 버퍼 형태의 청크 맵으로 구현한 Deque입니다.

- 양 끝에서 원소 삽입 및 삭제
- 청크 내부의 연속 메모리 사용
- 논리적 시작 위치를 이용한 원형 인덱싱
- 복사·이동 생성 및 대입 지원
- 템플릿 인자를 통한 청크 크기 변경

```cpp
MyDeque<int> defaultDeque;       // 기본 1024Byte
MyDeque<int, 64> smallDeque;     // 64Byte
MyDeque<int, 4096> largeDeque;   // 4096Byte
```

## 기능

- 삽입: `push_front`, `push_back`
- 삭제: `pop_front`, `pop_back`, `clear`
- 접근: `front`, `back`, `operator[]`
- 상태: `empty`, `size`
- 객체 관리: 복사 생성, 복사 대입, 이동 생성, 이동 대입
- 분석 보조: `print_info`, `print_elements`

참고 문헌: [cppreference - std::deque](https://en.cppreference.com/w/cpp/container/deque.html)

## 유효성 테스트

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

## 부하 테스트

### 실행 방법

```text
MyDeque.exe --benchmark [원소 수] [측정 횟수]
```

문서의 결과는 다음 조건으로 측정했습니다.

- 빌드: Visual Studio 2022 Release x64
- 원소 타입: `int`
- 원소 수: 100,000개
- 측정 횟수: 각 케이스 100회
- 작업: `push_front`/`push_back` 교대 실행 후 `pop_front`/`pop_back` 교대 실행
- 시간 단위: microseconds (`us`)
- 모든 케이스 체크섬: `4,999,950,000`

### Release 측정 결과

| 구현 | 청크/블록 크기 | 청크당 int | 평균 | 중앙값 | 최소 | 최대 | std::deque 대비 평균 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `MyDeque<int, 16>` | 16B | 4 | 2,434us | 2,390us | 2,144us | 3,460us | 1.21배 |
| `MyDeque<int, 64>` | 64B | 16 | 1,185us | 1,122us | 1,035us | 1,723us | 0.59배 |
| `MyDeque<int, 256>` | 256B | 64 | 813us | 784us | 711us | 1,137us | 0.40배 |
| `MyDeque<int, 1024>` | 1,024B | 256 | 757us | 713us | 615us | 1,434us | 0.38배 |
| `MyDeque<int, 4096>` | 4,096B | 1,024 | 693us | 593us | 537us | 7,254us | 0.34배 |
| `std::deque<int>` | 16B¹ | 4 | 2,018us | 1,698us | 1,514us | 6,213us | 기준 |

¹ 이번 측정에 사용한 MSVC STL 14.44 구현 기준입니다. STL의 내부 블록 정책은 구현체와 버전에 따라 달라질 수 있습니다.

### 분석

- 기존 `Grow()`는 가득 찰 때마다 청크를 하나만 추가하고 전체 포인터를 재배치했습니다. 청크 수가 늘어날수록 누적 비용이 크게 증가했습니다.
- 청크 배열을 `1 → 2 → 4 → 8` 형태로 두 배씩 확장하도록 변경했습니다. 포인터 재배치는 남아 있지만 발생 횟수가 기하급수적으로 줄어 상환 비용이 낮아졌습니다.
- MSVC `std::deque<int>`도 16Byte 블록을 사용하지만, 블록 포인터 맵은 여유 슬롯을 두고 필요할 때만 확장합니다. 같은 16Byte 설정에서 `MyDeque`의 중앙값은 2,390us, STL은 1,698us로 `MyDeque`가 약 1.41배 느렸습니다.
- 64Byte 이상의 `MyDeque`는 이 부하에서 STL보다 낮은 평균과 중앙값을 보였습니다. 4,096Byte 청크의 중앙값은 593us로 가장 낮았습니다.
- 4,096Byte `MyDeque`와 STL의 최대값은 각각 7,254us와 6,213us로 평소보다 크게 튀었습니다. 백그라운드 스케줄링과 메모리 할당 변동의 영향을 줄이기 위해 대표값은 중앙값을 함께 확인했습니다.
- 64Byte 이상은 현재 부하에서 `std::deque`보다 빠르게 측정됐습니다. 다만 더 큰 청크는 적은 원소만 보관할 때 미사용 메모리가 증가합니다.
- 이 결과는 `int`와 해당 양방향 부하에 한정되므로 자료구조 전체의 일반적인 우위로 해석할 수 없습니다.
