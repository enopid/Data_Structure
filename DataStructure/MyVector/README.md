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

## 참고 문헌

- [std::vector - cppreference](https://en.cppreference.com/w/cpp/container/vector.html)

