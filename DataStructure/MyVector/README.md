# MyVector

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
  - 증가 계수에 따른 재할당 횟수, 메모리 여유 공간과 실행 시간의 변화를 비교하기 위한 기능입니다.
- `linear_push_back(const T& val)`
  - 용량이 부족할 때 기존 용량에 `10`을 더해 선형적으로 확장합니다.
  - Geometric Growth 방식과 Linear Growth 방식의 누적 재할당 비용을 비교하기 위한 기능입니다.

## 분석

`MyVector`의 핵심 자료구조 로직은 직접 구현했습니다. `Main.cpp`에 포함된 유효성 테스트와 벤치마크 코드는 구현 결과의 검증 및 측정을 위해 AI를 통해 생성했습니다.

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

```text
7 passed, 0 failed
```

### 벤치마크 결과

- 빌드 구성: `Release x64`
- 원소 타입: `int`
- 입력 크기: 각 컨테이너에 원소 10,000개 추가
- 측정 횟수: 각 방식당 1회
- 시간 단위: 마이크로초(μs)
- 측정 연산: `push_back` 또는 `linear_push_back`

실행 명령은 다음과 같습니다.

```powershell
MyVector.exe --benchmark 10000
```

| 방식 | 실행 시간 | 재할당 횟수 | 최종 용량 |
| --- | ---: | ---: | ---: |
| MyVector Geometric Growth | 53μs | 14회 | 16,384 |
| MyVector Linear Growth | 8,766μs | 1,000회 | 10,001 |
| `std::vector` | 71μs | 24회 | 12,138 |

Geometric Growth 방식은 메모리 여유 공간을 더 사용하는 대신 재할당 횟수를 줄였습니다. Linear Growth 방식은 최종 여유 공간은 적지만 재할당이 1,000회 발생하여 실행 시간이 크게 증가했습니다.

이번 측정에서 MyVector Geometric Growth가 `std::vector`보다 빠르게 나타났지만, 측정 횟수가 각 방식당 1회이고 두 구현의 용량 증가 정책도 다릅니다. 따라서 해당 수치는 일반적인 우위가 아니라 현재 환경과 입력 조건에서 얻은 결과로 한정합니다.

## 참고 문헌

- [std::vector - cppreference](https://en.cppreference.com/w/cpp/container/vector.html)
