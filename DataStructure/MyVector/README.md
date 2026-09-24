# MyVector

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 개요

`MyVector<T>`는 STL의 `std::vector`를 따라 구현한 동적 배열입니다.<br>
원소를 연속된 메모리 공간에 저장하고, 공간이 부족하면 기존 용량에 일정 비율을 곱하는 Geometric Growth 방식으로 더 큰 공간을 할당합니다.


<details>
<summary>(Geometric Growth라는 용어는 stl에서 사용하는 용어를 그대로 가져왔습니다)</summary>
  
```cpp
    //stl vector 일부 캡쳐
    _CONSTEXPR20 size_type _Calculate_growth(const size_type _Newsize) const {
        // given _Oldcapacity and _Newsize, calculate geometric growth
        const size_type _Oldcapacity = capacity();
        const auto _Max              = max_size();

        if (_Oldcapacity > _Max - _Oldcapacity / 2) {
            return _Max; // geometric growth would overflow
        }

        const size_type _Geometric = _Oldcapacity + _Oldcapacity / 2;

        if (_Geometric < _Newsize) {
            return _Newsize; // geometric growth would be insufficient
        }

        return _Geometric; // geometric growth is sufficient
    }
```
</details>

### 구현 중심점
- **Geometric Growth 방식의 메모리 할당**<br>(기본적인 벡터 구조의 메모리 관리 방식 적용)
- **원소 타입의 이동과 복사를 고려한 재할당**<br>(복사시는 trivially_copyable 가능시 memcpy 사용 / 이동시는 move_if_noexcept 사용을 통한 강한 예외 보장)
- **불필요한 원소 생성을 피하기 위해 `malloc`으로 원시 메모리 공간 확보**<br>(기본 생성자 없는 타입도 호환 + 최초 메모리 할당시 기본 생성자 호출로 인한 오버헤드 방지)
- **Placement New를 사용해 필요한 위치에서만 원소 생성**<br>(malloc을 통한 커스텀 메모리관리 방식에 따른 할당 방식 사용)
- **복사·이동 생성자와 복사·이동 대입 연산자 구현**<br>(rule of five 적용)

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

### 부하 테스트

#### 측정 조건

- 빌드 구성: `Release x64`
- 원소 타입: `int` 또는 `std::string`
- 입력 크기: 각 컨테이너에 원소 10,000개 추가
- 측정 횟수: 명령행 인자로 지정
- 시간 단위: 마이크로초(μs)
- 측정 연산: `push_back` 또는 `linear_push_back`

#### 실행 방법

실행 명령은 다음과 같습니다.

```powershell
# int 벤치마크
MyVector.exe --benchmark 10000 30 int

# std::string 벤치마크
MyVector.exe --benchmark 10000 5 string
```

첫 번째 숫자 `10000`은 각 방식에 추가할 원소 수이고, 두 번째 숫자는 각 방식을 반복 측정할 횟수입니다. 마지막 인자는 원소 타입으로 `int` 또는 `string`을 선택합니다. 원소 타입을 생략하면 `int`, 반복 횟수를 생략하면 10회를 기본값으로 사용합니다.

`string` 옵션은 고정된 난수 시드로 생성한 24자 문자열을 사용합니다. 모든 컨테이너에 동일한 문자열을 입력하며, 문자열 생성 시간은 측정에서 제외합니다. 반복 결과는 평균값, 중앙값, 최솟값과 최댓값으로 출력합니다.

#### Release 측정 결과

##### `int`

원소 10,000개를 각 방식에서 30회씩 측정했습니다.

| 방식 | 평균 | 중앙값 | 최소 | 최대 | 재할당 | 최종 용량 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyVector Geometric Growth 1.5 (기본값) | 42μs | 43μs | 29μs | 55μs | 23회 | 12,138 |
| MyVector Geometric Growth 2.0 (변경값) | 38μs | 38μs | 29μs | 73μs | 14회 | 16,384 |
| MyVector Linear Growth | 1,299μs | 546μs | 452μs | 5,861μs | 1,000회 | 10,001 |
| `std::vector` | 15μs | 14μs | 12μs | 36μs | 24회 | 12,138 |

##### `std::string`

24자의 무작위 문자열 10,000개를 각 방식에서 5회씩 측정했습니다.

| 방식 | 평균 | 중앙값 | 최소 | 최대 | 재할당 | 최종 용량 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| MyVector Geometric Growth 1.5 (기본값) | 775μs | 688μs | 627μs | 1,212μs | 23회 | 12,138 |
| MyVector Geometric Growth 2.0 (변경값) | 609μs | 607μs | 586μs | 634μs | 14회 | 16,384 |
| MyVector Linear Growth | 40,395μs | 41,581μs | 27,757μs | 54,571μs | 1,000회 | 10,001 |
| `std::vector` | 645μs | 607μs | 596μs | 727μs | 24회 | 12,138 |

#### 측정 결과 분석

<details>
<summary><strong>0. Geometric Factor에 따른 차이</strong></summary>

Growth Factor가 클수록 한 번에 확보하는 메모리는 증가하지만, 전체 **재할당 횟수**와 기존 원소를 **이동하는 누적 비용**은 감소한다.

> **변수 정의 및 재할당 횟수 계산**
>
> - $N$: 최종 원소 개수
> - $a$: Geometric Growth Factor
> - $k$: 재할당 횟수
>
> 비어 있는 상태에서 $N$개의 원소를 순차적으로 추가할 때의 재할당 횟수는 다음과 같다.
>
> $$a^k \ge N$$
>
> $$k = \lceil \log_a N \rceil$$

> **누적 메모리 할당 크기 계산**
>
> 비어 있는 상태에서 $N$개의 원소를 순차적으로 추가할 때의 누적 메모리 할당 크기는 다음과 같다.
>
> $$1 + a + a^2 + \cdots + a^k= \frac{a^{k+1}-1}{a-1}$$
>
> $$\frac{aN-1}{a-1}\approx \frac{a}{a-1}N$$

따라서 Growth Factor에 따른 누적 이동량(할당량)은 다음과 같이 근사할 수 있다.
- Factor 1.5: 약 $3N$
- Factor 2.0: 약 $2N$

이론적으로 Factor 1.5는 Factor 2.0보다 약 **1.5배** 많은 원소를 누적해서 이동(할당)한다. 실제 측정에서도 Factor 1.5는 23회, Factor 2.0은 14회의 재할당이 발생하여 Factor 2.0이 더 적은 재할당 횟수를 보였다.
다만 실행 시간이 반드시 누적 이동량과 같은 비율로 증가하지는 않는다. 이번 측정에서 최종 Capacity는 각각 다음과 같았다.
- Factor 1.5: `12,138`
- Factor 2.0: `16,384`

Factor 1.5의 최종 할당 크기는 Factor 2.0의 약 $3/4$ 수준이다. 재할당 작업량 차이를 단순하게 보정하면 다음과 같다.

$$1.5 \times \frac{3}{4} = 1.125$$

`int` 측정 중앙값인 `43us`와 `38us`의 비율도 대략 1.13배였다. (string역시 마찬가지이다)
이는 Factor의 값을 늘린다고 해도 그만큼 시간적인 차이가 크지 않고 메모리만 크게 점유를 할수있음을 보여준다. 

</details>

<details>
<summary><strong>1. Capacity 확장 방식에 따른 차이</strong></summary>

Linear Grow 방식은 재할당 횟수는 $O(N)$이고, 누적 원소 이동량은 $O(N^2)$이다.
반면 Geometric Growth 방식은 앞서 구했듯이 재할당 횟수는 $O(\log N)$이고, 누적 원소 이동량은 $O(N)$이다. 

실제 중앙값을 비교하면 다음과 같다.
| 자료형 | Linear | Factor 1.5 | Factor 2.0 |
| --- | ---: | ---: | ---: |
| `int` | 546us | 43us | 38us |
| `string` | 41,581us | 688us | 607us |

기본적으로 원소 수가 증가할수록 두 확장 방식의 성능 차이는 더욱 커질 것으로 예상된다.

</details>

<details>
<summary><strong>2. 자료형에 따른 move 효용성 분석</strong></summary>

`string` 테스트는 재할당 과정에서 복사 대신 Move 연산을 적용했을 때의 동작과 성능을 확인하기 위해 추가하였다.
`string`을 복사하면 문자열 내용까지 새로운 메모리에 복제해야 하지만, Move 연산은 일반적으로 기존 문자열 버퍼의 소유권을 새로운 객체로 이전할 수 있다.

현재 `MyVector`는 재할당 시 `std::move_if_noexcept`를 이용한다. 안전한 이동 생성이 가능한 자료형에는 Move를 적용하고, 이동 과정에서 예외가 발생할 가능성이 있는 자료형에는 복사를 선택하여 기존 데이터의 안정성을 유지한다.  따라서 move 시멘틱이 존재하는 string은  재할당에 있어서 move를 우선적으로 사용하게 된다.

`string` 측정 중앙값은 다음과 같다.

| 구현 | 중앙값 |
| --- | ---: |
| MyVector Factor 1.5 | 688us |
| MyVector Factor 2.0 | 607us |
| `std::vector` | 607us |
| MyVector Linear Growth | 41,581us |

`MyVector`가 `std::vector`와 유사한 중앙값을 기록했다. 이는 비단순 자료형의 재할당 과정에서 Move를 적용한 현재 구현이 충분히 유효하게 동작하고 있음을 보여준다.

다만 현재 테스트는 Move 버전과 강제 복사 버전을 직접 비교한 것이 아니므로 Move 자체의 개선 비율을 독립적으로 측정한 결과는 아니다. 이 분석에서는 `string` 벤치마크 결과를 통해 Move를 적용한 구현의 실질적인 성능을 확인하는 수준으로 해석한다.

</details>

<details>
<summary><strong>3. stl과의 벤치 마킹 결과</strong></summary>

`int` 테스트에서는 `std::vector`가 `MyVector`보다 빠른 결과(3배)를 보였고 'string'의 경우는 비슷한 결과를 보여준다. 

| 구현 | 중앙값 |
| --- | ---: |
| MyVector Factor 1.5 | 43us |
| MyVector Factor 2.0 | 38us |
| `std::vector` | 14us |

Factor 1.5의 경우 `MyVector`와 `std::vector`의 최종 Capacity가 모두 `12,138`로 같고 재할당 횟수도 실질적으로 동일하다. 따라서 약 3배의 시간 차이는 Growth 정책보다는 구현 세부사항과 측정 단위의 영향을 받은 것으로 판단된다.
또한, 현재 `MyVector` 역시 `int` 재할당에 `memcpy`를 사용하므로 단순히 STL만 일괄 메모리 복사를 사용하기 때문에 발생한 차이는 아니다.

가능한 원인은 다음과 같다.

- 표준 라이브러리의 메모리 할당 및 재배치 경로 최적화
- 컴파일러가 STL 내부 코드를 인라인화하고 최적화하는 정도의 차이
- Capacity 확인 및 측정용 분기에서 발생하는 고정 오버헤드

반면 `string`에서는 MyVector Factor 2.0과 `std::vector`의 중앙값이 모두 `607us`였으며, Factor 1.5도 약 13% 느린 수준이었다.
`string`은 각 원소의 이동 생성과 소멸 비용이 전체 실행 시간에서 큰 비중을 차지하므로 컨테이너 자체의 작은 고정 오버헤드가 상대적으로 덜 부각된 것으로 볼 수 있다.

</details>

## 참고 문헌

- [std::vector - cppreference](https://en.cppreference.com/w/cpp/container/vector.html)
