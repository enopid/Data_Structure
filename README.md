# Data Structure

C++ 표준 라이브러리의 자료구조를 직접 구현하고, 내부 동작과 성능 특성을 분석하는 프로젝트입니다.

완성된 기능의 수를 늘리는 것보다 메모리 관리, 데이터 이동, 용량 증가 정책처럼 자료구조의 성능을 결정하는 핵심 원리를 확인하는 데 목적이 있습니다. <br>
각 자료구조는 가능한 한 독립적으로 구성하여 구현과 분석을 쉽게 읽을 수 있도록 합니다.

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 직접 검토하고 실행하여 결과를 확인했습니다.

- 각 프로젝트의 `Main.cpp`의 유효성 테스트와 벤치마크 코드는 AI의 도움을 받아 작성했습니다.
- 자료구조의 핵심 로직은 직접 설계하고 구현했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 구현 목록

| 자료구조 | 상태 | 설명 |
| --- | --- | --- |
| [MyVector](DataStructure/MyVector/README.md) | 구현 완료 | continuous memory based dynamic array |
| [MyList](DataStructure/MyList/README.md) | 구현 완료 | Doubly Linked List |
| [MyPriorityQueue](DataStructure/MyPriorityQueue/README.md) | 구현 및 유효성 검증 완료 | Binary Heap + Handle 기반 Decrease-Key |
| [MyDeque](DataStructure/MyDeque/README.md) | 구현·유효성 검증·성능 측정 완료 | Chunked Ring Buffer Deque |
| MyStack/MyQueue | 준비 중 | Linked Block List |
| MyUnordered_Set/Map | 준비 중 | Hash Table |
| MyOrdered_Set/Map | 준비 중 | Red Black Tree |

## 프로젝트 구성

```text
DataStructure/
├─ Common/                  # 공통 인터페이스 및 테스트 유틸리티
├─ MyVector/
│  ├─ Public/              # 자료구조 구현
│  ├─ Private/             # 실행 및 테스트 진입점
│  └─ README.md            # 구현 개요와 분석
├─ MyList/
│  ├─ Public/              # 자료구조 구현
│  ├─ Private/             # 실행 및 테스트 진입점
│  └─ README.md            # 구현 개요와 분석
├─ MyPriorityQueue/         # Binary Heap 기반 Priority Queue
├─ MyDeque/                 # Deque
Result/                    # 성능 측정 결과와 그래프
```

## 성능 측정

각 자료구조는 동일한 입력 크기와 빌드 환경에서 표준 라이브러리 구현과 비교하는 것을 원칙으로 합니다.

- 시간 측정: 반복 실행한 연산의 경과 시간을 측정하고 평균값을 사용합니다.
- 비교 대상: 대응하는 C++ 표준 컨테이너를 기준으로 합니다.
- 빌드 조건: 최적화가 적용된 `Release x64` 구성을 기준으로 합니다.
- 주요 지표: 입력 크기에 따른 실행 시간, 재할당 횟수, 용량 증가 정책에 따른 차이를 확인합니다.

현재 저장소에는 정식 벤치마크 결과가 포함되어 있지 않습니다. 측정값이 확보되면 각 자료구조의 README에는 구현 세부사항 대신 비교 그래프와 해석을 중심으로 추가합니다.

## 개발 환경

- C++17
- Visual Studio 2022 (`v143` toolset)
- Windows x64

## 문서 원칙

공개 저장소에서 자료구조별 코드를 독립적으로 읽을 수 있도록 불필요한 의존성을 줄입니다. 메인 문서는 프로젝트와 측정 방식만 설명하고, 세부 문서는 구현된 기능과 성능 분석 결과에 집중합니다.
