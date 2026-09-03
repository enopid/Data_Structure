# MyVector

## AI 활용 범위

> `Main.cpp`의 유효성 테스트와 벤치마크 코드에는 AI를 적극적으로 활용했습니다. 생성된 테스트와 측정 코드는 직접 검토하고 Release 환경에서 실행하여 결과를 확인했습니다.

- 테스트 항목 설계, 자체 테스트 러너 및 벤치마크 코드 작성: AI 활용
- `MyVector`의 메모리 관리와 자료구조 핵심 로직: 직접 설계 및 구현
- AI 생성 코드의 역할: 직접 구현한 자료구조의 유효성 검증 및 성능 측정

## 개요

`MyVector<T>`는 STL의 `std::vector`를 따라 구현한 동적 배열입니다.<br>
원소를 연속된 메모리 공간에 저장하고, 공간이 부족하면 기존 용량에 일정 비율을 곱하는 Geometric Growth 방식으로 더 큰 공간을 할당합니다.

### 구현 중심점
- Geometric Growth 방식의 메모리 할당
- 원소 타입의 이동과 복사를 고려한 재할당
- 불필요한 원소 생성을 피하기 위해 `malloc`으로 원시 메모리 공간 확보
- Placement New를 사용해 필요한 위치에서만 원소 생성
- 복사·이동 생성자와 복사·이동 대입 연산자 구현

## 포함 기능

| 구분 | 메서드 |
| --- | --- |
| 생성 및 소유권 | `MyVector`, 복사/이동 생성자, 복사/이동 대입, 소멸자 |
| 상태 조회 | `size`, `capacity`, `data`, `empty` |
| 용량 관리 | `resize`, `reserve`, `shrink_to_fit`, `clear` |
| 원소 변경 | `push_back`, `pop_back`, `insert`, `erase` |
| 원소 접근 | `operator[]`, `front`, `back` |
| 분석 보조 | `set_growth_factor`, `linear_push_back` |

### 분석 보조 기능

- `set_growth_factor(double growth_factor)`
  - `push_back`에서 사용할 용량 증가 계수를 변경합니다.
  - 기본 증가 계수는 MSVC `std::vector`와 유사한 `1.5`입니다.
  - 증가 계수에 따른 재할당 횟수, 메모리 여유 공간과 실행 시간의 변화를 비교하기 위한 기능입니다.
- `linear_push_back(const T& val)`
  - 용량이 부족할 때 기존 용량에 `10`을 더해 선형적으로 확장합니다.
  - Geometric Growth 방식과 Linear Growth 방식의 누적 재할당 비용을 비교하기 위한 기능입니다.

## 분석

### 유효성 테스트

별도의 테스트 프레임워크 없이 자체 테스트 러너를 사용하며, 일부 연산은 동일한 입력을 적용한 `std::vector`의 결과와 비교합니다.

| 테스트 | 확인 내용 | 결과 |
| --- | --- | --- |
| 기본 상태 | 기본 생성 후 크기, 용량, 빈 상태와 데이터 포인터 | PASS |
| 추가·제거·접근 | `push_back`, `pop_back`, `front`, `back`, `operator[]` | PASS |
| 용량 관리 | `reserve`, `resize`, `shrink_to_fit`, `clear` | PASS |
| 삽입·삭제 | `std::string` 원소의 `insert`, `erase` | PASS |
| 복사·이동 | 복사/이동 생성자 및 복사/이동 대입 연산자 | PASS |
| 객체 수명 | 비단순 타입의 생성, 이동, 소멸 및 잔존 객체 수 | PASS |
| 무작위 비교 | 1,000회의 무작위 연산 결과를 `std::vector`와 비교 | PASS |
| Growth Factor 변경 | 증가 계수 `1.5`와 `2.0`에서 예상한 용량으로 확장되는지 비교 | PASS |

```text
8 passed, 0 failed
```

<details>
<summary>1. 기본 상태 테스트 코드</summary>

```cpp
void test_default_state() {
    MyVector<int> values;
    require(values.empty(), "new vector must be empty");
    require(values.size() == 0, "new vector size must be zero");
    require(values.capacity() >= 1, "new vector must have initial storage");
    require(values.data() != nullptr, "data pointer must be valid");
}
```

</details>

<details>
<summary>2. 추가·제거·원소 접근 테스트 코드</summary>

```cpp
void test_push_and_access() {
    MyVector<int> actual;
    std::vector<int> expected;
    for (int i = 0; i < 100; ++i) {
        actual.push_back(i * 3);
        expected.push_back(i * 3);
    }
    require_equal(actual, expected);
    require(actual.front() == expected.front(), "front mismatch");
    require(actual.back() == expected.back(), "back mismatch");
    for (int i = 0; i < 25; ++i) {
        actual.pop_back();
        expected.pop_back();
    }
    require_equal(actual, expected);
}
```

</details>

<details>
<summary>3. 용량 관리 테스트 코드</summary>

```cpp
void test_capacity_operations() {
    MyVector<int> values(4, 7);
    values.reserve(32);
    require(values.capacity() >= 32, "reserve did not increase capacity");
    require(values.size() == 4, "reserve changed size");
    values.resize(10, 9);
    require(values.size() == 10, "resize did not grow size");
    for (int i = 0; i < 4; ++i) require(values[i] == 7, "resize changed old value");
    for (int i = 4; i < 10; ++i) require(values[i] == 9, "resize fill value mismatch");
    values.resize(3);
    values.shrink_to_fit();
    require(values.size() == 3 && values.capacity() == 3, "shrink_to_fit mismatch");
    values.clear();
    require(values.empty(), "clear did not empty vector");
}
```

</details>

<details>
<summary>4. 삽입·삭제 테스트 코드</summary>

```cpp
void test_insert_and_erase() {
    MyVector<std::string> actual(3, "base");
    std::vector<std::string> expected(3, "base");
    actual.insert(1, "inserted");
    expected.insert(expected.begin() + 1, "inserted");
    actual.erase(2);
    expected.erase(expected.begin() + 2);
    require_equal(actual, expected);
}
```

</details>

<details>
<summary>5. 복사·이동 테스트 코드</summary>

```cpp
void test_copy_and_move() {
    MyVector<std::string> original;
    original.push_back("alpha");
    original.push_back("beta");
    MyVector<std::string> copied(original);
    copied[0] = "changed";
    require(original[0] == "alpha", "copy constructor shared storage");
    MyVector<std::string> assigned;
    assigned = original;
    require(assigned.size() == 2 && assigned[1] == "beta", "copy assignment mismatch");
    MyVector<std::string> moved(std::move(copied));
    require(moved.size() == 2 && moved[0] == "changed", "move constructor mismatch");
    require(copied.size() == 0, "moved-from vector is not empty");
    MyVector<std::string> move_assigned;
    move_assigned = std::move(assigned);
    require(move_assigned.size() == 2, "move assignment mismatch");
    require(assigned.size() == 0, "move-assigned source is not empty");
}
```

</details>

<details>
<summary>6. 비단순 객체 수명 테스트 코드</summary>

```cpp
void test_non_trivial_lifetime() {
    TrackedValue::reset();
    {
        MyVector<TrackedValue> values;
        TrackedValue first(1);
        TrackedValue second(2);
        values.push_back(first);
        values.push_back(second);
        values.reserve(16);
        values.erase(0);
        require(values.size() == 1 && values[0].value == 2, "tracked value mismatch");
        require(TrackedValue::moves > 0, "reallocation did not move values");
    }
    require(TrackedValue::alive == 0, "tracked object lifetime leak");
}
```

</details>

<details>
<summary>7. std::vector 무작위 비교 테스트 코드</summary>

```cpp
void test_against_std_vector() {
    MyVector<int> actual;
    std::vector<int> expected;
    std::mt19937 random(20260827);
    for (int step = 0; step < 1000; ++step) {
        const int operation = static_cast<int>(random() % 4);
        if (operation == 0 || expected.empty()) {
            const int value = static_cast<int>(random() % 10000);
            actual.push_back(value);
            expected.push_back(value);
        } else if (operation == 1) {
            actual.pop_back();
            expected.pop_back();
        } else if (operation == 2) {
            const int position = static_cast<int>(random() % expected.size());
            const int value = static_cast<int>(random() % 10000);
            actual.insert(position, value);
            expected.insert(expected.begin() + position, value);
        } else {
            const int position = static_cast<int>(random() % expected.size());
            actual.erase(position);
            expected.erase(expected.begin() + position);
        }
        require_equal(actual, expected);
    }
}
```

</details>

<details>
<summary>8. Growth Factor 변경 테스트 코드</summary>

```cpp
void test_growth_factor_change() {
    MyVector<int>::set_growth_factor(1.5);
    MyVector<int> default_growth;
    for (int i = 0; i < 5; ++i) default_growth.push_back(i);
    require(default_growth.capacity() == 6,
            "growth factor 1.5 must produce capacity 6 after five pushes");

    MyVector<int>::set_growth_factor(2.0);
    MyVector<int> doubled_growth;
    for (int i = 0; i < 5; ++i) doubled_growth.push_back(i);
    require(doubled_growth.capacity() == 8,
            "growth factor 2.0 must produce capacity 8 after five pushes");

    MyVector<int>::set_growth_factor(1.5);
}
```

</details>

### 벤치마크 결과

- 빌드 구성: `Release x64`
- 원소 타입: `int` 또는 `std::string`
- 입력 크기: 각 컨테이너에 원소 10,000개 추가
- 측정 횟수: 명령행 인자로 지정
- 시간 단위: 마이크로초(μs)
- 측정 연산: `push_back` 또는 `linear_push_back`

실행 명령은 다음과 같습니다.

```powershell
# int 벤치마크
MyVector.exe --benchmark 10000 30 int

# std::string 벤치마크
MyVector.exe --benchmark 10000 5 string
```

첫 번째 숫자 `10000`은 각 방식에 추가할 원소 수이고, 두 번째 숫자는 각 방식을 반복 측정할 횟수입니다. 마지막 인자는 원소 타입으로 `int` 또는 `string`을 선택합니다. 원소 타입을 생략하면 `int`, 반복 횟수를 생략하면 10회를 기본값으로 사용합니다.

`string` 옵션은 고정된 난수 시드로 생성한 24자 문자열을 사용합니다. 모든 컨테이너에 동일한 문자열을 입력하며, 문자열 생성 시간은 측정에서 제외합니다. 반복 결과는 평균값, 중앙값, 최솟값과 최댓값으로 출력합니다.

#### `int` 결과

원소 10,000개를 각 방식에서 30회씩 측정했습니다.

| 방식 | 평균 | 중앙값 | 최소 | 최대 | 재할당 | 최종 용량 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyVector Geometric Growth 1.5 (기본값) | 42μs | 43μs | 29μs | 55μs | 23회 | 12,138 |
| MyVector Geometric Growth 2.0 (변경값) | 38μs | 38μs | 29μs | 73μs | 14회 | 16,384 |
| MyVector Linear Growth | 1,299μs | 546μs | 452μs | 5,861μs | 1,000회 | 10,001 |
| `std::vector` | 15μs | 14μs | 12μs | 36μs | 24회 | 12,138 |

#### `std::string` 결과

24자의 무작위 문자열 10,000개를 각 방식에서 5회씩 측정했습니다.

| 방식 | 평균 | 중앙값 | 최소 | 최대 | 재할당 | 최종 용량 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyVector Geometric Growth 1.5 (기본값) | 775μs | 688μs | 627μs | 1,212μs | 23회 | 12,138 |
| MyVector Geometric Growth 2.0 (변경값) | 609μs | 607μs | 586μs | 634μs | 14회 | 16,384 |
| MyVector Linear Growth | 40,395μs | 41,581μs | 27,757μs | 54,571μs | 1,000회 | 10,001 |
| `std::vector` | 645μs | 607μs | 596μs | 727μs | 24회 | 12,138 |

기본 증가 계수 1.5에서 MyVector와 `std::vector`의 최종 용량은 12,138로 동일했습니다. 표시된 재할당 횟수가 23회와 24회로 다른 이유는 MyVector가 생성 시 확보하는 최초 capacity 1을 현재 측정 코드에서 재할당으로 집계하지 않는 반면, `std::vector`는 capacity 0에서 시작하여 최초 할당도 집계되기 때문입니다.

증가 계수를 2.0으로 변경하면 최종 여유 공간은 늘어나지만 재할당 횟수는 14회로 감소하여 MyVector 1.5보다 중앙값이 줄었습니다. 그러나 `int` 30회 반복 결과에서는 두 MyVector 설정 모두 `std::vector`보다 느렸습니다. Linear Growth 방식은 재할당이 1,000회 발생했고 최댓값도 크게 흔들렸습니다.

`std::string` 측정에서는 MyVector 기본값의 평균 시간이 `std::vector`보다 느렸고, 중앙값은 Growth Factor 2.0과 `std::vector`가 동일하게 측정됐습니다. 비단순 타입에서는 원소의 이동·복사와 메모리 할당 비용이 함께 반영되므로 `int` 결과와 양상이 달라질 수 있습니다.

`int`는 각 방식당 30회, `std::string`은 각 방식당 5회 측정했습니다. 실행 시간이 짧아 캐시, 실행 순서와 시스템 상태의 영향을 받을 수 있으므로 해당 수치는 현재 환경과 입력 조건에서 얻은 결과로 한정합니다.

## 참고 문헌

- [std::vector - cppreference](https://en.cppreference.com/w/cpp/container/vector.html)
