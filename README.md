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

| 자료구조 | 상태 | 설명 |
| --- | --- | --- |
| [MyVector](DataStructure/MyVector/README.md) | 구현·분석 | 연속 메모리 기반 동적 배열 |
| [MyList](DataStructure/MyList/README.md) | 구현·분석 | 양방향 연결 리스트 |
| [MyPriorityQueue](DataStructure/MyPriorityQueue/README.md) | 구현·분석 | Binary Heap과 Handle 기반 Decrease-Key |
| [MyDeque](DataStructure/MyDeque/README.md) | 구현·분석 | Chunked Ring Buffer Deque |
| [MyUnorderedHash](DataStructure/MyUnorderedHash/README.md) | 구현·분석 | Hash Table 기반 Set/Map/MultiSet/MultiMap; 유효성 테스트 9개 통과, 로드 팩터별 조회 측정 |
| [MyOrderedTree](DataStructure/MyOrderedTree/README.md) | 네 어댑터 틀 구성 | Red-Black Tree 기반 Set/Map/MultiSet/MultiMap 구현 예정 |

## 프로젝트 구성

```text
DataStructure/
├─ Common/             # 공통 인터페이스와 테스트 유틸리티
├─ MyVector/
├─ MyList/
├─ MyPriorityQueue/
├─ MyDeque/
├─ MyUnorderedHash/
└─ MyOrderedTree/      # 구현 전 프로젝트 뼈대

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

측정 대상 연산과 입력 조건은 프로젝트마다 다릅니다. 예를 들어 해시 프로젝트의 로드 팩터 비교는 정수 키 **조회만** 측정하므로 삽입·삭제 성능까지 대표하지 않습니다. 아직 구현 전인 MyOrderedTree에는 측정 결과가 없습니다.

## 개발 환경

- C++17
- Visual Studio 2022 (`v143` toolset)
- Windows x64

## 문서 원칙

공개 저장소에서 자료구조별 코드를 독립적으로 읽을 수 있도록 불필요한 의존성을 줄입니다. 메인 문서는 프로젝트와 측정 방식만 설명하고, 세부 문서는 구현된 기능과 성능 분석 결과에 집중합니다.
