#include "MyPriorityQueue.h"
template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue()
{
	vecNodes.push_back({ T(), 0 });
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue(const MyPriorityQueue<T, Comp>& other)
{
	vecNodes		= other.vecNodes;
	vecHandles	= other.vecHandles;
	vecFreeIDs	= other.vecFreeIDs;
	comp		= other.comp;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>& MyPriorityQueue<T, Comp>::operator=(const MyPriorityQueue<T, Comp>& other)
{
	vecNodes		= other.vecNodes;
	vecHandles	= other.vecHandles;
	vecFreeIDs	= other.vecFreeIDs;
	comp		= other.comp;
	return *this;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue(MyPriorityQueue<T, Comp>&& other) noexcept
{
	vecNodes		= std::move(other.vecNodes);
	vecHandles	= std::move(other.vecHandles);
	vecFreeIDs	= std::move(other.vecFreeIDs);
	comp		= std::move(other.comp);
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>& MyPriorityQueue<T, Comp>::operator=(MyPriorityQueue&& other) noexcept
{
	vecNodes		= std::move(other.vecNodes);
	vecHandles	= std::move(other.vecHandles);
	vecFreeIDs	= std::move(other.vecFreeIDs);
	comp		= std::move(other.comp);

	return *this;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::~MyPriorityQueue()
{
}

template<typename T, typename Comp>
inline bool MyPriorityQueue<T, Comp>::empty() const
{
	return vecNodes.size() <= 1;
}

template<typename T, typename Comp>
inline int MyPriorityQueue<T, Comp>::size() const
{
	return (vecNodes.size() > 0) ? vecNodes.size() - 1 : 0;
}

template<typename T, typename Comp>
inline const T& MyPriorityQueue<T, Comp>::top() const
{
	if (empty()) {
		throw std::out_of_range("Priority queue is empty");
	}
	return vecNodes[1].value;
}

template<typename T, typename Comp>
inline const T& MyPriorityQueue<T, Comp>::get(FPQHandle handle) const
{
	if (!valid(handle)) {
		throw std::out_of_range("Handle is invalid");
	}

	return vecNodes[vecHandles[handle.iHandleID].iHeapIdx].value;
}

template<typename T, typename Comp>
inline bool MyPriorityQueue<T, Comp>::valid(FPQHandle handle) const
{
	if (handle.iHandleID >= static_cast<uint32_t>(vecHandles.size())) return false;
	if (vecHandles[handle.iHandleID].iGeneration != handle.iGeneration) return false;
	if (!vecHandles[handle.iHandleID].bAlive) return false;

	return true;
}

template<typename T, typename Comp>
inline typename MyPriorityQueue<T, Comp>::FPQHandle MyPriorityQueue<T, Comp>::push(const T& value)
{
	// 1	  : 루트
	// 2n + 0 : 왼쪽 자식
	// 2n + 1 : 오른쪽 자식

	if (vecFreeIDs.empty()) {
		vecFreeIDs.push_back	(static_cast<uint32_t>(vecHandles.size()));
		vecHandles.push_back	({ vecNodes.size(), 0, true});
	}
	uint32_t iHandleID = vecFreeIDs.back();
	vecFreeIDs.pop_back();

	vecHandles[iHandleID].iHeapIdx	= vecNodes.size();
	vecHandles[iHandleID].iGeneration++;
	vecHandles[iHandleID].bAlive		= true;
	vecNodes.push_back({ value, iHandleID });

	int iCurIdx		 = vecNodes.size() - 1;
	while (iCurIdx > 1) {
		int iParentIdx		= iCurIdx / 2;
		if (comp(vecNodes[iParentIdx].value, vecNodes[iCurIdx].value)) {
			swap_node(iParentIdx, iCurIdx);
			iCurIdx			= iParentIdx;
		}
		else {
			break;
		}
	}

	return FPQHandle{ iHandleID, vecHandles[iHandleID].iGeneration };
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::swap_node(int iParentIdx, int iCurIdx)
{
	if (iParentIdx == iCurIdx) return;

	int iCurHandleID	= vecNodes[iCurIdx	].iHandleID;
	int iParentHandleID = vecNodes[iParentIdx	].iHandleID;

	std::swap(vecNodes[iCurIdx], vecNodes[iParentIdx]);

	vecHandles[iCurHandleID	].iHeapIdx = iParentIdx;
	vecHandles[iParentHandleID].iHeapIdx = iCurIdx;
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::print_info()
{
	std::cout << "size : " << size();
	std::cout << std::endl;
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::print_elements()
{
	for (int i = 1; i < vecNodes.size(); i++) {
		std::cout << vecNodes[i].value << '\t';
	}
	std::cout << std::endl;
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::update(FPQHandle handle, const T& value)
{
	if (!valid(handle)) {
		throw std::out_of_range("Handle is invalid");
	}

	int iCurIdx = vecHandles[handle.iHandleID].iHeapIdx;
	
	if		(comp(vecNodes[iCurIdx].value, value))	{//위로
		vecNodes[iCurIdx].value = value;
		while (iCurIdx > 1) {
			int iParentIdx = iCurIdx / 2;
			if (comp(vecNodes[iParentIdx].value, vecNodes[iCurIdx].value)) {
				swap_node(iParentIdx, iCurIdx);
				iCurIdx = iParentIdx;
			}
			else {
				break;
			}
		}
	}
	else									{//아래로
		vecNodes[iCurIdx].value = value;
		while (iCurIdx < vecNodes.size()) {
			int iChildIdx0 = 2 * iCurIdx;
			int iChildIdx1 = std::min(2 * iCurIdx + 1, vecNodes.size() - 1);
			if (iChildIdx0 >= vecNodes.size()) break;

			int iChildIdx = (comp(vecNodes[iChildIdx0].value, vecNodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

			if (comp(vecNodes[iCurIdx].value, vecNodes[iChildIdx].value)) {
				swap_node(iCurIdx, iChildIdx);
				iCurIdx = iChildIdx;
			}
			else {
				break;
			}
		}

	}

}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::erase(FPQHandle handle)
{
	if (!valid(handle)) {
		throw std::out_of_range("Handle is invalid");
	}

	int iCurIdx = vecHandles[handle.iHandleID].iHeapIdx;
	swap_node(iCurIdx, vecNodes.size() - 1);

	auto _handleID	= vecNodes.back().iHandleID;
	vecNodes.pop_back();
	vecHandles[_handleID].bAlive = false;
	vecFreeIDs.push_back(_handleID);

	if (iCurIdx >= vecNodes.size()) return;

	if (iCurIdx > 1 && comp(vecNodes[iCurIdx / 2].value, vecNodes[iCurIdx].value)) {//위로
		while (iCurIdx > 1) {
			int iParentIdx = iCurIdx / 2;
			if (comp(vecNodes[iParentIdx].value, vecNodes[iCurIdx].value)) {
				swap_node(iParentIdx, iCurIdx);
				iCurIdx = iParentIdx;
			}
			else {
				break;
			}
		}
	}
	else {//아래로
		while (iCurIdx < vecNodes.size()) {
			int iChildIdx0 = 2 * iCurIdx;
			int iChildIdx1 = std::min(2 * iCurIdx + 1, vecNodes.size() - 1);
			if (iChildIdx0 >= vecNodes.size()) break;

			int iChildIdx = (comp(vecNodes[iChildIdx0].value, vecNodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

			if (comp(vecNodes[iCurIdx].value, vecNodes[iChildIdx].value)) {
				swap_node(iCurIdx, iChildIdx);
				iCurIdx = iChildIdx;
			}
			else {
				break;
			}
		}

	}

}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::clear()
{
	vecNodes.clear();
	vecFreeIDs.clear();
	vecNodes.push_back({ T(), 0 });

	for (int i = 0; i < vecHandles.size(); i++) {
		vecHandles[i].bAlive = false;
		vecFreeIDs.push_back(i);
	}
}
template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::pop()
{
	if (empty()) return;

	swap_node(1, vecNodes.size() - 1);

	auto _handleID  = vecNodes.back().iHandleID;
	vecNodes.pop_back();
	vecHandles[_handleID].bAlive = false;
	vecFreeIDs.push_back(_handleID);

	int iCurIdx = 1;
	while (iCurIdx < vecNodes.size()) {
		int iChildIdx0 = 2 * iCurIdx;
		int iChildIdx1 = std::min(2 * iCurIdx + 1, vecNodes.size() - 1);
		if (iChildIdx0 >= vecNodes.size()) break;

		int iChildIdx = (comp(vecNodes[iChildIdx0].value, vecNodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

		if (comp(vecNodes[iCurIdx].value, vecNodes[iChildIdx].value)) {
			swap_node(iCurIdx, iChildIdx);
			iCurIdx = iChildIdx;
		}
		else {
			break;
		}
	}
}
