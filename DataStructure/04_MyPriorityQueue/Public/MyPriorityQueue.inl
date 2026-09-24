#include "MyPriorityQueue.h"
template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue()
{
	_nodes.push_back({ T(), 0 });
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue(const MyPriorityQueue<T, Comp>& other)
{
	_nodes		= other._nodes;
	_handles	= other._handles;
	_freeIDs	= other._freeIDs;
	_comp		= other._comp;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>& MyPriorityQueue<T, Comp>::operator=(const MyPriorityQueue<T, Comp>& other)
{
	_nodes		= other._nodes;
	_handles	= other._handles;
	_freeIDs	= other._freeIDs;
	_comp		= other._comp;
	return *this;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::MyPriorityQueue(MyPriorityQueue<T, Comp>&& other) noexcept
{
	_nodes		= std::move(other._nodes);
	_handles	= std::move(other._handles);
	_freeIDs	= std::move(other._freeIDs);
	_comp		= std::move(other._comp);
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>& MyPriorityQueue<T, Comp>::operator=(MyPriorityQueue&& other) noexcept
{
	_nodes		= std::move(other._nodes);
	_handles	= std::move(other._handles);
	_freeIDs	= std::move(other._freeIDs);
	_comp		= std::move(other._comp);

	return *this;
}

template<typename T, typename Comp>
inline MyPriorityQueue<T, Comp>::~MyPriorityQueue()
{
}

template<typename T, typename Comp>
inline bool MyPriorityQueue<T, Comp>::empty() const
{
	return _nodes.size() <= 1;
}

template<typename T, typename Comp>
inline int MyPriorityQueue<T, Comp>::size() const
{
	return (_nodes.size() > 0) ? _nodes.size() - 1 : 0;
}

template<typename T, typename Comp>
inline const T& MyPriorityQueue<T, Comp>::top() const
{
	if (empty()) {
		throw std::out_of_range("Priority queue is empty");
	}
	return _nodes[1].value;
}

template<typename T, typename Comp>
inline const T& MyPriorityQueue<T, Comp>::get(FPQHandle handle) const
{
	if (!valid(handle)) {
		throw std::out_of_range("Handle is invalid");
	}

	return _nodes[_handles[handle.iHandleID].iHeapIdx].value;
}

template<typename T, typename Comp>
inline bool MyPriorityQueue<T, Comp>::valid(FPQHandle handle) const
{
	if (handle.iHandleID >= static_cast<uint32_t>(_handles.size())) return false;
	if (_handles[handle.iHandleID].iGeneration != handle.iGeneration) return false;
	if (!_handles[handle.iHandleID].bAlive) return false;

	return true;
}

template<typename T, typename Comp>
inline typename MyPriorityQueue<T, Comp>::FPQHandle MyPriorityQueue<T, Comp>::push(const T& value)
{
	// 1	  : 루트
	// 2n + 0 : 왼쪽 자식
	// 2n + 1 : 오른쪽 자식

	if (_freeIDs.empty()) {
		_freeIDs.push_back	(static_cast<uint32_t>(_handles.size()));
		_handles.push_back	({ _nodes.size(), 0, true});
	}
	uint32_t iHandleID = _freeIDs.back();
	_freeIDs.pop_back();

	_handles[iHandleID].iHeapIdx	= _nodes.size();
	_handles[iHandleID].iGeneration++;
	_handles[iHandleID].bAlive		= true;
	_nodes.push_back({ value, iHandleID });

	int iCurIdx		 = _nodes.size() - 1;
	while (iCurIdx > 1) {
		int iParentIdx		= iCurIdx / 2;
		if (_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) {
			SwapNode(iParentIdx, iCurIdx);
			iCurIdx			= iParentIdx;
		}
		else {
			break;
		}
	}

	return FPQHandle{ iHandleID, _handles[iHandleID].iGeneration };
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::SwapNode(int iParentIdx, int iCurIdx)
{
	if (iParentIdx == iCurIdx) return;

	int iCurHandleID	= _nodes[iCurIdx	].iHandleID;
	int iParentHandleID = _nodes[iParentIdx	].iHandleID;

	std::swap(_nodes[iCurIdx], _nodes[iParentIdx]);

	_handles[iCurHandleID	].iHeapIdx = iParentIdx;
	_handles[iParentHandleID].iHeapIdx = iCurIdx;
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
	for (int i = 1; i < _nodes.size(); i++) {
		std::cout << _nodes[i].value << '\t';
	}
	std::cout << std::endl;
}

template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::update(FPQHandle handle, const T& value)
{
	if (!valid(handle)) {
		throw std::out_of_range("Handle is invalid");
	}

	int iCurIdx = _handles[handle.iHandleID].iHeapIdx;
	
	if		(_comp(_nodes[iCurIdx].value, value))	{//위로
		_nodes[iCurIdx].value = value;
		while (iCurIdx > 1) {
			int iParentIdx = iCurIdx / 2;
			if (_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) {
				SwapNode(iParentIdx, iCurIdx);
				iCurIdx = iParentIdx;
			}
			else {
				break;
			}
		}
	}
	else									{//아래로
		_nodes[iCurIdx].value = value;
		while (iCurIdx < _nodes.size()) {
			int iChildIdx0 = 2 * iCurIdx;
			int iChildIdx1 = std::min(2 * iCurIdx + 1, _nodes.size() - 1);
			if (iChildIdx0 >= _nodes.size()) break;

			int iChildIdx = (_comp(_nodes[iChildIdx0].value, _nodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

			if (_comp(_nodes[iCurIdx].value, _nodes[iChildIdx].value)) {
				SwapNode(iCurIdx, iChildIdx);
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

	int iCurIdx = _handles[handle.iHandleID].iHeapIdx;
	SwapNode(iCurIdx, _nodes.size() - 1);

	auto _handleID	= _nodes.back().iHandleID;
	_nodes.pop_back();
	_handles[_handleID].bAlive = false;
	_freeIDs.push_back(_handleID);

	if (iCurIdx >= _nodes.size()) return;

	if (iCurIdx > 1 && _comp(_nodes[iCurIdx / 2].value, _nodes[iCurIdx].value)) {//위로
		while (iCurIdx > 1) {
			int iParentIdx = iCurIdx / 2;
			if (_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) {
				SwapNode(iParentIdx, iCurIdx);
				iCurIdx = iParentIdx;
			}
			else {
				break;
			}
		}
	}
	else {//아래로
		while (iCurIdx < _nodes.size()) {
			int iChildIdx0 = 2 * iCurIdx;
			int iChildIdx1 = std::min(2 * iCurIdx + 1, _nodes.size() - 1);
			if (iChildIdx0 >= _nodes.size()) break;

			int iChildIdx = (_comp(_nodes[iChildIdx0].value, _nodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

			if (_comp(_nodes[iCurIdx].value, _nodes[iChildIdx].value)) {
				SwapNode(iCurIdx, iChildIdx);
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
	_nodes.clear();
	_freeIDs.clear();
	_nodes.push_back({ T(), 0 });

	for (int i = 0; i < _handles.size(); i++) {
		_handles[i].bAlive = false;
		_freeIDs.push_back(i);
	}
}
template<typename T, typename Comp>
inline void MyPriorityQueue<T, Comp>::pop()
{
	if (empty()) return;

	SwapNode(1, _nodes.size() - 1);

	auto _handleID  = _nodes.back().iHandleID;
	_nodes.pop_back();
	_handles[_handleID].bAlive = false;
	_freeIDs.push_back(_handleID);

	int iCurIdx = 1;
	while (iCurIdx < _nodes.size()) {
		int iChildIdx0 = 2 * iCurIdx;
		int iChildIdx1 = std::min(2 * iCurIdx + 1, _nodes.size() - 1);
		if (iChildIdx0 >= _nodes.size()) break;

		int iChildIdx = (_comp(_nodes[iChildIdx0].value, _nodes[iChildIdx1].value)) ? iChildIdx1 : iChildIdx0;

		if (_comp(_nodes[iCurIdx].value, _nodes[iChildIdx].value)) {
			SwapNode(iCurIdx, iChildIdx);
			iCurIdx = iChildIdx;
		}
		else {
			break;
		}
	}
}
