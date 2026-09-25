# Data Structure

C++ 표준 라이브러리의 자료구조를 직접 구현하고, 내부 동작과 성능 특성을 분석하는 프로젝트입니다.

완성된 기능의 수를 늘리는 것보다 메모리 관리, 데이터 이동, 용량 증가 정책처럼 자료구조의 성능을 결정하는 핵심 원리를 확인하는 데 목적이 있습니다. <br>
각 자료구조는 가능한 한 독립적으로 구성하여 구현과 분석을 쉽게 읽을 수 있도록 합니다.

## AI 활용 범위

> 이 프로젝트는 유효성 테스트와 벤치마크 코드의 설계 및 작성에 AI를 적극적으로 활용했습니다. AI가 생성한 코드는 **직접 검토하고 실행하여 결과를 확인**했습니다.

- **자료구조의 핵심 로직은 직접 설계하고 구현했습니다.**
- 구현·테스트가 진행된 각 프로젝트의 `Main.cpp`는 유효성 테스트와 벤치마크 코드 부분으로 AI의 도움을 받아 작성했습니다.
- AI는 직접 구현한 자료구조의 동작 검증과 성능 측정을 위한 보조 도구로 사용했습니다.

## 구현 목록

| 자료구조 | 핵심 구현 | 핵심 분석 |
| --- | --- | --- |
| [MyVector](DataStructure/01_MyVector/README.md) | 연속 메모리, 직접 객체 수명 관리, 복사·이동, 가변 성장 계수 | 성장 계수 1.5/2.0과 선형 증가 방식의 재할당 횟수·삽입 시간 비교 |
| [MyList](DataStructure/02_MyList/README.md) | Sentinel 기반 양방향 연결 리스트와 양방향 iterator | 노드 기반 순차 삽입을 `std::list` 및 연속 메모리 컨테이너와 비교 |
| [MyDeque](DataStructure/03_MyDeque/README.md) | 청크 배열과 링 구조를 결합한 양방향 큐 | 청크 크기별 순차·복합 연산 비용 및 `std::deque`의 작은 청크 정책 비교 |
| [MyPriorityQueue](DataStructure/04_MyPriorityQueue/README.md) | Binary Heap, 세대 기반 Handle, `update`·`erase`·Decrease-Key | STL 기본 힙 연산과 비교하고 추가 기능의 핸들 관리 비용 분석 |
| [MyUnorderedHash](DataStructure/05_MyUnorderedHash/README.md) | 버킷별 리스트 구간과 Set/Map/Multi 어댑터 | `std::unordered_set`과 삽입·탐색·삭제를 비교하고 Load Factor에 따른 조회 비용 분석 |
| [MyOrderedTree](DataStructure/06_MyOrderedTree/README.md) | Red-Black Tree의 회전·삽입/삭제 복구와 Ordered 어댑터 | `std::set` 비교 및 Ordered/Unordered의 삽입·탐색·삭제 성능 분석 |

## 프로젝트 구성

```text
DataStructure/
├─ Common/             # 공통 인터페이스와 테스트 유틸리티
├─ 01_MyVector/
├─ 02_MyList/
├─ 03_MyDeque/
├─ 04_MyPriorityQueue/
├─ 05_MyUnorderedHash/
└─ 06_MyOrderedTree/

각 프로젝트/
├─ Public/             # 자료구조 헤더와 템플릿 정의
├─ Private/            # 실행·유효성·부하 테스트 진입점
├─ Default/            # Visual Studio 프로젝트 설정
└─ README.md           # 구현 개요, 테스트 코드와 측정 결과
```

## 성능 측정

완성된 각 자료구조의 `Private/Main.cpp`에서 자체 유효성 테스트를 실행하고, 대응하는 표준 라이브러리 컨테이너와 부하 테스트를 비교합니다. 자세한 테스트 함수와 실제 결과는 프로젝트별 README에 기록합니다.

- 유효성 검증: 중복·빈 상태·복사·이동 등 자료구조별 주요 경로를 확인합니다.
- 시간 측정: 연산을 여러 번 반복해 경과 시간을 측정하고 평균·중앙값을 표시합니다.
- 비교 대상: 대응하는 MSVC STL 컨테이너를 사용합니다.
- 빌드 조건: 최적화가 적용된 `Release x64`를 기준으로 합니다.
- 측정 지표: 자료구조에 따라 실행 시간, 재할당 횟수, 청크 크기 또는 로드 팩터를 비교합니다.

측정 대상 연산과 입력 조건은 프로젝트마다 다릅니다. 예를 들어 해시 프로젝트의 로드 팩터 비교는 정수 키 **조회만** 측정하므로 삽입·삭제 성능까지 대표하지 않습니다. 각 결과는 특정 입력과 실행 환경에서 구현 선택에 따른 차이를 확인하기 위한 자료이며, 모든 환경에서의 절대적인 성능 우위를 의미하지 않습니다.

## 개발 환경

- C++17
- Visual Studio 2022 (`v143` toolset)
- Windows x64

## 문서 원칙

공개 저장소에서 자료구조별 코드를 독립적으로 읽을 수 있도록 불필요한 의존성을 줄입니다. 메인 문서는 프로젝트와 측정 방식만 설명하고, 세부 문서는 구현된 기능과 성능 분석 결과에 집중합니다.
