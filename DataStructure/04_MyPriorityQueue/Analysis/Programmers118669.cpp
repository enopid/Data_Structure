#include <algorithm>
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>

using namespace std;

#define USE_DECREASE_KEY 1

template<typename T, typename Comp>
class MyPriorityQueue {
public:
    struct FPQHandle {
        uint32_t iHandleID = 0;
        uint32_t iGeneration = 0;
    };

private:
    struct FPQHandleSlot {
        int iHeapIdx = 0;
        uint32_t iGeneration = 0;
        bool bAlive = false;
    };

    struct FPQNode {
        T value;
        uint32_t iHandleID = 0;
    };

public:
    MyPriorityQueue() {
        _nodes.push_back({T(), 0});
    }

    bool empty() const {
        return _nodes.size() <= 1;
    }

    int size() const {
        return static_cast<int>(_nodes.size()) - 1;
    }

    const T& top() const {
        return _nodes[1].value;
    }

    const T& get(FPQHandle handle) const {
        return _nodes[_handles[handle.iHandleID].iHeapIdx].value;
    }

    bool valid(FPQHandle handle) const {
        if (handle.iHandleID >= _handles.size()) return false;
        if (_handles[handle.iHandleID].iGeneration != handle.iGeneration) return false;
        return _handles[handle.iHandleID].bAlive;
    }

    FPQHandle push(const T& value) {
        if (_freeIDs.empty()) {
            _freeIDs.push_back(static_cast<uint32_t>(_handles.size()));
            _handles.push_back({static_cast<int>(_nodes.size()), 0, true});
        }

        const uint32_t iHandleID = _freeIDs.back();
        _freeIDs.pop_back();

        _handles[iHandleID].iHeapIdx = static_cast<int>(_nodes.size());
        ++_handles[iHandleID].iGeneration;
        _handles[iHandleID].bAlive = true;
        _nodes.push_back({value, iHandleID});

        int iCurIdx = static_cast<int>(_nodes.size()) - 1;
        while (iCurIdx > 1) {
            const int iParentIdx = iCurIdx / 2;
            if (!_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) break;
            SwapNode(iParentIdx, iCurIdx);
            iCurIdx = iParentIdx;
        }

        return {iHandleID, _handles[iHandleID].iGeneration};
    }

    void pop() {
        if (empty()) return;

        SwapNode(1, static_cast<int>(_nodes.size()) - 1);
        const uint32_t iHandleID = _nodes.back().iHandleID;
        _nodes.pop_back();
        _handles[iHandleID].bAlive = false;
        _freeIDs.push_back(iHandleID);
        SiftDown(1);
    }

    void update(FPQHandle handle, const T& value) {
        int iCurIdx = _handles[handle.iHandleID].iHeapIdx;
        const bool bMoveUp = _comp(_nodes[iCurIdx].value, value);
        _nodes[iCurIdx].value = value;

        if (bMoveUp) {
            while (iCurIdx > 1) {
                const int iParentIdx = iCurIdx / 2;
                if (!_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) break;
                SwapNode(iParentIdx, iCurIdx);
                iCurIdx = iParentIdx;
            }
        } else {
            SiftDown(iCurIdx);
        }
    }

    void erase(FPQHandle handle) {
        int iCurIdx = _handles[handle.iHandleID].iHeapIdx;
        SwapNode(iCurIdx, static_cast<int>(_nodes.size()) - 1);

        const uint32_t iHandleID = _nodes.back().iHandleID;
        _nodes.pop_back();
        _handles[iHandleID].bAlive = false;
        _freeIDs.push_back(iHandleID);

        if (iCurIdx >= static_cast<int>(_nodes.size())) return;
        if (iCurIdx > 1 && _comp(_nodes[iCurIdx / 2].value, _nodes[iCurIdx].value)) {
            while (iCurIdx > 1) {
                const int iParentIdx = iCurIdx / 2;
                if (!_comp(_nodes[iParentIdx].value, _nodes[iCurIdx].value)) break;
                SwapNode(iParentIdx, iCurIdx);
                iCurIdx = iParentIdx;
            }
        } else {
            SiftDown(iCurIdx);
        }
    }

    void clear() {
        _nodes.clear();
        _freeIDs.clear();
        _nodes.push_back({T(), 0});

        for (int i = 0; i < static_cast<int>(_handles.size()); ++i) {
            _handles[i].bAlive = false;
            _freeIDs.push_back(static_cast<uint32_t>(i));
        }
    }

private:
    void SwapNode(int iFirstIdx, int iSecondIdx) {
        if (iFirstIdx == iSecondIdx) return;

        const uint32_t iFirstHandleID = _nodes[iFirstIdx].iHandleID;
        const uint32_t iSecondHandleID = _nodes[iSecondIdx].iHandleID;
        swap(_nodes[iFirstIdx], _nodes[iSecondIdx]);
        _handles[iFirstHandleID].iHeapIdx = iSecondIdx;
        _handles[iSecondHandleID].iHeapIdx = iFirstIdx;
    }

    void SiftDown(int iCurIdx) {
        while (iCurIdx < static_cast<int>(_nodes.size())) {
            const int iLeftIdx = iCurIdx * 2;
            if (iLeftIdx >= static_cast<int>(_nodes.size())) break;

            const int iRightIdx = min(iLeftIdx + 1, static_cast<int>(_nodes.size()) - 1);
            const int iChildIdx = _comp(_nodes[iLeftIdx].value, _nodes[iRightIdx].value)
                ? iRightIdx
                : iLeftIdx;

            if (!_comp(_nodes[iCurIdx].value, _nodes[iChildIdx].value)) break;
            SwapNode(iCurIdx, iChildIdx);
            iCurIdx = iChildIdx;
        }
    }

private:
    vector<FPQNode> _nodes;
    vector<FPQHandleSlot> _handles;
    vector<uint32_t> _freeIDs;
    Comp _comp;
};

vector<int> DijkstraSTL(
    const vector<vector<pair<int, int>>>& vecGraph,
    const vector<int>& gates,
    const vector<bool>& vecSummit) {
    vector<int> vecIntensity(vecGraph.size(), 10000001);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pqOpen;

    for (int gate : gates) {
        vecIntensity[gate] = 0;
        pqOpen.push({0, gate});
    }

    while (!pqOpen.empty()) {
        const pair<int, int> cur = pqOpen.top();
        pqOpen.pop();

        if (vecIntensity[cur.second] != cur.first) continue;
        if (vecSummit[cur.second]) continue;

        for (const pair<int, int>& edge : vecGraph[cur.second]) {
            const int iNextIntensity = max(cur.first, edge.first);
            if (vecIntensity[edge.second] <= iNextIntensity) continue;

            vecIntensity[edge.second] = iNextIntensity;
            pqOpen.push({iNextIntensity, edge.second});
        }
    }

    return vecIntensity;
}

vector<int> DijkstraDecreaseKey(
    const vector<vector<pair<int, int>>>& vecGraph,
    const vector<int>& gates,
    const vector<bool>& vecSummit) {
    MyPriorityQueue<pair<int, int>, greater<pair<int, int>>> pqOpen;
    vector<int> vecIntensity(vecGraph.size(), 10000001);
    vector<MyPriorityQueue<pair<int, int>, greater<pair<int, int>>>::FPQHandle> vecHandle(
        vecGraph.size());
    vector<bool> vecDiscovered(vecGraph.size(), false);

    for (int gate : gates) {
        vecIntensity[gate] = 0;
        vecHandle[gate] = pqOpen.push({0, gate});
        vecDiscovered[gate] = true;
    }

    while (!pqOpen.empty()) {
        const pair<int, int> cur = pqOpen.top();
        pqOpen.pop();

        if (vecSummit[cur.second]) continue;

        for (const pair<int, int>& edge : vecGraph[cur.second]) {
            const int iNextIntensity = max(cur.first, edge.first);
            if (vecIntensity[edge.second] <= iNextIntensity) continue;
            if (vecDiscovered[edge.second] && !pqOpen.valid(vecHandle[edge.second])) continue;

            vecIntensity[edge.second] = iNextIntensity;
            if (vecDiscovered[edge.second]) {
                pqOpen.update(vecHandle[edge.second], {iNextIntensity, edge.second});
            } else {
                vecHandle[edge.second] = pqOpen.push({iNextIntensity, edge.second});
                vecDiscovered[edge.second] = true;
            }
        }
    }

    return vecIntensity;
}

vector<int> Dijkstra(
    const vector<vector<pair<int, int>>>& vecGraph,
    const vector<int>& gates,
    const vector<bool>& vecSummit) {
#if USE_DECREASE_KEY
    return DijkstraDecreaseKey(vecGraph, gates, vecSummit);
#else
    return DijkstraSTL(vecGraph, gates, vecSummit);
#endif
}

vector<int> solution(
    int n,
    vector<vector<int>> paths,
    vector<int> gates,
    vector<int> summits) {
    vector<vector<pair<int, int>>> vecGraph(n + 1);
    vector<bool> vecSummit(n + 1, false);

    for (const vector<int>& path : paths) {
        vecGraph[path[0]].push_back({path[2], path[1]});
        vecGraph[path[1]].push_back({path[2], path[0]});
    }

    for (int summit : summits) vecSummit[summit] = true;

    const vector<int> vecIntensity = Dijkstra(vecGraph, gates, vecSummit);
    sort(summits.begin(), summits.end());

    vector<int> answer{0, 10000001};
    for (int summit : summits) {
        if (vecIntensity[summit] >= answer[1]) continue;
        answer[0] = summit;
        answer[1] = vecIntensity[summit];
    }

    return answer;
}
