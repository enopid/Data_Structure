# MyOrderedTree

레드블랙 트리 기반 Ordered Set/Map/MultiSet/MultiMap 구현을 위한 프로젝트입니다. `MyUnorderedHash`와 같은 네 어댑터 구조를 준비하고 공통 기반 클래스 이름을 `MyRedBlackTree`로 바꿨습니다. 트리 노드, 내부 변수, 탐색·삽입·삭제 및 성능 측정은 아직 구현하지 않았습니다.

- `Public/MyRedBlackTree.h/.inl`: 공통 레드블랙 트리 기반 틀
- `Public/MyOrderedSet.h/.inl`, `MyOrderedMultiSet.h/.inl`: Set 계열 어댑터
- `Public/MyOrderedMap.h/.inl`, `MyOrderedMultiMap.h/.inl`: Map 계열 어댑터
- `Private/Main.cpp`: 네 어댑터의 프로젝트 빌드 확인 진입점

기존 unordered 어댑터의 키·값 추출 방식만 옮겼습니다. 해시 버킷 전용 `max_size` 생성자는 가져오지 않았고, `MyOrderedMap::at`과 `operator[]`는 트리 탐색·삽입 인터페이스가 생긴 뒤 구현할 수 있도록 선언만 남겼습니다.
